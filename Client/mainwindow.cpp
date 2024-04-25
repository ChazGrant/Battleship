#include "mainwindow.h"
#include "ui_mainwindow.h"


const QMap<QString, QStringList> GAME_OVER_CLAUSES = {
    {"opponent_disconnected", {"", "Противник покинул игру"}},
    {"all_ships_are_dead", {"Ваши корабли были уничтожены", "Вы уничтожили все корабли"}}
};

const int FIELD_ROW_COUNT = 10;
const int FIELD_COLUMN_COUNT = 10;


/*! @brief Конструктор класса
 *
 *  @details Создаются 2 таймера для ожидания хода и ожидания начала игры
 *
 *  @param *parent Указатель на родительский виджет
 *  @param t_game_id Идентификатор игры
 *  @param t_user_id Идентификатор пользователя
 *
 *  @return MainWindow
*/
MainWindow::MainWindow(const QString t_gameId, const int t_userId,
                       const QString t_gameInviteId, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_gameId(t_gameId)
    , m_userId(t_userId)
    , m_gameInviteId(t_gameInviteId)
{
    m_closeEventIsAccepted = false;

    ui->setupUi(this);

    connect(ui->makeTurnButton, &QPushButton::clicked, this, &MainWindow::makeTurn);

    ui->makeTurnButton->hide();
    ui->opponentField->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    m_opponentConnectionTimer = new QTimer();
    m_userTurnTimer = new QTimer();
    m_manager = new QNetworkAccessManager(this);

    ui->opponentField->setSelectionMode(QAbstractItemView::NoSelection);
    ui->opponentField->setMouseTracking(true);

    connect(ui->opponentField, &QTableWidget::itemEntered, this, &MainWindow::highlightOpponentCell);
    connect(ui->opponentField, &QTableWidget::itemClicked, this, &MainWindow::setFirePosition);
    connect(ui->weaponsComboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::setCurrentWeaponAmount);
    connect(ui->activateWeaponCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_weaponActivated = ui->activateWeaponCheckBox->isChecked();
        clearHighlightedCells();
        m_firePosition = {};
    });

    connect(ui->autoPlaceShipsButton, &QPushButton::clicked, this, &MainWindow::autoPlaceShips);

    m_gameStarted = false;

    createTablesWidgets();
    initSockets();

    secondsToConnectPassed = 0;
    secondsToMakeTurnPassed = 0;
    connect(m_opponentConnectionTimer, &QTimer::timeout, this, [=]() {
        onTimeOut(m_opponentConnectionTimer);
    });
    m_opponentConnectionTimer->setInterval(1000);
    // m_opponentConnectionTimer->start();

    setEnabled(false);
}

/*! @brief Закрытие главного окна
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
 */
void MainWindow::acceptCloseEvent(QNetworkReply *t_reply)
{
    m_closeEventIsAccepted = true;
    const QString replyStr = t_reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    getErrorMessage(jsonObj);
    close();
}

/*! @brief Вывод на экран сколько применений оружия осталось
 *
 *  @param t_currentWeaponText Наименование текущего оружия
 *
 *  @return void
*/
void MainWindow::setCurrentWeaponAmount(QString t_currentWeaponText)
{
    ui->weaponUsesLeftLabel->setText("Осталось применений: " +
                                     QString::number(m_availableWeapons[t_currentWeaponText]));
    clearHighlightedCells();
}

/*! @brief Обработчик подключения сокета к серверу
 *
 *  @details Отправляет серверу идентификатор пользователя чтоб подписаться на события
 *
 *  @return void
*/
void MainWindow::onGameSocketConnected()
{
    QJsonObject jsonObj;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::SUBSCRIBE];
    jsonObj["user_id"] = QString::number(m_userId);

    m_gameSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
}

/*! @brief Обработчик отключения сокета от сервера
 *
 *  @return void
*/
void MainWindow::onGameSocketDisconnected()
{
    showMessage("Вы были отключены от сервера", QMessageBox::Icon::Critical);
    close();
}

