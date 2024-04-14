#include "mainmenu.h"
#include "ui_mainmenu.h"

#include "mainwindow.h"


/*! @brief Конструктор класса
 *
 *  @details Инициализирует свойство класса m_manager
 *
 *  @param t_userId Идентификатор пользователя, который прошёл авторизацию
 *  @param *parent Указатель на родительский виджет
 *
 *  @return MainMenu
*/
MainMenu::MainMenu(int t_userId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainMenu),
    m_userId(t_userId)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);

    ui->userID->setText("Ваш ID: " + QString::number(t_userId));

    ui->tabWidget->setStyleSheet("QTabWidget::pane { border: 0; }");

    connect(ui->addFriendButton, &QPushButton::clicked, this, &MainMenu::openFriendAdder);
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateFriendsTab(int)));
    connect(ui->exitButton, &QPushButton::clicked, this, &MainMenu::close);
    connect(ui->friendRequestsListWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showFriendRequestsContextMenu(QPoint)));
    connect(ui->friendsListWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showFriendsContextMenu(QPoint)));

    ui->friendRequestsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->friendsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    menu = new QMenu("Ответ на запрос в друзья", this);
    m_manager = new QNetworkAccessManager(this);

    initSockets();
    setActionsLists();

    this->updateFriendsTab(ui->tabWidget->currentIndex());
}

//! @brief Деструктор класса
MainMenu::~MainMenu()
{
    delete ui;
}

/*! @brief Метод для обработки нажатия на кнопку "Подключиться к игре"
 *
 *  @details Делает запрос к адресу по подключению к игре.
 *  В параметры передаёт идентификатор пользователя
 *  после ответа от сервера передаёт управление методу connectToCreatedGame
 *
 *  @return void
*/
void MainMenu::on_createNewGameButton_clicked()
{
    QMap<QString, QString> queryItems;
    queryItems["user_id"] = QString::number(m_userId);

    QObject::connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(connectToCreatedGame(QNetworkReply*)));
    sendServerRequest("http://127.0.0.1:8000/games/create_game/", queryItems, m_manager);
}

/*! @brief Подключение к указанной игре
 *
 *  @details Получает идентификатор игры из ответа сервера, выдаёт ошибку
 *  или открывает окно с игрой, передавая айди игры
 *
 *  @param *t_reply Указатель  на ответ от сервера
 *
 *  @return void
*/
void MainMenu::connectToCreatedGame(QNetworkReply *t_reply)
{
    // Конвертируем строку в json object
    QJsonObject jsonResponse = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();

    // Если ответ содержит id игры, значит она создана и ошибок нет
    if (jsonResponse.contains("game_id")) {
        QString gameId = jsonResponse["game_id"].toString();
        QMessageBox msgBox;
        msgBox.setText("ID вашей игры:\n" + gameId);
        msgBox.exec();
        this->openMainWindow(gameId);
    } else if (jsonResponse.contains("Error")) {
        QString error = jsonResponse["Error"].toString();
        QMessageBox msgBox;
        msgBox.setText("Возникли ошибки:\n" + error);
        msgBox.exec();
    }
}

/*! @brief Вывод контекстного меню на виджет
 *
 *  @details Заполняет контекстное меню действиями, которые можно совершить с заявками в друзья
 *
 *  @param &t_point Ссылка на точку, где была нажата кнопка ЛКМ
 *
 *  @return void
*/
void MainMenu::showFriendRequestsContextMenu(const QPoint &t_point)
{
    QListWidgetItem *item = ui->friendRequestsListWidget->itemAt(t_point);
    if (item == NULL) return;

    assert(friendRequestsActionsText.size() == friendRequestsActionsData.size());

    QList<QAction*> actions;
    for (int i = 0; i < friendRequestsActionsText.size(); ++i){
        actions.append(new QAction(friendRequestsActionsText[i], NULL));
        actions[actions.size() - 1]->setData(friendRequestsActionsData[i]);
        connect(actions[actions.size() - 1], &QAction::triggered, this, [=] {
            MainMenu::processFriendRequestAction(
                item->text(),
                actions[actions.size() - 1]->data().toInt());
            });
    }

    menu->exec(actions, ui->friendRequestsListWidget->mapToGlobal(t_point));
}

