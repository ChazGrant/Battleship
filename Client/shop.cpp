#include "shop.h"
#include "ui_shop.h"

Shop::Shop(int t_userId, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Shop)
    , m_userId(t_userId)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager();

    connect(ui->buyWeaponButton, &QPushButton::clicked, this, &Shop::buyWeapon);
    connect(ui->sellWeaponButton, &QPushButton::clicked, this, &Shop::sellWeapon);

    getWeapons();
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
    qDebug() << jsonReply;

    QString weaponName = jsonReply["weapon_name"].toString();
    int weaponsAmount = jsonReply["weapons_amount"].toInt();
    double weaponSellPrice = jsonReply["weapon_sell_price"].toDouble();
    double silverCoinsLeft = jsonReply["silver_coins_left"].toDouble();

    qDebug() << "PASS";

    QStringList alreadyAquiredWeaponsNames;
    for (int row = 0; row < ui->weaponsInStockTableWidget->rowCount(); ++row) {
        alreadyAquiredWeaponsNames << ui->weaponsInStockTableWidget->itemAt(row, 0)->text();
    }

    if (!alreadyAquiredWeaponsNames.contains(weaponName)) {
        int currentRowCount = ui->weaponsInStockTableWidget->rowCount();
        ui->weaponsInStockTableWidget->setRowCount(currentRowCount + 1);
        QStringList columnValues = QStringList()
                << weaponName
                << QString::number(weaponsAmount)
                << QString::number(weaponSellPrice);
        for (int column = 0; column < columnValues.size(); ++column) {
            QTableWidgetItem *item = new QTableWidgetItem(columnValues[column]);
            ui->weaponsInStockTableWidget->setItem(column, currentRowCount + 1, item);
        }
    } else {
        int aquiredWeaponRow = alreadyAquiredWeaponsNames.indexOf(weaponName);
        int aquiredAmount = ui->weaponsInStockTableWidget->itemAt(
                    aquiredWeaponRow, 0)->text().toInt();

        ui->weaponsInStockTableWidget->itemAt(aquiredWeaponRow, 0)->setText(
                    QString::number(aquiredAmount + 1));
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
        columnValues << userWeapon["weapon_amount"].toString();
        columnValues << userWeapon["weapon_sell_price"].toString();

        for (int j = 0; j < columnValues.size(); ++j){
            QTableWidgetItem *item = new QTableWidgetItem(columnValues[j]);
            ui->weaponsInStockTableWidget->setItem(i, j, item);
        }
    }
}

void Shop::fillWeaponsShop(QJsonArray t_shopWeapons)
{
    ui->weaponsShopTableWidget->setRowCount(t_shopWeapons.size());
    ui->weaponsShopTableWidget->setColumnCount(t_shopWeapons[0].toObject().size());
    ui->weaponsShopTableWidget->setHorizontalHeaderLabels(
                QStringList() << "Наименование" << "Цена" << "X поражение" << "Y поражение"
    );
    for (int i = 0; i < t_shopWeapons.size(); ++ i) {
        QJsonObject shopWeapon = t_shopWeapons[i].toObject();

        QStringList columnValues;
        columnValues << shopWeapon["weapon_name"].toString();
        columnValues << QString::number(shopWeapon["weapon_price"].toDouble());
        columnValues << QString::number(shopWeapon["weapon_x_range"].toInt());
        columnValues << QString::number(shopWeapon["weapon_y_range"].toInt());

        for (int j = 0; j < columnValues.size(); ++j){
            QTableWidgetItem *item = new QTableWidgetItem(columnValues[j]);
            ui->weaponsShopTableWidget->setItem(i, j, item);
        }
    }
}

void Shop::getWeapons()
{
    QMap<QString, QString> queryParams;

    queryParams["user_id"] = QString::number(m_userId);

    connect(m_manager, &QNetworkAccessManager::finished, this, &Shop::fillWeaponsComboBoxes);
    sendServerRequest("http://127.0.0.1:8000/shop/get_weapons/", queryParams, m_manager);
}

void Shop::buyWeapon()
{
    QList<QTableWidgetItem*> selecteItems = ui->weaponsShopTableWidget->selectedItems();
    if (selecteItems.isEmpty()) {
        return showMessage("Выберите оружие для покупки", QMessageBox::Icon::Information);
    }
    QMap<QString, QString> queryParams;
    QString weaponName = ui->weaponsShopTableWidget->itemAt(0, selecteItems[0]->row())->text();

    queryParams["user_id"] = QString::number(m_userId);
    queryParams["weapon_name"] = weaponName;
    queryParams["weapons_amount"] = ui->weaponAmountSpinBox->text();

    connect(m_manager, &QNetworkAccessManager::finished, this, &Shop::getWeaponBoughtStatus);
    sendServerRequest("http://127.0.0.1:8000/shop/buy_weapon/", queryParams, m_manager);
}

void Shop::sellWeapon()
{

}