/*! @brief Обработчик получения информации с сервера через сокет
 *
 *  @param t_textMessage Текст полученного сообщения
 *
 *  @return void
*/
void MainWindow::onGameSocketMessageReceived(QString t_textMessage)
{
    qDebug() << t_textMessage;
    QJsonObject jsonResponse = QJsonDocument::fromJson(t_textMessage.toUtf8()).object();
    if (jsonResponse.contains("error")) {
        return showMessage(jsonResponse["error"].toString(), QMessageBox::Icon::Critical);
    }
    if (!jsonResponse.contains("action_type")) {
        showMessage("Неожиданный ответ от сервера", QMessageBox::Icon::Critical);
        close();
        return;
    }

    QString actionType = jsonResponse["action_type"].toString();
    if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::SHIP_PLACED]) {
        QJsonArray placedCells = jsonResponse["ship_parts_pos"].toArray();
        placeShip(placedCells);
        setShipsAmountLabel(jsonResponse);
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::SUBSCRIBED]) {
        connectToGame();
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::CONNECTED_TO_GAME]) {
        setShipsAmountLabel(jsonResponse);
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::ALL_SHIPS_PLACED]) {
        if (!m_gameStarted) setDisabled(true);
        ui->placeShipButton->hide();
        ui->makeTurnButton->show();
        showMessage("Все корабли были установлены, ожидаем оппонента", QMessageBox::Icon::Information);
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::GAME_STARTED]) {
        int userIdTurn = jsonResponse["user_id_turn"].toInt();
        getAvailableWeapons();
        m_gameStarted = true;
        // setDisabled(true);
        if (userIdTurn == m_userId) {
            setDisabled(false);
            ui->makeTurnButton->setVisible(true);
            ui->placeShipButton->setVisible(false);
            showMessage("Вы начинаете игру", QMessageBox::Icon::Information);
        }
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::AVAILABLE_WEAPONS]) {
        fillWeaponsComboBox(jsonResponse);
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::TURN_MADE]) {
        markOpponentField(jsonResponse);
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::GAME_OVER]) {
        int winnerId = jsonResponse["winner_id"].toInt();
        QString gameOverCause = jsonResponse["game_over_cause"].toString();
        QString winnerString = "Вы проиграли";
        if (winnerId == m_userId) winnerString = "Вы победили";
        showMessage("Игра закончена\n" + winnerString + "\n" + GAME_OVER_CLAUSES[gameOverCause][winnerId==m_userId],
                    QMessageBox::Icon::Information);
        close();
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::OPPONENT_MADE_TURN]) {
        markOwnerField(jsonResponse);
    }
}

/*! @brief Обработчик ошибки, полученной во время отправки запроса на сервер через сокет
 *
 *  @param t_socketError Вид ошибки, полученной при передачи данных
 *
 *  @return void
*/
void MainWindow::onGameSocketErrorOccurred(QAbstractSocket::SocketError t_socketError)
{
    if (t_socketError == QAbstractSocket::SocketError::ConnectionRefusedError) {
        showMessage("Не удалось подключиться к серверу", QMessageBox::Icon::Critical);
        close();
    }
}

/*! @brief Обработчик подключения сокета к серверу
 *
 *  @details Отправляет серверу идентификатор пользователя чтоб подписаться на события
 *
 *  @return void
*/
void MainWindow::onChatSocketConnected()
{

}

/*! @brief Обработчик получения информации с сервера через сокет
 *
 *  @param t_textMessage Текст полученного сообщения
 *
 *  @return void
*/
void MainWindow::onChatSocketMessageReceived(QString t_textMessage)
{

}

/*! @brief Обработчик ошибки, полученной во время отправки запроса на сервер через сокет
 *
 *  @param t_socketError Вид ошибки, полученной при передачи данных
 *
 *  @return void
*/
void MainWindow::onChatSocketErrorOccurred(QAbstractSocket::SocketError t_socketError)
{

}