/*! @brief Вывод контекстного меню на виджет
 *
 *  @details Заполняет контекстное меню действиями, которые можно совершить с имеющимися друзьями
 *
 *  @param &t_point Ссылка на точку, где была нажата кнопка ЛКМ
 *
 *  @return void
*/
void MainMenu::showFriendsContextMenu(const QPoint &t_point)
{
    QListWidgetItem *item = ui->friendsListWidget->itemAt(t_point);
    if (item == NULL) return;

    assert(friendsActionsText.size() == friendsActionsData.size());

    QList<QAction*> actions;
    for (int i = 0; i < friendsActionsText.size(); ++i){
        actions.append(new QAction(friendsActionsText[i], NULL));
        actions[actions.size() - 1]->setData(friendsActionsData[i]);
        connect(actions[actions.size() - 1], &QAction::triggered, this, [=] {
            MainMenu::interactWithFriend(
                item->text(),
                actions[actions.size() - 1]->data().toInt());
        });
    }

    menu->exec(actions, ui->friendsListWidget->mapToGlobal(t_point));
}

/*! @brief Обработка действия вызванного в контекстном меню
 *
 *  @details Обрабатывает действие, которое пользователь хочет применить к имеющемуся другу
 *
 *  @param t_friendUserName Никнейм друга
 *  @param t_action Код действия над другом
 *
 *  @return void
*/
void MainMenu::interactWithFriend(QString t_friendUserName, int t_action)
{
    if (t_action == FriendAction::DELETE_FRIEND_ACTION) {
        deleteFriend(m_userId, t_friendUserName);
    } else if (t_action == FriendAction::SEND_FRIENDLY_DUEL_REQUEST_ACTION) {
        sendFriendlyDuelRequest(t_friendUserName);
    }
}

/*! @brief Обработка действия вызванного в контекстном меню
 *
 *  @details Обрабатывает действие, которое пользователь хочет применить к заявке в друзья
 *
 *  @param t_friendUserName Никнейм друга
 *  @param t_action Код действия над другом
 *
 *  @return void
*/
void MainMenu::processFriendRequestAction(QString t_friendUserName, int t_action)
{
    QJsonObject queryParams;
    queryParams["action_type"] = "process_friend_request";
    queryParams["friend_username"] = t_friendUserName;
    queryParams["user_id"] = QString::number(m_userId);
    queryParams["process_status"] = QString::number(t_action == FriendRequestAction::ACCEPT_REQUEST_ACTION);

    m_friendsUpdateSocket->sendTextMessage(jsonObjectToQString(queryParams));
}

/*! @brief Обработчик нажатия кнопки "Подключиться"
 *
 *  @details Открывает окно для ввода идентификатора игры
 *
 *  @return void
*/
void MainMenu::on_connectToExistingGame_clicked()
{
    m_inputGameIdWindow = new InputGameID(this->m_userId);
    m_inputGameIdWindow->show();

    QObject::connect(m_inputGameIdWindow, &InputGameID::connectionAccepted, this, &MainMenu::openMainWindow);
}

/*! @brief Открытие окна с игрой
 *
 *  @param gameId Идентификатор игры
 *
 *  @return void
*/
void MainMenu::openMainWindow(QString t_gameId)
{
    m_mainWindow = new MainWindow(t_gameId, m_userId);

    m_mainWindow->show();
    close();
}

/*! @brief Получение друзей пользователя
 *
 *  @return void
*/
void MainMenu::getFriends()
{
    QMap<QString, QString> queryItems;
    queryItems["user_id"] = QString::number(m_userId);

    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fillFriendsTab(QNetworkReply*)));
    sendServerRequest("http://127.0.0.1:8000/friends/get_friends/", queryItems, m_manager);
}

