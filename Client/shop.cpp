#include "shop.h"
#include "ui_shop.h"

const QColor WHITE = QColor("white");
//const QColor GREEN = QColor("darkgreen");
const QColor YELLOW = QColor("yellow");
//const QColor RED = QColor("darkred");
//const QColor GRAY = QColor("darkgray");
//const QColor ORANGE = QColor("orange");


Shop::Shop(int t_userId, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Shop)
    , m_userId(t_userId)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager();

    ui->shootingRangeTable->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui->weaponsShopTableWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui->weaponsInStockTableWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui->shootingRangeTable->setSelectionMode(QAbstractItemView::NoSelection);
    ui->shootingRangeTable->setMouseTracking(true);

    connect(ui->buyWeaponButton, &QPushButton::clicked, this, &Shop::buyWeapon);
    connect(ui->buyAllWeaponsButton, &QPushButton::clicked, this, [=]() { buyWeapon(true); });
    connect(ui->sellWeaponButton, &QPushButton::clicked, this, &Shop::sellWeapon);
    connect(ui->sellAllWeaponsButton, &QPushButton::clicked, this, [=]() { sellWeapon(true); });
    connect(ui->shootingRangeTable, &QTableWidget::itemEntered, this, &Shop::highlightCell);

    connect(ui->weaponsShopTableWidget, &QTableWidget::itemSelectionChanged,
            this, &Shop::setCurrentWeaponRange);

    connect(ui->weaponsInStockTableWidget, &QTableWidget::itemSelectionChanged,
            this, [=]() { setTotalPrices(ui->weaponAmountSpinBox->value()); });
    connect(ui->weaponsShopTableWidget, &QTableWidget::itemSelectionChanged,
            this, [=]() { setTotalPrices(ui->weaponAmountSpinBox->value()); });

    connect(ui->weaponAmountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setTotalPrices(int)));

    getWeapons();
    initShootingRange();
}

Shop::~Shop()
{
    delete ui;
}

void Shop::fillWeaponsComboBoxes(QNetworkReply *t_reply)
{
    disconnect(m_manager, &QNetworkAccessManager::finished, this, &Shop::fillWeaponsComboBoxes);
    QJsonObject jsonReply = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();
    if (jsonReply.contains("error")) {
        return showMessage(jsonReply["error"].toString(), QMessageBox::Icon::Critical);
    }

    QJsonArray userWeapons = jsonReply["user_weapons"].toArray();
    QJsonArray shopWeapons = jsonReply["all_weapons"].toArray();

    fillWeaponsInStock(userWeapons);
    fillWeaponsShop(shopWeapons);

    getCoinsAmount();
}

void Shop::getWeaponBoughtStatus(QNetworkReply *t_reply)
{
    disconnect(m_manager, &QNetworkAccessManager::finished, this, &Shop::getWeaponBoughtStatus);
    QString replyStr = t_reply->readAll();
    QJsonObject jsonReply = QJsonDocument::fromJson(replyStr.toUtf8()).object();
    if (jsonReply.contains("error")) {
        showMessage(jsonReply["error"].toString(), QMessageBox::Icon::Critical);
        return;
    }
    if (jsonReply.isEmpty()) {
        return showMessage("Пустой ответ от сервера", QMessageBox::Icon::Critical);
    }

    QString weaponName = jsonReply["weapon_name"].toString();
    int weaponsAmount = jsonReply["weapons_amount"].toInt();
    double weaponSellPrice = jsonReply["weapon_sell_price"].toDouble();
    double silverCoinsLeft = jsonReply["silver_coins_left"].toDouble();

    QStringList alreadyAquiredWeaponsNames;
    for (int row = 0; row < ui->weaponsInStockTableWidget->rowCount(); ++row) {
        alreadyAquiredWeaponsNames << ui->weaponsInStockTableWidget->item(row, 0)->text();
    }

    if (!alreadyAquiredWeaponsNames.contains(weaponName)) {
        int currentRowCount = ui->weaponsInStockTableWidget->rowCount();
        if (currentRowCount == 0) {
            ui->weaponsInStockTableWidget->setRowCount(1);
            ui->weaponsInStockTableWidget->setColumnCount(3);
            ui->weaponsInStockTableWidget->setHorizontalHeaderLabels(
                QStringList() << "Наименование" << "В наличии" << "Цена продажи"
                );
        } else {
            ui->weaponsInStockTableWidget->setRowCount(currentRowCount + 1);
        }
        QStringList columnValues = QStringList()
                << weaponName
                << QString::number(weaponsAmount)
                << QString::number(weaponSellPrice);
        for (int column = 0; column < columnValues.size(); ++column) {
            QTableWidgetItem *item = new QTableWidgetItem(columnValues[column]);
            ui->weaponsInStockTableWidget->setItem(currentRowCount, column, item);
        }
    } else {
        int aquiredWeaponRow = alreadyAquiredWeaponsNames.indexOf(weaponName);
        int aquiredAmount = ui->weaponsInStockTableWidget->item(
                    aquiredWeaponRow, 1)->text().toInt();

        ui->weaponsInStockTableWidget->item(aquiredWeaponRow, 1)->setText(
                    QString::number(aquiredAmount + weaponsAmount));
    }

    renderCoinsAmount(silverCoinsLeft);
}