/*! @brief Обновление оставшегося времени для подключения или совершения хода
 *
 *  @param *t_timer Указатель на таймер, на котором прошло определённое время
 *
 *  @return void
*/
void MainWindow::onTimeOut(QTimer *t_timer)
{
    int timeRemain;
    if (t_timer == m_opponentConnectionTimer) {
        ++secondsToConnectPassed;
        timeRemain = 60 - secondsToConnectPassed;
        ui->opponentWaitingRemainingTimeLabel->setText("Осталось времени для подключения: " + \
            QString("0:%1%2").arg((QString::number(timeRemain).size() > 1) ? "" : "0", QString::number(timeRemain)));
    } else if (t_timer == m_userTurnTimer) {
        ++secondsToMakeTurnPassed;
        timeRemain = 60 - secondsToConnectPassed;
        ui->yourTurnTimeRemainLabel->setText("Осталось времени на ход: " + \
            QString("0:%1%2").arg((QString::number(timeRemain).size() > 1) ? "" : "0", timeRemain));
    }

    if (timeRemain == 0) {
        if (t_timer == m_opponentConnectionTimer) {
            showMessage("Игрок не подключился", QMessageBox::Icon::Critical);
        } else if (t_timer == m_userTurnTimer) {
            showMessage("Ваше время на ход истекло", QMessageBox::Icon::Critical);
        }
        close();
    }
}

/*! @brief Инициалиализация сокетов
 *
 *  @return void
*/
void MainWindow::initSockets()
{
    initGameSocket();
    // initChatSocket();
}

/*! @brief Создание сокета для обновления состояний игры и событий к нему
 *
 *  @return void
*/
void MainWindow::initGameSocket()
{
    m_gameSocket = new QWebSocket();

    m_gameSocketUrl.setPort(8080);
    m_gameSocketUrl.setHost("127.0.0.1");
    m_gameSocketUrl.setPath("/game/");
    m_gameSocketUrl.setScheme("ws");

    m_gameSocket->open(m_gameSocketUrl);

    connect(m_gameSocket, SIGNAL(connected()), this, SLOT(onGameSocketConnected()));
    connect(m_gameSocket, SIGNAL(disconnected()), this, SLOT(onGameSocketDisconnected()));
    connect(m_gameSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onGameSocketErrorOccurred(QAbstractSocket::SocketError)));
    connect(m_gameSocket, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onGameSocketMessageReceived(QString)));
}

/*! @brief Создание сокета для обновления чата и событий к нему
 *
 *  @return void
*/
void MainWindow::initChatSocket()
{
    m_chatSocket = new QWebSocket();

    m_chatSocketUrl.setPort(8080);
    m_chatSocketUrl.setHost("127.0.0.1");
    m_chatSocketUrl.setPath("/chat/");
    m_chatSocketUrl.setScheme("ws");

    m_chatSocket->open(m_chatSocketUrl);

    connect(m_chatSocket, SIGNAL(connected()), this, SLOT(onChatSocketConnected()));
    connect(m_chatSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onChatSocketErrorOccurred(QAbstractSocket::SocketError)));
    connect(m_chatSocket, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onChatSocketMessageReceived(QString)));
}

void MainWindow::connectToGame()
{
    QJsonObject jsonObj;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::CONNECT_TO_GAME];
    jsonObj["user_id"] = QString::number(m_userId);
    jsonObj["game_id"] = m_gameId;
    jsonObj["game_invite_id"] = m_gameInviteId;

    m_gameSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
}

//void MainWindow::__createField()
//{
//    QJsonObject jsonObj;
//    jsonObj["action_type"] = "create_field";
//    jsonObj["user_id"] = QString::number(m_userId);
//    jsonObj["game_id"] = m_gameId;

//    m_gameSocket->sendTextMessage(jsonObjectToQString(jsonObj));
//}