/*! @brief Получение запросов в друзья
 *
 *  @return void
*/
void MainMenu::getFriendsRequests()
{
    QMap<QString, QString> queryParams;
    queryParams["user_id"] = QString::number(m_userId);

    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fillFriendsRequestsTab(QNetworkReply*)));
    sendServerRequest("http://127.0.0.1:8000/friends/get_incoming_friend_requests/", queryParams, m_manager);
}

/*! @brief Добавление друга
 *
 *  @details На сервер передаётся запрос, в котором указывается идентификатор
 *  пользователя которому нужно отправить запрос в друзья
 *
 *  @param t_friendId Идентификатор друга, которому отправляется запрос
 *
 *  @return void
*/
void MainMenu::sendFriendRequest(int t_friendId)
{
    QJsonObject queryItems;
    queryItems["action_type"] = "send_friend_request";
    queryItems["sender_id"] = QString::number(m_userId);
    queryItems["receiver_id"] = QString::number(t_friendId);

    m_friendsUpdateSocket->sendTextMessage(jsonObjectToQString(queryItems));
}

/*!  @brief Отправка запроса на дуэль
 *
 *   @param t_friendUsername Имя пользователя, которому адресована дуэль
 *
 *   @return void
*/
void MainMenu::sendFriendlyDuelRequest(QString t_friendUsername)
{
    QJsonObject jsonObj;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::SEND_GAME_INVITE];
    jsonObj["from_user_id"] = QString::number(m_userId);
    jsonObj["to_user_name"] = t_friendUsername;

    m_friendlyDuelSocket->sendTextMessage(jsonObjectToQString(jsonObj));
}

/*! @brief Удаление друга
 *
 *  @param t_userId Идентифиатор пользователя, который хочет удалить друга
 *  @param t_friendUserName Имя друга, которого нужно удалить
 *
 *  @return void
*/
void MainMenu::deleteFriend(int t_userId, QString t_friendUserName)
{
    QJsonObject queryParams;
    queryParams["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::DELETE_FRIEND];
    queryParams["user_id"] = QString::number(t_userId);
    queryParams["friend_username"] = t_friendUserName;

    m_friendsUpdateSocket->sendTextMessage(jsonObjectToQString(queryParams));
}

/*! @brief Установка списка действий
 *
 *  @details Заполняет списков действий и их описаний
 *
 *  @return void
*/
void MainMenu::setActionsLists()
{
    friendRequestsActionsText = QStringList() << "Принять запрос" << "Отклонить запрос";
    friendRequestsActionsData = QList<FriendRequestAction>()
                                << FriendRequestAction::ACCEPT_REQUEST_ACTION
                                << FriendRequestAction::DECLINE_REQUEST_ACTION;

    friendsActionsText = QStringList() << "Вызвать на дуэль" << "Удалить друга";
    friendsActionsData = QList<FriendAction>()
                         << FriendAction::SEND_FRIENDLY_DUEL_REQUEST_ACTION
                         << FriendAction::DELETE_FRIEND_ACTION;
}

/*! @brief Открытие окна для ввода идентификатора друга
 *
 *  @return void
*/
void MainMenu::openFriendAdder()
{
    m_friendAdderWindow = new FriendAdder();
    m_friendAdderWindow->show();

    connect(m_friendAdderWindow, &FriendAdder::friendAdded, this, &MainMenu::sendFriendRequest);
}

/*! @brief Инициалиализация сокетов их адресов и сигналов
 *
 *  @return void
*/
void MainMenu::initSockets()
{
    initFriendsUpdateSocket();
    initFriendlyDuelSocket();
}

/*! @brief Создание сокета для обновления друзей и событий к нему
 *
 *  @return void
*/
void MainMenu::initFriendsUpdateSocket()
{
    m_friendsUpdateSocket = new QWebSocket();

    m_friendsUpdateUrl.setPort(8080);
    m_friendsUpdateUrl.setHost("127.0.0.1");
    m_friendsUpdateUrl.setPath("/friends_update/");
    m_friendsUpdateUrl.setScheme("ws");

    m_friendsUpdateSocket->open(m_friendsUpdateUrl);

    connect(m_friendsUpdateSocket, SIGNAL(connected()), this, SLOT(onFriendsUpdateSocketConnected()));
    connect(m_friendsUpdateSocket, SIGNAL(disconnected()), this, SLOT(onFriendsUpdateSocketDisconnected()));
    connect(m_friendsUpdateSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onFriendsUpdateSocketErrorOccurred(QAbstractSocket::SocketError)));
    connect(m_friendsUpdateSocket, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onFriendsUpdateSocketMessageReceived(QString)));
}