void Shop::getWeaponSoldStatus(QNetworkReply *t_reply)
{
    disconnect(m_manager, &QNetworkAccessManager::finished, this, &Shop::getWeaponSoldStatus);

    QJsonObject jsonReply = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();
    if (jsonReply.contains("error")) {
        return showMessage(jsonReply["error"].toString(), QMessageBox::Icon::Critical);
    }

    QString weaponName = jsonReply["weapon_name"].toString();
    int weaponAmountLeft = jsonReply["weapon_amount_left"].toInt();
    double silverCoinsLeft = jsonReply["silver_coins_left"].toDouble();

    for (int row = 0; row < ui->weaponsInStockTableWidget->rowCount(); ++row) {
        QString weaponInStockName = ui->weaponsInStockTableWidget->item(row, 0)->text();
        if (weaponName == weaponInStockName) {
            if (weaponAmountLeft == 0) {
                rearrangeWeaponsInStockTable(row);
            } else {
                ui->weaponsInStockTableWidget->item(row, 1)->setText(QString::number(weaponAmountLeft));
            }
            break;
        }
    }

    renderCoinsAmount(silverCoinsLeft);
}

void Shop::setCoinsAmount(QNetworkReply *t_reply)
{
    disconnect(m_manager, &QNetworkAccessManager::finished, this, &Shop::setCoinsAmount);
    QJsonObject jsonReply = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();
    if (jsonReply.contains("error")) {
        return showMessage(jsonReply["error"].toString(), QMessageBox::Icon::Critical);
    }

    double coinsAmount = jsonReply["silver_coins_left"].toDouble();
    renderCoinsAmount(coinsAmount);
}

void Shop::setTotalPrices(int t_weaponAmount)
{
    QList<QTableWidgetItem*> selectedShopItems = ui->weaponsShopTableWidget->selectedItems();
    QList<QTableWidgetItem*> selectedInStockItems = ui->weaponsInStockTableWidget->selectedItems();
    double price = 0.f;
    if (!selectedShopItems.isEmpty()) {
        int row = selectedShopItems[0]->row();
        price = ui->weaponsShopTableWidget->item(row, 1)->text().toDouble();
        ui->totalBuyLabel->setText("Итого для покупки: " + QString::number(t_weaponAmount * price));
    }
    if (!selectedInStockItems.isEmpty()) {
        int row = selectedInStockItems[0]->row();
        price = ui->weaponsInStockTableWidget->item(row, 2)->text().toDouble();
        ui->totalSellLabel->setText("Итого для продажи: " + QString::number(t_weaponAmount * price));
    }
}

void Shop::fillWeaponsInStock(QJsonArray t_userWeapons)
{
    ui->weaponsInStockTableWidget->setRowCount(t_userWeapons.size());
    ui->weaponsInStockTableWidget->setColumnCount(t_userWeapons[0].toObject().size());
    ui->weaponsInStockTableWidget->setHorizontalHeaderLabels(
                QStringList() << "Наименование" << "В наличии" << "Цена продажи"
    );

    for (int i = 0; i < t_userWeapons.size(); ++ i) {
        QJsonObject userWeapon = t_userWeapons[i].toObject();

        QStringList columnValues;
        columnValues << userWeapon["weapon_name"].toString();
        columnValues << QString::number(userWeapon["weapon_amount"].toInt());
        columnValues << QString::number(userWeapon["weapon_sell_price"].toDouble());

        for (int j = 0; j < columnValues.size(); ++j){
            QTableWidgetItem *item = new QTableWidgetItem(columnValues[j]);
            ui->weaponsInStockTableWidget->setItem(i, j, item);
        }
    }
}

void Shop::fillWeaponsShop(QJsonArray t_shopWeapons)
{
    ui->weaponsShopTableWidget->setRowCount(t_shopWeapons.size());
    ui->weaponsShopTableWidget->setColumnCount(t_shopWeapons[0].toObject().size() - 2);
    ui->weaponsShopTableWidget->setHorizontalHeaderLabels(
        QStringList() << "Наименование" << "Цена"
    );
    for (int i = 0; i < t_shopWeapons.size(); ++ i) {
        QJsonObject shopWeapon = t_shopWeapons[i].toObject();

        QStringList columnValues;
        QString weaponName = shopWeapon["weapon_name"].toString();
        int xRange = shopWeapon["weapon_x_range"].toInt();
        int yRange = shopWeapon["weapon_y_range"].toInt();
        columnValues << weaponName;
        columnValues << QString::number(shopWeapon["weapon_price"].toDouble());
        m_weaponsRange[weaponName] = { xRange, yRange };

        for (int j = 0; j < columnValues.size(); ++j){
            QTableWidgetItem *item = new QTableWidgetItem(columnValues[j]);
            ui->weaponsShopTableWidget->setItem(i, j, item);
        }
    }
}