/*! @brief Выделение ячейки поля оппонента
 *
 *  @details Выделяет ячейку, на которое наведена мышь, жёлтым цветом
 *
 *  @param *t_item Указатель на ячейку поля
 *
 *  @return void
 *
 *  @todo Разобраться в алгоритме пометки при использовании оружия
*/
void MainWindow::highlightOpponentCell(QTableWidgetItem *t_item)
{
    clearHighlightedCells(GREEN);
    m_lastHighlightedItem = t_item;
    if (t_item->backgroundColor() != WHITE ||
        t_item->backgroundColor() == ORANGE ||
        t_item->backgroundColor() == GRAY ||
        t_item->backgroundColor() == RED) {
        return;
    }

    paintOpponentCells(t_item->row(), t_item->column(), YELLOW);
}

void MainWindow::paintOpponentCells(int t_xStart, int t_yStart, QColor t_color) {
    int xRange, yRange;
    if (m_weaponActivated) {
        xRange = m_weaponRange[ui->weaponsComboBox->currentText()][1];
        yRange = m_weaponRange[ui->weaponsComboBox->currentText()][0];
        qDebug() << xRange << yRange;
    } else {
        xRange = 1;
        yRange = 1;
    }

    for (int row = t_yStart; row < t_yStart + yRange; ++row) {
        for (int column = t_xStart; column < t_xStart + xRange; ++column) {
            QTableWidgetItem *item = ui->opponentField->item(column, row);
            if (item == nullptr) {
                continue;
            }

            QColor currentItemColor = item->background().color();
            if (currentItemColor == WHITE || currentItemColor == YELLOW) {
                item->setBackgroundColor(t_color);
            }
        }
    }
}

/*! @brief Пометка ячейки для выстрела
 *
 *  @param *t_item Указатель на ячейку поля
 *
 *  @return void
*/
void MainWindow::setFirePosition(QTableWidgetItem *t_item)
{
    if (m_lastMarkedItem == t_item) return;
    clearHighlightedCells(YELLOW);
    if (m_lastMarkedItem != nullptr) {
        m_lastMarkedItem->setBackgroundColor(WHITE);
    }

    if (t_item->backgroundColor() == YELLOW) {
        m_lastMarkedItem = t_item;
        m_firePosition = { t_item->column(), t_item->row() };
    }

    paintOpponentCells(t_item->row(), t_item->column(), GREEN);

}

/*! @brief Удаление цвета с помеченных клеток
 *
 *  @details Меняет цвет всех жёлтых клеток на белый
 *
 *  @return void
*/
void MainWindow::clearHighlightedCells(QColor t_avoidColor)
{
    for (int x = 0; x < FIELD_ROW_COUNT; ++x) {
        for (int y = 0; y < FIELD_COLUMN_COUNT; ++y) {
            if (ui->opponentField->item(x, y)->backgroundColor() == ORANGE ||
                ui->opponentField->item(x, y)->backgroundColor() == GRAY ||
                ui->opponentField->item(x, y)->backgroundColor() == RED ||
                ui->opponentField->item(x, y)->backgroundColor() == t_avoidColor) {
                continue;
            }
            QTableWidgetItem *currentItem = ui->opponentField->item(x, y);
            if (currentItem != nullptr) {
                currentItem->setBackgroundColor(WHITE);
            }
        }
    }
    if (t_avoidColor == YELLOW) {
        m_lastMarkedItem = nullptr;
        m_firePosition = {};
    }
}