/*! @brief Создание сокета для дуэлей и событий к нему
 *
 *  @return void
*/
void MainMenu::initFriendlyDuelSocket()
{
    m_friendlyDuelSocket = new QWebSocket();

    m_friendlyDuelUrl.setPort(8080);
    m_friendlyDuelUrl.setHost("127.0.0.1");
    m_friendlyDuelUrl.setPath("/friendly_duel/");
    m_friendlyDuelUrl.setScheme("ws");

    m_friendlyDuelSocket->open(m_friendlyDuelUrl);

    connect(m_friendlyDuelSocket, SIGNAL(connected()), this, SLOT(onFriendlyDuelSocketConnected()));
    connect(m_friendlyDuelSocket, SIGNAL(disconnected()), this, SLOT(onFriendlyDuelSocketDisconnected()));
    connect(m_friendlyDuelSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onFriendlyDuelSocketErrorOccurred(QAbstractSocket::SocketError)));
    connect(m_friendlyDuelSocket, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onFriendlyDuelSocketMessageReceived(QString)));
}

/*! @brief Обрабатывание события при изменение активной вкладки
 *
 *  @param t_tabIndex Текущий идентификатор вкладки
 *
 *  @return void
*/
void MainMenu::updateFriendsTab(int t_tabIndex)
{
    if (t_tabIndex == 0) {
        getFriends();
    } else if (t_tabIndex == 1) {
        getFriendsRequests();
    }
}

/*! @brief Обработчик подключения сокета к серверу
 *
 *  @details Отправляет серверу идентификатор пользователя чтоб подписаться на события
 *
 *  @return void
*/
void MainMenu::onFriendsUpdateSocketConnected()
{
    QJsonObject jsonObj;
    jsonObj["user_id"] = m_userId;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::SUBSCRIBE];
    m_friendsUpdateSocket->sendTextMessage(jsonObjectToQString(jsonObj));
}

/*! @brief Обработчик отключения сокета от сервера
 *
 *  @return void
*/
void MainMenu::onFriendsUpdateSocketDisconnected()
{

}

/*! @brief Обработчик получения информации с сервера через сокет
 *
 *  @param t_textMessage Текст полученного сообщения
 *
 *  @return void
*/
void MainMenu::onFriendsUpdateSocketMessageReceived(QString t_textMessage)
{
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
    if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::SUBSCRIBED]) {
        return;
    } else if  (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::DELETED_BY_FRIEND]){
        getFriends();
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::FRIEND_DELETED]) {
        showMessage("Друг был успешно удалён", QMessageBox::Icon::Information);
        getFriends();
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::NEW_FRIEND_REQUEST]) {
        getFriendsRequests();
    } else if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::FRIEND_REQUEST_PROCESSED]) {
        getFriends();
        connect(this, &MainMenu::friendsPulled, this, &MainMenu::getFriendsRequests);
    } else {
        showMessage("Успешно", QMessageBox::Icon::Information);
    }
}

