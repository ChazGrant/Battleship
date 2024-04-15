#include "mainwindow.h"
#include "ui_mainwindow.h"


const QColor WHITE = QColor("white");
const QColor GREEN = QColor("darkgreen");
const QColor YELLOW = QColor("yellow");
const QColor RED = QColor("darkred");
const QColor GRAY = QColor("darkgray");

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
    this->m_closeEventIsAccepted = false;

    ui->setupUi(this);

    ui->gameIdLabel->setText("ID игры: " + m_gameId);
    ui->userIdLabel->setText("Ваш ID: " + QString::number(m_userId));

    connect(ui->makeTurnButton, &QPushButton::clicked, this, &MainWindow::makeTurn);

    // ui->fireButton->hide();
    ui->opponentField->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    m_oponnentConnectionTimer = new QTimer();
    m_userTurnTimer = new QTimer();
    m_manager = new QNetworkAccessManager(this);
    m_lastHighlightedItem = nullptr;
    m_lastMarkedItem = nullptr;
    m_weaponActivated = true;

    ui->opponentField->setSelectionMode(QAbstractItemView::NoSelection);
    ui->opponentField->setMouseTracking(true);

//    connect(ui->opponentField, &QTableWidget::itemEntered, this, &MainWindow::highlightOpponentCell);
//    connect(ui->opponentField, &QTableWidget::itemClicked, this, &MainWindow::markOpponentCell);
    connect(ui->weaponsComboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::setWeaponsUsesLeftLabel);
    connect(ui->activateWeaponButton, &QPushButton::clicked, this, [=]() {
        m_weaponActivated = !m_weaponActivated;
    });

    createTablesWidgets();
    initSockets();
    fillWeaponsComboBox();

    // this->getShipsAmountResponse();
}

/*! @brief Получение идентификатора пользователя, который должен делать ход
 *
 *  @details Сейчас ход текущего пользователя, то останавливается таймер ожидания хода
 *  и получаются повреждённые клетки
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainWindow::getUserIdTurn(QNetworkReply *reply)
{
    const QString replyStr = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    if (this->getErrorMessage(jsonObj))
    {
        return;
    }

    if (jsonObj["user_id_turn"].toString() == this->m_userId)
    {
        // Отключаем таймер и событие
        // m_timerForUserTurn->stop();
        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getUserIdTurn(QNetworkReply* )));
        // Получаем клетки, по которым попал соперник
        this->getDamagedCells();
    }
}

/*! @brief Ожидание хода текущего пользователя
 *
 *  @details Отправляется запрос на сервер, в параметры передаётся идентификатор игры
 *
 *  @return void
*/
void MainWindow::waitForTurn()
{
    QUrl url("http://127.0.0.1:8000/games/get_user_id_turn/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("game_id", this->m_gameId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getUserIdTurn(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}

/*! @brief Заполнение поля
 *
 *  @details С json object'а получаем повреждённые части кораблей и мёртвые корабли
 *  и закрашиваем их на поле
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
 *
 *  @todo Решить проблему из-за которой иногда может прийти ответ от getUserIdTurn
*/
void MainWindow::fillField(QNetworkReply *reply)
{
    const QString replyStr = reply->readAll();
    qDebug() << "GET FILL FIELD";
    qDebug() << replyStr;

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    /*
     * Костыль, т.к. непонятно почему иногда принимает нужный ответ, а иногда ответ от getUserIdTurn
    */

    // Если ответ пришёл от нужного url, то заполняем данные
    // Иначе послыаем ещё один запрос
    if (jsonObj.contains("user_id_turn"))
    {
        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(fillField(QNetworkReply* )));
        return this->getDamagedCells();
    }

    // Закрашиваем повреждённые клетки
    QJsonArray marked_cells = jsonObj["marked_cells"].toArray();
    qDebug() << marked_cells;
    if (!marked_cells.isEmpty())
    {
        for (int i = 0; i < marked_cells.size(); ++i)
        {
            QStringList currentCell = marked_cells[i].toString().split(" ");

            int x = currentCell[0].toInt();
            int y = currentCell[1].toInt();

            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setBackground(GRAY);
            ui->yourField->setItem(y, x, item);
        }
    }

    // Закрашиваем мёртвые корабли
    QJsonArray deadParts = jsonObj["dead_parts"].toArray();
    qDebug() << deadParts;
    if (!deadParts.isEmpty())
    {
        for (int i = 0; i < deadParts.size(); ++i)
        {
            QStringList currentCell = deadParts[i].toString().split(" ");

            int x = currentCell[0].toInt();
            int y = currentCell[1].toInt();

            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setBackground(RED);
            ui->yourField->setItem(y, x, item);
        }
    }

    // Закрашиваем повреждённые корабли
    QJsonArray damagedParts = jsonObj["damaged_parts"].toArray();
    qDebug() << damagedParts;
    if (!damagedParts.isEmpty())
    {
        for (int i = 0; i < damagedParts.size(); ++i)
        {
            QStringList currentCell = damagedParts[i].toString().split(" ");

            int x = currentCell[0].toInt();
            int y = currentCell[1].toInt();

            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setBackground(YELLOW);
            ui->yourField->setItem(y, x, item);
        }
    }

    // Отписываемся от события
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(fillField(QNetworkReply* )));

    // Проверяем, игра окончена или нет
    this->checkForWinner();
    // "Включаем интерфейс"
    this->setDisabled(false);
}