/*! @brief Обработка сигнала закрытия окна
 *
 *  @details На сервер отсылается запрос для отключения от текущей игры. В параметры передаются
 *  идентификатор игры и пользователя
 *
 *  @param *event Указатель на тип события
 *
 *  @return void
*/
void MainWindow::closeEvent(QCloseEvent *event)
{
    // Пока не нашёл альтернативы как это сделать более грамотно,
    // поэтому пока так
    event->accept();
    return;
    if (this->m_closeEventIsAccepted) {
        return event->accept();
    }

    QUrl url("http://127.0.0.1:8000/games/disconnect/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("user_id", QString::number(this->m_userId));
    query.addQueryItem("game_id", this->m_gameId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::acceptCloseEvent);
    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
    event->ignore();
}

/*! @brief Получение текста ошибки из json object'а
 *
 *  @param t_json_obj Сам объект, который может содержать текст ошибки
 *
 *  @return bool
*/
bool MainWindow::getErrorMessage(QJsonObject t_json_obj)
{
    if (t_json_obj.contains("Error")) {
        showMessage(t_json_obj["Error"].toString(), QMessageBox::Icon::Critical);
        return true;
    }
    if (t_json_obj.contains("Critical Error")) {
        // m_timerForUserTurn->stop();
        showMessage(t_json_obj["Critical Error"].toString(), QMessageBox::Icon::Critical);
        this->close();
        return true;
    }

    return false;
}

/*! @brief Создание пол пользователя и оппонента
 *
 *  @return void
*/
void MainWindow::createTablesWidgets()
{
    createUserTable();
    createOpponentTable();
}

/*! @brief Вывод на экран пользователя количество оставшихся кораблей
 *
 *  @param t_jsonResponse Json, содержащий количество оставшихся кораблей
 *
 *  @return void
*/
void MainWindow::setShipsAmountLabel(QJsonObject t_jsonResponse)
{
    QString totalShipsLeft = "Осталось кораблей:\n";
    totalShipsLeft += "1: " + QString::number(t_jsonResponse["one_deck_left"].toInt()) + "\n";
    totalShipsLeft += "2: " + QString::number(t_jsonResponse["two_deck_left"].toInt()) + "\n";
    totalShipsLeft += "3: " + QString::number(t_jsonResponse["three_deck_left"].toInt()) + "\n";
    totalShipsLeft += "4: " + QString::number(t_jsonResponse["four_deck_left"].toInt()) + "\n";

    ui->shipsAmountLabel->setText(totalShipsLeft);
    this->setDisabled(false);
}

/*! @brief Создание пустого поля пользователя
 *
 *  @details Поле создаётся путём добавления туда ячеек содержащих пробел, чтоб ячейки
 *  существовали и могли иметь задний фон
 *
 *  @return void
*/
void MainWindow::createUserTable()
{
    ui->ownerField->setRowCount(FIELD_ROW_COUNT);
    ui->ownerField->setColumnCount(FIELD_COLUMN_COUNT);
    for (int y = 0; y < FIELD_ROW_COUNT; ++y) {
        for (int x = 0;x < FIELD_COLUMN_COUNT; ++x) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setText(QString(" "));

            item->setBackground(WHITE);
            ui->ownerField->setItem(y, x, item);
        }
    }

    ui->ownerField->resizeColumnsToContents();
    ui->ownerField->resizeRowsToContents();
    ui->ownerField->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
}

/*! @brief Создание пустого поля оппонента
 *
 *  @details Поле создаётся путём добавления туда ячеек содержащих пробел, чтоб ячейки
 *  существовали и могли иметь задний фон
 *
 *  @return void
*/
void MainWindow::createOpponentTable()
{
    ui->opponentField->setRowCount(FIELD_ROW_COUNT);
    ui->opponentField->setColumnCount(FIELD_COLUMN_COUNT);
    for (int y = 0; y < FIELD_ROW_COUNT; ++y) {
        for (int x = 0;x < FIELD_COLUMN_COUNT; ++x) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setText(QString::number(x + y));

            item->setBackground(WHITE);
            ui->opponentField->setItem(y, x, item);
        }
    }

    ui->opponentField->resizeColumnsToContents();
    ui->opponentField->resizeRowsToContents();
    ui->opponentField->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
}

void MainWindow::getAvailableWeapons()
{
    qDebug() << "getAvailableWeapons";
    QJsonObject jsonObj;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::GET_WEAPONS];
    jsonObj["user_id"] = QString::number(m_userId);

    m_gameSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
}