void Shop::getCoinsAmount()
{
    QMap<QString, QString> queryParams;

    queryParams["user_id"] = QString::number(m_userId);

    connect(m_manager, &QNetworkAccessManager::finished, this, &Shop::setCoinsAmount);
    sendServerRequest("http://127.0.0.1:8000/shop/get_user_coins/", queryParams, m_manager);
}

void Shop::getWeapons()
{
    QMap<QString, QString> queryParams;

    queryParams["user_id"] = QString::number(m_userId);

    connect(m_manager, &QNetworkAccessManager::finished, this, &Shop::fillWeaponsComboBoxes);
    sendServerRequest("http://127.0.0.1:8000/shop/get_weapons/", queryParams, m_manager);
}

void Shop::renderCoinsAmount(double t_coinsAmount)
{
    ui->userCoinsLabel->setText("На Вашем счету: " + QString::number(t_coinsAmount) + "\nсеребряных монет");
}

void Shop::buyWeapon(bool t_buyAll)
{
    QList<QTableWidgetItem*> selectedItems = ui->weaponsShopTableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        return showMessage("Выберите оружие для покупки", QMessageBox::Icon::Information);
    }

    QMap<QString, QString> queryParams;
    QString weaponName = ui->weaponsShopTableWidget->item(selectedItems[0]->row(), 0)->text();

    queryParams["user_id"] = QString::number(m_userId);
    queryParams["weapon_name"] = weaponName;
    queryParams["weapons_amount"] = ui->weaponAmountSpinBox->text();
    queryParams["buy_all"] = QString::number(t_buyAll);

    connect(m_manager, &QNetworkAccessManager::finished, this, &Shop::getWeaponBoughtStatus);
    sendServerRequest("http://127.0.0.1:8000/shop/buy_weapon/", queryParams, m_manager);
}

void Shop::sellWeapon(bool t_sellAll)
{
    QList<QTableWidgetItem*> selectedItems = ui->weaponsInStockTableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        return showMessage("Выберите оружие для продажи", QMessageBox::Icon::Information);
    }
    QMap<QString, QString> queryParams;
    QString weaponName = ui->weaponsInStockTableWidget->item(selectedItems[0]->row(), 0)->text();

    int weaponAmount = ui->weaponAmountSpinBox->text().toInt();
    if (weaponAmount == 0) {
        return showMessage("Выберите хотя бы 1 кол-во", QMessageBox::Icon::Critical);
    }

    queryParams["user_id"] = QString::number(m_userId);
    queryParams["weapon_name"] = weaponName;
    queryParams["weapon_amount"] = QString::number(weaponAmount);
    queryParams["sell_all"] = QString::number(t_sellAll);

    connect(m_manager, &QNetworkAccessManager::finished, this, &Shop::getWeaponSoldStatus);
    sendServerRequest("http://127.0.0.1:8000/shop/sell_weapon/", queryParams, m_manager);
}

void Shop::initShootingRange()
{
    ui->shootingRangeTable->setRowCount(10);
    ui->shootingRangeTable->setColumnCount(10);
    ui->shootingRangeTable->resizeColumnsToContents();

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(i + j));
            ui->shootingRangeTable->setItem(i, j, item);
        }
    }
}

void Shop::setCurrentWeaponRange()
{
    QList<QTableWidgetItem*> selectedItems = ui->weaponsShopTableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        m_currentRange = {1, 1};
        return;
    }

    int selectedRow = selectedItems[0]->row();
    QString weaponName = ui->weaponsShopTableWidget->item(selectedRow, 0)->text();
    m_currentRange = m_weaponsRange[weaponName];
    clearHighlightedCells();
}

void Shop::clearHighlightedCells()
{
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            ui->shootingRangeTable->item(i, j)->setBackgroundColor(WHITE);
        }
    }
}

void Shop::highlightCell(QTableWidgetItem *t_item)
{
    clearHighlightedCells();
    int x_start = t_item->row();
    int y_start = t_item->column();
    for (int x = x_start; x < x_start + m_currentRange[0]; ++x) {
        for (int y = y_start; y < y_start + m_currentRange[1]; ++ y) {
            QTableWidgetItem *item = ui->shootingRangeTable->item(x, y);
            if (item != nullptr) {
                item->setBackgroundColor(YELLOW);
            }
        }
    }
}

/*! @brief Перемещение всех ячеек на 1 вверх
 *
 *  @param t_toRow Ряд, до которого нужно переместить ячейки
 *
 *  @return void
*/
void Shop::rearrangeWeaponsInStockTable(int t_fromRow)
{
    int totalRowCount = ui->weaponsInStockTableWidget->rowCount();
    int totalColumnCount = ui->weaponsInStockTableWidget->columnCount();
    if (t_fromRow + 1 > totalRowCount) {
        return;
    }

    for (int row = t_fromRow + 1; row < totalRowCount; ++row) {
        for (int column = 0; column < totalColumnCount; ++column) {
            ui->weaponsInStockTableWidget->item(row - 1, column)->setText(
                ui->weaponsInStockTableWidget->item(row, column)->text()
            );
        }
    }

    ui->weaponsInStockTableWidget->setRowCount(totalRowCount - 1);
}