/*! @brief Получение всех клеток, по которым противник стрелял
 *
 *  @details Отправляется запрос на сервер. В параметрах передаётся идентификатор текущего пользователя
 *
 *  @return void
*/
void MainWindow::getDamagedCells()
{
    QUrl url("http://127.0.0.1:8000/fields/get_damaged_cells/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("user_id", QString::number(this->m_userId));

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(fillField(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}

/*! @brief Получение победителя игры
 *
 *  @details Если json object содержит параметр с победителем, то он выводится на экран и
 *  игра завершается
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainWindow::getWinner(QNetworkReply *reply)
{
    const QString replyStr = reply->readAll();
    qDebug() << "GET GAME OVER STATE";
    qDebug() << replyStr;

    QJsonObject jsonObject = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    if (jsonObject["game_is_over"].toBool())
    {
        qDebug() << "WINNER OF THE GAME";
        qDebug() << "YOUR ID " << this->m_userId;
        qDebug() << "WINNER ID " << jsonObject["winner"].toString();

        if (jsonObject["winner"].toString() == this->m_userId)
        {
            showMessage("Игра закончена и победу одержали Вы", QMessageBox::Icon::Information);
        }
        else
        {
            showMessage("Игра закончена и Вы проиграли", QMessageBox::Icon::Information);
        }
        this->close();
    }

    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getWinner(QNetworkReply* )));
}

/*! @brief Проверка победителя
 *
 *  @details На сервер отправляется запрос для получения победителя игры. В параметры
 *  передаются идентификатор игры и пользователя
 *
 *  @return void
*/
void MainWindow::checkForWinner()
{
    QUrl url("http://127.0.0.1:8000/games/game_is_over/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("game_id", this->m_gameId);
    query.addQueryItem("user_id", QString::number(this->m_userId));

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getWinner(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
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
void MainWindow::setWeaponsUsesLeftLabel(QString t_currentWeaponText)
{
    ui->weaponUsesLeftLabel->setText("Осталось применений: " +
                                     QString::number(m_availableWeapons[t_currentWeaponText]));
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

    m_gameSocket->sendTextMessage(jsonObjectToQString(jsonObj));
}

/*! @brief Обработчик отключения сокета от сервера
 *
 *  @return void
*/
void MainWindow::onGameSocketDisconnected()
{

}

/*! @brief Обработчик получения информации с сервера через сокет
 *
 *  @param t_textMessage Текст полученного сообщения
 *
 *  @return void
*/
void MainWindow::onGameSocketMessageReceived(QString t_textMessage)
{

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

/*! @brief Обработчик отключения сокета от сервера
 *
 *  @return void
*/
void MainWindow::onChatSocketDisconnected()
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

/*! @brief Инициалиализация сокетов
 *
 *  @return void
*/
void MainWindow::initSockets()
{
    initGameSocket();
    initChatSocket();
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
    m_chatSocketUrl.setPath("/game/");
    m_chatSocketUrl.setScheme("ws");

    m_chatSocket->open(m_chatSocketUrl);

    connect(m_chatSocket, SIGNAL(connected()), this, SLOT(onChatSocketConnected()));
    connect(m_chatSocket, SIGNAL(disconnected()), this, SLOT(onChatSocketDisconnected()));
    connect(m_chatSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onChatSocketErrorOccurred(QAbstractSocket::SocketError)));
    connect(m_chatSocket, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onChatSocketMessageReceived(QString)));
}

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
    // DEBUG
//    for (int x = 0; x < FIELD_ROW_COUNT; ++x) {
//        for (int y = 0; y < FIELD_COLUMN_COUNT; ++y) {
//            ui->opponentField->itemAt(x, y)->setBackgroundColor(WHITE);
//        }
//    }
    if (m_lastHighlightedItem != nullptr && m_lastHighlightedItem->backgroundColor() != RED) {
        // m_lastHighlightedItem->setBackgroundColor(WHITE);
    }

    m_lastHighlightedItem = t_item;
    // if (item->backgroundColor() != WHITE) return;
    if (m_weaponActivated) {
        int xOffset = m_weaponSelection[ui->weaponsComboBox->currentText()]["x"];
        int yOffset = m_weaponSelection[ui->weaponsComboBox->currentText()]["y"];


        int y = t_item->column();

        for (int x = 0; x < 0 + xOffset; ++x) {
            qDebug() << x;
            QTableWidgetItem *itemToHighlight = ui->opponentField->itemAt(x, y);
            if (itemToHighlight != nullptr)
                itemToHighlight->setBackgroundColor(YELLOW);
        }
//        for (; x < x + xOffset; ++x) {
//            for (; y < y + yOffset; ++y) {
//                qDebug() << x << y;
//                QTableWidgetItem *itemToHighlight = ui->opponentField->itemAt(x, y);
//                if (itemToHighlight != nullptr)
//                    itemToHighlight->setBackgroundColor(YELLOW);
//            }
//        }
    } else {
        t_item->setBackgroundColor(YELLOW);
    }
}

/*! @brief Пометка ячейки для выстрела
 *
 *  @param *t_item Указатель на ячейку поля
 *
 *  @return void
*/
void MainWindow::markOpponentCell(QTableWidgetItem *t_item)
{
    if (m_lastMarkedItem != nullptr) {
        m_lastMarkedItem->setBackgroundColor(WHITE);
    }

    m_lastMarkedItem = t_item;
    t_item->setBackgroundColor(RED);

    m_firePosition["x"] = t_item->row();
    m_firePosition["y"] = t_item->column();
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
    if (this->m_closeEventIsAccepted)
        return event->accept();

    qDebug() << "closeEvent called";
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
    if (t_json_obj.contains("Error"))
    {
        showMessage(t_json_obj["Error"].toString(), QMessageBox::Icon::Critical);
        return true;
    }
    if (t_json_obj.contains("Critical Error"))
    {
        // m_timerForUserTurn->stop();
        QObject::disconnect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::getUserIdTurn);

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

/*! @brief Получение текущего состояние игры(началась она или нет)
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainWindow::getGameState(QNetworkReply* reply)
{
    const QString replyStr = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    if (jsonObj["game_is_started"].toBool())
    {
        // Останавливаем прошлый таймер и отключаем сигнал
        // m_timerForGameStart->stop();
        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getGameState(QNetworkReply* )));
        // m_timerForUserTurn->start();

        ui->makeTurnButton->show();
        ui->placeShipButton->hide();
        showMessage("Игра начата, ожидайте свой ход", QMessageBox::Icon::Information);
    }

}

/*! @brief Ожидание начала играы
 *
 *  @details Отсылается на сервер по истечению времени таймера
 *
 *  @return void
*/
void MainWindow::waitForGameStart()
{
    QUrl url("http://127.0.0.1:8000/games/game_is_started/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("game_id", this->m_gameId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getGameState(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}

/*! @brief Вывод на экран пользователя количество оставших кораблей
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainWindow::setShipsAmountLabel(QNetworkReply* reply)
{
    const QString replyStr = reply->readAll();
    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    if (this->getErrorMessage(jsonObj))
        return;

    qDebug() << jsonObj["four_deck"].toInt();
    int fourDeckLeft = jsonObj["four_deck"].toInt();
    int threeDeckLeft = jsonObj["three_deck"].toInt();
    int twoDeckLeft = jsonObj["two_deck"].toInt();
    int oneDeckLeft = jsonObj["one_deck"].toInt();

    QString shipsAmountStr = "";
    shipsAmountStr = "4: " + QString::number(fourDeckLeft) + "\n" +\
                     "3: " + QString::number(threeDeckLeft) + "\n" +\
                     "2: " + QString::number(twoDeckLeft) + "\n" +\
                     "1: " + QString::number(oneDeckLeft);

    ui->shipsAmountLabel->setText(shipsAmountStr);
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* )), this, SLOT(setShipsAmountLabel(QNetworkReply* )));

    if ((fourDeckLeft + threeDeckLeft + twoDeckLeft + oneDeckLeft) == 0)
    {
        showMessage("Корабли закончились, ожидаем соперника", QMessageBox::Icon::Information);
        this->setDisabled(true);
        // Ставим таймер на функцию, в которой ожидаем когда игра будет начата

        //  m_timerForGameStart->start();
    }
}

/*! @brief Получение количества оставшихся кораблей
 *
 *  @details Делает запрос на сервер чтобы получить количество каждого типа кораблей, оставшегося
 *  для расстановки на поле. В параметры передаётся идентификатор пользователя(владельца поля)
 *
 *  @return void
*/
void MainWindow::getShipsAmountResponse()
{
    QUrl url("http://127.0.0.1:8000/fields/get_field/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("owner_id", QString::number(this->m_userId));

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ),
                     this, SLOT(setShipsAmountLabel(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
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
    ui->yourField->setRowCount(FIELD_ROW_COUNT);
    ui->yourField->setColumnCount(FIELD_COLUMN_COUNT);
    for (int y = 0; y < FIELD_ROW_COUNT; ++y)
        for (int x = 0;x < FIELD_COLUMN_COUNT; ++x) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setText(QString(" "));

            item->setBackground(WHITE);
            ui->yourField->setItem(y, x, item);
        }

    ui->yourField->resizeColumnsToContents();
    ui->yourField->resizeRowsToContents();
    ui->yourField->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
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
    for (int y = 0; y < FIELD_ROW_COUNT; ++y)
        for (int x = 0;x < FIELD_COLUMN_COUNT; ++x) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setText(QString::number(x + y));

            item->setBackground(WHITE);
            ui->opponentField->setItem(y, x, item);
        }

    ui->opponentField->resizeColumnsToContents();
    ui->opponentField->resizeRowsToContents();
    ui->opponentField->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
}

/*! @brief Заполнение comboBox'а оружиями в наличии
 *
 *  @return void
 *
 *  @todo Брать эту информацию от сервера
*/
void MainWindow::fillWeaponsComboBox()
{
    QMapIterator<QString, int> iterator(m_availableWeapons);
    int i = 0;
    while(iterator.hasNext()) {
        iterator.next();
        QString weaponName = iterator.key();
        ui->weaponsComboBox->addItem(weaponName);
        ++i;
    }
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
void MainWindow:: placeShip(QNetworkReply* reply)
{
    const QString replyStr = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    // Могут возникнуть ошибки, такие как коллизии, корабль слишком длинный, широкий, или корабли закончились
    if (getErrorMessage(jsonObj))
        return;

    QJsonArray cells = jsonObj["cells"].toArray();

    for (int i = 0; i < cells.size(); ++i)
    {
        QJsonArray currentCell = cells[i].toArray();
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setBackground(GREEN);
        ui->yourField->setItem(currentCell[1].toInt(), currentCell[0].toInt(), item);
        item->setSelected(false);
    }
    QObject::disconnect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::placeShip);
    this->getShipsAmountResponse();
    this->setDisabled(false);
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
    if (!(ui->yourField->selectedItems().length()))
        return showMessage("Ячейки не выбраны", QMessageBox::Icon::Critical);

    this->setDisabled(true);

    QUrl url("http://127.0.0.1:8000/fields/place_ship/");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QListIterator<QTableWidgetItem *> iter(ui->yourField->selectedItems());

    QString values = "";
    while (iter.hasNext())
    {
        QTableWidgetItem* currentItem = iter.next();

        QString value = QString::number(currentItem->column()) + "," + QString::number(currentItem->row());
        values += value;
        values += " ";
    }
    QByteArray json = "{ \"cells\":\"" + values.toUtf8() + \
            "\", \"owner_id\":\"" + QString::number(this->m_userId).toUtf8() + \
            "\", \"game_id\":\"" +  this->m_gameId.toUtf8() + "\" }";


    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(placeShip(QNetworkReply* )));
    m_manager->post(request, json);
}

/*! @brief Получение статус куда попал пользователь
 *
 *  @details Получает клетки, по которым промахнулись
 *  Получаем клетки, по которым попали
 *  Получаем корабли, которые убили
 *  Если убили, то проверяем на конец хода
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainWindow::getFireStatus(QNetworkReply *reply)
{
    ui->opponentField->clearSelection();
    const QString replyStr = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    if (getErrorMessage(jsonObj))
        return this->setDisabled(false);

    // Не попали, закрашиваем серым
    if (jsonObj.contains("missed"))
    {
        QStringList missedCell = jsonObj["missed_cell"].toString().split(" ");

        int x = missedCell[0].toInt();
        int y = missedCell[1].toInt();

        QTableWidgetItem* item = new QTableWidgetItem("");
        item->setBackground(GRAY);

        ui->opponentField->setItem(y, x, item);

        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getFireStatus(QNetworkReply* )));
        // Ожидаем пока оппонент сделает ход
        // m_timerForUserTurn->start();
    }

    // Закрашиваем либо жёлтым цветом
    else if (jsonObj.contains("ship_is_damaged"))
    {
        QStringList damagedPart = jsonObj["damaged_part"].toString().split(" ");

        int x = damagedPart[0].toInt();
        int y = damagedPart[1].toInt();

        QTableWidgetItem* item = new QTableWidgetItem("");
        item->setBackground(YELLOW);
        ui->opponentField->setItem(y, x, item);
        this->setDisabled(false);
    }

    // Либо весь корабль красным
    else if (jsonObj.contains("ship_is_killed"))
    {
        QJsonArray deadParts = jsonObj["dead_parts"].toArray();
        QJsonArray missedCells = jsonObj["missed_cells"].toArray();

        /* Сначала закрашиваем серым цветом, потому
         * что они почему-то перекрывают красные клетки */
        for (int i = 0; i < missedCells.size(); ++i)
        {
            QStringList currentCell = missedCells[i].toString().split(" ");

            int x = currentCell[0].toInt();
            int y = currentCell[1].toInt();

            QTableWidgetItem* item = new QTableWidgetItem("");
            item->setBackground(GRAY);
            ui->opponentField->setItem(y, x, item);
        }

        /* Затем закрашиваем мёртвые части корабля */
        for (int i = 0; i < deadParts.size(); ++i)
        {
            QStringList currentPart = deadParts[i].toString().split(" ");

            int x = currentPart[0].toInt();
            int y = currentPart[1].toInt();

            QTableWidgetItem* item = new QTableWidgetItem("");
            item->setBackground(RED);
            ui->opponentField->setItem(y, x, item);
        }

        this->setDisabled(false);

        // Отписываемся от события, т.к. иначе в checkForWinner будет передаваться информация о кораблях
        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getFireStatus(QNetworkReply* )));
        // Проверяем является ли этот корабль последним, т.е. закончена ли игра
        this->checkForWinner();
    }
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
    qDebug() << "shoot";
    if (m_firePosition.size() == 0)
        return showMessage("Клетки не были выбраны", QMessageBox::Icon::Critical);

    return showMessage(QString::number(m_firePosition["x"]) + QString::number(m_firePosition["y"]), QMessageBox::Icon::Information);
    this->setDisabled(true);

    QUrl url("http://127.0.0.1:8000/games/fire/");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery jsonData;
    jsonData.addQueryItem("user_id", QString::number(this->m_userId));
    jsonData.addQueryItem("game_id", this->m_gameId);

    jsonData.addQueryItem("x", QString::number(m_firePosition["x"]));
    jsonData.addQueryItem("y", QString::number(m_firePosition["y"]));

    QUrl data;
    data.setQuery(jsonData);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getFireStatus(QNetworkReply* )));
    m_manager->post(request, data.toEncoded().remove(0, 1));
}