/*! @brief Заполнение comboBox'а оружиями в наличии
 *
 *  @return void
 *
 *  @todo Брать эту информацию от сервера
*/
void MainWindow::fillWeaponsComboBox(QJsonObject jsonObj)
{
    QJsonObject availableWeapons = jsonObj["available_weapons"].toObject();
    foreach(const QString& weaponName, availableWeapons.keys()) {
        QJsonObject availableWeapon = availableWeapons[weaponName].toObject();
        m_availableWeapons[weaponName] = availableWeapon["weapon_amount"].toInt();
        ui->weaponsComboBox->addItem(weaponName);
        int xRange = availableWeapon["weapon_x_range"].toInt();
        int yRange = availableWeapon["weapon_y_range"].toInt();
        m_weaponRange[weaponName] = {xRange, yRange};
    }
}

void MainWindow::autoPlaceShips()
{
    setDisabled(true);
    m_shipsAutoPlacing = true;

    ui->ownerField->clear();
    QJsonObject jsonObj;
    jsonObj["action_type"] = "generate_field";
    jsonObj["user_id"] = QString::number(m_userId);
    jsonObj["game_id"] = m_gameId;

    m_gameSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
}

//! @brief Деструктор класса
MainWindow::~MainWindow()
{
    delete ui;
}

/*! @brief Установка корабля на поле
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainWindow::placeShip(QJsonArray t_cells)
{
    for (int i = 0; i < t_cells.size(); ++i) {
        QJsonArray currentCell = t_cells[i].toArray();
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setBackground(GREEN);
        ui->ownerField->setItem(currentCell[1].toInt(), currentCell[0].toInt(), item);
        item->setSelected(false);
    }
    if (!m_shipsAutoPlacing) {
        setDisabled(false);
    }
}

/*! @brief Обработка события на кнопке "Установить корабль"
 *
 *  @details В запрос передаётся json, по примеру
 *  cells : [[2,3],[4,5],[1,2]]
 *  owner_id: foaishf0bhas9-f09
 *  game_id: 15812571257
 *
 *  @return void
*/
void MainWindow::on_placeShipButton_clicked()
{
    if (ui->ownerField->selectedItems().isEmpty()) {
        return showMessage("Ячейки не выбраны", QMessageBox::Icon::Critical);
    }

    this->setDisabled(true);

    QJsonArray cells;
    QList<QTableWidgetItem*> selectedItems = ui->ownerField->selectedItems();
    QListIterator<QTableWidgetItem*> itemsIterator(selectedItems);

    while (itemsIterator.hasNext())
    {
        QTableWidgetItem* currentItem = itemsIterator.next();
        QJsonArray xyCells;
        xyCells.append(QString::number(currentItem->column()));
        xyCells.append(QString::number(currentItem->row()));

        cells.append(xyCells);
    }

    QJsonObject jsonObj;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::PLACE_SHIP];
    jsonObj["user_id"] = QString::number(m_userId);
    jsonObj["game_id"] = m_gameId;
    jsonObj["cells"] = cells;

    m_gameSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
}

/*! @brief Выстрел по клетке
 *
 *  @details Отправляем запрос на сервер, блокируя интерфейс
 *  Если ошибок нет, то проверяем на попадание
 *  Если не попал, то ожидаем своего хода
 *  Если попал, то даём ещё один ход
 *  Если убил, то проверяем на конец игры
 *
 *  @return void
*/
void MainWindow::makeTurn()
{
    if (m_firePosition.size() == 0)
        return showMessage("Клетки не были выбраны", QMessageBox::Icon::Critical);

    QJsonObject jsonObj;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::MAKE_TURN];
    jsonObj["user_id"] = QString::number(m_userId);
    jsonObj["game_id"] = m_gameId;
    if (m_weaponActivated) {
        jsonObj["weapon_name"] = ui->weaponsComboBox->currentText();
    }
    jsonObj["shoot_position"] = m_firePosition;

    clearHighlightedCells();
    m_lastHighlightedItem = nullptr;
    m_lastMarkedItem = nullptr;
    m_firePosition = {};

    m_gameSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
    // setDisabled(true);
}