/*! @brief Обработчик ошибки, полученной во время отправки запроса на сервер через сокет
 *
 *  @param t_socketError Вид ошибки, полученной при передачи данных
 *
 *  @return void
*/
void MainMenu::onFriendsUpdateSocketErrorOccurred(QAbstractSocket::SocketError t_socketError)
{
    showMessage("Возникла ошибка при подключении к серверу", QMessageBox::Icon::Critical);
    close();
}

/*! @brief Обработчик подключения сокета к серверу
 *
 *  @details Отправляет серверу идентификатор пользователя чтоб подписаться на события
 *
 *  @return void
*/
void MainMenu::onFriendlyDuelSocketConnected()
{
    QJsonObject jsonObj;
    jsonObj["user_id"] = m_userId;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::SUBSCRIBE];
    m_friendlyDuelSocket->sendTextMessage(jsonObjectToQString(jsonObj));
}

void MainMenu::onFriendlyDuelSocketDisconnected()
{

}

void MainMenu::onFriendlyDuelSocketMessageReceived(QString t_textMessage)
{
    QJsonObject jsonResponse = QJsonDocument::fromJson(t_textMessage.toUtf8()).object();
    if (jsonResponse.contains("error")) {
        return showMessage(jsonResponse["error"].toString(), QMessageBox::Icon::Critical);
    }
    if (!jsonResponse.contains("action_type")) {
        return;
    }

    QString actionType = jsonResponse["action_type"].toString();
    if (actionType == "subscribed") {
        return;
    }

    if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::GAME_INVITE_SENT]){
        QString gameId = jsonResponse["game_id"].toString();
        QString gameInviteId = jsonResponse["game_invite_id"].toString();
        // m_mainWindow = new MainWindow()
    // Открываем окно с игрой и подключаемся к игре
    } else if (actionType == "incoming_game_invite") {
        QString gameInviteId = jsonResponse["game_invite_id"].toString();
        GameInviteNotifier widget(this);
        widget.show();
    }
}

void MainMenu::onFriendlyDuelSocketErrorOccurred(QAbstractSocket::SocketError t_socketError)
{

}

/*! @brief Заполнение списка пользователями
 *
 *  @param *t_reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainMenu::fillFriendsTab(QNetworkReply *t_reply)
{
    disconnect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fillFriendsTab(QNetworkReply*)));
    QJsonObject jsonResponse = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();

    qDebug() << "fillFriendsTab";
    qDebug() << jsonResponse;
    if (jsonResponse.isEmpty()) {
        showMessage("Сервер недоступен", QMessageBox::Icon::Critical);
        close();
        return;
    }

    if (jsonResponse.contains("error")) {
        QString error_message = jsonResponse["error"].toString();
        return showMessage(error_message, QMessageBox::Icon::Critical);
    }

    QJsonArray friends = jsonResponse["friends"].toArray();
    ui->friendsListWidget->clear();
    for (int i = 0; i < friends.size(); ++i) {
        QListWidgetItem *friend_name = new QListWidgetItem(friends[i].toString());
        ui->friendsListWidget->addItem(friend_name);
    }

    // Используется чтобы после запроса на получения друзей
    // запустить запрос на получение заявок в друзья
    emit friendsPulled();
}

/*! @brief Заполнение списка заявок в друзья
 *
 *  @param *t_reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainMenu::fillFriendsRequestsTab(QNetworkReply *t_reply)
{
    disconnect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fillFriendsRequestsTab(QNetworkReply*)));
    QJsonObject jsonResponse = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();

    if (jsonResponse.isEmpty()) {
        showMessage("Сервер недоступен", QMessageBox::Icon::Critical);
        close();
        return;
    }

    if (jsonResponse.contains("error")) {
        QString error_message = jsonResponse["error"].toString();
        return showMessage(error_message, QMessageBox::Icon::Critical);
    }

    QJsonArray jsonArray = jsonResponse["friend_requests"].toArray();
    ui->friendRequestsListWidget->clear();
    for (int i = 0; i < jsonArray.size(); ++i) {
        ui->friendRequestsListWidget->addItem(jsonArray[i].toString());
    }
}