void MainWindow::markOpponentField(QJsonObject t_jsonObj)
{
    QJsonArray damagedCells = t_jsonObj["damaged_cells"].toArray();
    QJsonArray deadCells = t_jsonObj["dead_cells"].toArray();
    QJsonArray missedCells = t_jsonObj["missed_cells"].toArray();

    int x, y;
    for (int i = 0; i < damagedCells.size(); ++i) {
        QJsonArray xYPos = damagedCells[i].toArray();
        x = QString::number(xYPos[0].toDouble()).toInt();
        y = QString::number(xYPos[1].toDouble()).toInt();

        QTableWidgetItem *item = new QTableWidgetItem("");
        item->setBackground(ORANGE);
        ui->opponentField->setItem(y, x, item);
    }

    for (int i = 0; i < deadCells.size(); ++i) {
        QJsonArray xYPos = deadCells[i].toArray();
        x = QString::number(xYPos[0].toDouble()).toInt();
        y = QString::number(xYPos[1].toDouble()).toInt();

        QTableWidgetItem *item = new QTableWidgetItem("");
        item->setBackground(RED);
        ui->opponentField->setItem(y, x, item);
    }

    for (int i = 0; i < missedCells.size(); ++i) {
        QJsonArray xYPos = missedCells[i].toArray();
        x = static_cast<int>(xYPos[0].toDouble());
        y = static_cast<int>(xYPos[1].toDouble());

        QTableWidgetItem *item = new QTableWidgetItem("");
        item->setBackground(GRAY);
        ui->opponentField->setItem(y, x, item);
    }

    if (deadCells.size() || damagedCells.size()) {
        setDisabled(false);
    }

    if (t_jsonObj.contains("weapon_name")) {
        QString weaponName = t_jsonObj["weapon_name"].toString();
        int weaponAmountLeft = t_jsonObj["weapon_amount_left"].toInt();

        m_availableWeapons[weaponName] = weaponAmountLeft;
        setCurrentWeaponAmount(ui->weaponsComboBox->currentText());
    }
}

void MainWindow::markOwnerField(QJsonObject t_jsonObj)
{
    QJsonArray damagedCells = t_jsonObj["damaged_cells"].toArray();
    QJsonArray deadCells = t_jsonObj["dead_cells"].toArray();
    QJsonArray missedCells = t_jsonObj["missed_cells"].toArray();
    int userIdTurn = t_jsonObj["user_id_turn"].toInt();

    int x, y;
    for (int i = 0; i < damagedCells.size(); ++i) {
        QJsonArray xYPos = damagedCells[i].toArray();
        x = QString::number(xYPos[0].toDouble()).toInt();
        y = QString::number(xYPos[1].toDouble()).toInt();

        QTableWidgetItem *item = new QTableWidgetItem("");
        item->setBackground(ORANGE);
        ui->ownerField->setItem(y, x, item);
    }

    for (int i = 0; i < deadCells.size(); ++i) {
        QJsonArray xYPos = deadCells[i].toArray();
        x = QString::number(xYPos[0].toDouble()).toInt();
        y = QString::number(xYPos[1].toDouble()).toInt();

        QTableWidgetItem *item = new QTableWidgetItem("");
        item->setBackground(RED);
        ui->ownerField->setItem(y, x, item);
    }

    for (int i = 0; i < missedCells.size(); ++i) {
        QJsonArray xYPos = missedCells[i].toArray();
        x = static_cast<int>(xYPos[0].toDouble());
        y = static_cast<int>(xYPos[1].toDouble());

        QTableWidgetItem *item = new QTableWidgetItem("");
        item->setBackground(GRAY);
        ui->ownerField->setItem(y, x, item);
    }

    if (userIdTurn == m_userId) {
        setDisabled(false);
    }
}
