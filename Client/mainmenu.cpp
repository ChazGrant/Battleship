
#include "mainmenu.h"
#include "ui_mainmenu.h"

#include "mainwindow.h"


/*! @brief Конструктор класса
 *
 *  @details Инициализирует свойство класса m_manager
 *
 *  @param t_userId Идентификатор пользователя
 *  @param t_userName Имя пользователя
 *  @param *parent Указатель на родительский виджет
 *
 *  @return MainMenu
*/
MainMenu::MainMenu(int t_userId, QString t_userName, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainMenu),
    m_userId(t_userId),
    m_userName(t_userName)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);

    ui->userDataLabel->setText(QString("Ваш ID:%1 \nВаше имя пользователя: %2").
                               arg(QString::number(m_userId), m_userName));
    ui->tabWidget->setStyleSheet("QTabWidget::pane { border: 0; }");

    connect(ui->createNewGameButton, &QPushButton::clicked, this, &MainMenu::createGame);
    connect(ui->startGameWithAIButton, &QPushButton::clicked, this, [this]() { createGame(true); });
    connect(ui->findGameButton, &QPushButton::clicked, this, &MainMenu::connectToRandomGame);

    connect(ui->openShopButton, &QPushButton::clicked, this, &MainMenu::openShopWidget);

    connect(ui->addFriendButton, &QPushButton::clicked, this, &MainMenu::openFriendAdder);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainMenu::updateFriendsTab);
    connect(ui->friendRequestsListWidget, &QListWidget::customContextMenuRequested, this, &MainMenu::showFriendRequestsContextMenu);
    connect(ui->friendsListWidget, &QListWidget::customContextMenuRequested, this, &MainMenu::showFriendsContextMenu);
    connect(ui->openTopPlayersWidget, &QPushButton::clicked, this, &MainMenu::getTopPlayers);

    connect(ui->showClansButton, &QPushButton::clicked, this, &MainMenu::showNotImplementedFeature);
    connect(ui->createClanButton, &QPushButton::clicked, this, &MainMenu::showNotImplementedFeature);
    connect(ui->createTournamentButton, &QPushButton::clicked, this, &MainMenu::showNotImplementedFeature);

    connect(ui->hideButton, &QPushButton::clicked, this, &MainMenu::showMinimized);
    connect(ui->exitButton, &QPushButton::clicked, this, &MainMenu::close);

    ui->friendRequestsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->friendsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    menu = new QMenu("Ответ на запрос в друзья", this);
    m_manager = new QNetworkAccessManager(this);

    initSockets();
    setActionsLists();

    updateFriendsTab(ui->tabWidget->currentIndex());
}

//! @brief Деструктор класса
MainMenu::~MainMenu()
{
    delete ui;
}

/*! @brief Обработка перемещения мыши
 *
 *  @details Меняет переменную delta и перемещает окно к её координатам
 *
 *  @param *event Указатель на событие поведения мыши
 *
 *  @return void
*/
void MainMenu::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF delta = event->globalPos() - m_mouse_point;
    move(delta.toPoint());

    event->accept();
}

/*! @brief Обработка нажатия кнопки мыши
 *
 *  @details При нажатии на левую, правую или среднюю кнопку мыши
 *  меняет приватную переменную m_mouse_point
 *
 *  @param *event Указатель на событие поведения мыши
 *
 *  @return void
*/
void MainMenu::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton ||
        event->button() == Qt::RightButton ||
        event->button() == Qt::MiddleButton) {
        m_mouse_point = event->pos();
        event->accept();
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
    queryParams["action_type"] =
        OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::PROCESS_FRIEND_REQUEST];
    queryParams["friend_username"] = t_friendUserName;
    queryParams["user_id"] = QString::number(m_userId);
    queryParams["process_status"] =
        QString::number(t_action == FriendRequestAction::ACCEPT_REQUEST_ACTION);

    m_friendsUpdateSocket->sendTextMessage(convertJsonObjectToString(queryParams));
}

/*! @brief Открытие окна с игрой
 *
 *  @param gameId Идентификатор игры
 *
 *  @return void
*/
void MainMenu::openMainWindow(QString t_gameId, QString t_gameInviteId)
{
    m_mainWindow = new MainWindow(t_gameId, m_userId, t_gameInviteId);
    m_mainWindow->show();
    hide();

    disconnect(m_mainWindow, &MainWindow::widgetClosed, this, &MainMenu::show);
    connect(m_mainWindow, &MainWindow::widgetClosed, this, &MainMenu::show);
}

/*! @brief Открытие виджета магазина
 *
 *  @return void
*/
void MainMenu::openShopWidget()
{
    m_shopWindow = new Shop(m_userId);
    m_shopWindow->show();
    hide();

    disconnect(m_shopWindow, &Shop::widgetClosed, this, &MainMenu::show);
    connect(m_shopWindow, &Shop::widgetClosed, this, &MainMenu::show);
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
    queryItems["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::SEND_FRIEND_REQUEST];
    queryItems["sender_id"] = QString::number(m_userId);
    queryItems["receiver_id"] = QString::number(t_friendId);

    m_friendsUpdateSocket->sendTextMessage(convertJsonObjectToString(queryItems));
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
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::CREATE_GAME];
    jsonObj["user_id"] = QString::number(m_userId);
    jsonObj["to_user_name"] = t_friendUsername;

    m_gameCreatorSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
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

    m_friendsUpdateSocket->sendTextMessage(convertJsonObjectToString(queryParams));
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
    setDisabled(true);
    m_friendAdderWindow = new FriendAdder();
    m_friendAdderWindow->show();

    connect(m_friendAdderWindow, &FriendAdder::friendAdded, this, &MainMenu::sendFriendRequest);
    connect(m_friendAdderWindow, &FriendAdder::destroyed, this, [=] {
        setDisabled(false);
        disconnect(m_friendAdderWindow, &FriendAdder::friendAdded, this, &MainMenu::sendFriendRequest);
    });
}

/*! @brief Получение топа игроков
 *
 *  @return void
*/
void MainMenu::getTopPlayers()
{
    QMap<QString, QString> queryParams;

    sendServerRequest("http://127.0.0.1:8000/leagues/get_top_players/", queryParams, m_manager);
    connect(m_manager, &QNetworkAccessManager::finished, this, &MainMenu::openTopPlayersWidget);
}

/*! @brief Создание игры
 *
 *  @return void
*/
void MainMenu::createGame(bool t_opponentIsAI)
{
    QJsonObject jsonObj;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::CREATE_GAME];
    jsonObj["user_id"] = QString::number(m_userId);
    jsonObj["opponent_is_ai"] = QString::number(t_opponentIsAI);
    m_gameCreatorSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
}

/*! @brief Подключение к случайной игре
 *
 *  @return void
*/
void MainMenu::connectToRandomGame()
{
    QJsonObject jsonObj;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::FIND_GAME];
    m_gameCreatorSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
}

/*! @brief Вывод сообщения о том, что функция будет в будущих обновлениях
 *
 *  @return void
*/
void MainMenu::showNotImplementedFeature()
{
    return showMessage("В будущих обновлениях...", QMessageBox::Icon::Information);
}

void MainMenu::closeEvent(QCloseEvent *event)
{
    m_gameCreatorSocket->disconnect();
    emit widgetClosed();
    event->accept();
}

/*! @brief Инициалиализация сокетов
 *
 *  @return void
*/
void MainMenu::initSockets()
{
    initFriendsUpdateSocket();
    initGameCreatorSocket();
}

/*! @brief Создание сокета для обновления друзей и событий к нему
 *
 *  @return void
*/
void MainMenu::initFriendsUpdateSocket()
{
    m_friendsUpdateSocket = new QWebSocket();

    m_friendsUpdateSocketUrl.setPort(8080);
    m_friendsUpdateSocketUrl.setHost("127.0.0.1");
    m_friendsUpdateSocketUrl.setPath("/friends_update/");
    m_friendsUpdateSocketUrl.setScheme("ws");

    m_friendsUpdateSocket->open(m_friendsUpdateSocketUrl);

    connect(m_friendsUpdateSocket, SIGNAL(connected()), this, SLOT(onFriendsUpdateSocketConnected()));
    connect(m_friendsUpdateSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onFriendsUpdateSocketErrorOccurred(QAbstractSocket::SocketError)));
    connect(m_friendsUpdateSocket, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onFriendsUpdateSocketMessageReceived(QString)));
}

/*! @brief Создание сокета для дуэлей и событий к нему
 *
 *  @return void
*/
void MainMenu::initGameCreatorSocket()
{
    m_gameCreatorSocket = new QWebSocket();

    m_gameCreatorSocketUrl.setPort(8080);
    m_gameCreatorSocketUrl.setHost("127.0.0.1");
    m_gameCreatorSocketUrl.setPath("/game_creator/");
    m_gameCreatorSocketUrl.setScheme("ws");

    m_gameCreatorSocket->open(m_gameCreatorSocketUrl);

    connect(m_gameCreatorSocket, SIGNAL(connected()), this, SLOT(onGameCreatorSocketConnected()));
    connect(m_gameCreatorSocket, SIGNAL(disconnected()), this, SLOT(onGameCreatorSocketDisconnected()));
    connect(m_gameCreatorSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onGameCreatorSocketErrorOccurred(QAbstractSocket::SocketError)));
    connect(m_gameCreatorSocket, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onGameCreatorSocketMessageReceived(QString)));
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
    m_friendsUpdateSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
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
void MainMenu::onGameCreatorSocketConnected()
{
    QJsonObject jsonObj;
    jsonObj["user_id"] = m_userId;
    jsonObj["action_type"] = OUTGOING_ACTIONS[OUTGOING_ACTIONS_NAMES::SUBSCRIBE];
    m_gameCreatorSocket->sendTextMessage(convertJsonObjectToString(jsonObj));
}

/*! @brief Обработчик отключения сокета от сервера
 *
 *  @return void
*/
void MainMenu::onGameCreatorSocketDisconnected()
{
    showMessage("Вы были отключены от сервера", QMessageBox::Icon::Critical);
    close();
}

/*! @brief Обработчик получения сообщения для сокета GameCreator
 *
 *  @param t_textMessage Полученное сообщение
 *
 *  @return void
*/
void MainMenu::onGameCreatorSocketMessageReceived(QString t_textMessage)
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

    if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::GAME_CREATED] ||
        actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::INCOMIG_GAME_INVITE] ||
        actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::GAME_FOUND]){
        QString gameId = jsonResponse["game_id"].toString();
        QString gameInviteId = jsonResponse["game_invite_id"].toString();
        if (actionType == INCOMING_ACTIONS[INCOMING_ACTIONS_NAMES::INCOMIG_GAME_INVITE]) {
            m_gameInviteNotifier = new GameInviteNotifier();
            m_gameInviteNotifier->show();
            connect(m_gameInviteNotifier, &GameInviteNotifier::gameInviteAccepted, this, [=]() {
                openMainWindow(gameId, gameInviteId);
            });
            return;
        }
        openMainWindow(gameId, gameInviteId);
    }
}

/*! @brief Обработчик ошибок, полученных при передаче данных на сервер
 *
 *  @param t_socketError Тип ошибки
 *
 *  @return void
*/
void MainMenu::onGameCreatorSocketErrorOccurred(QAbstractSocket::SocketError t_socketError)
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

/*! @brief Открытие виджета топа игроков
 *
 *  @param *t_reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainMenu::openTopPlayersWidget(QNetworkReply *t_reply)
{
    disconnect(m_manager, &QNetworkAccessManager::finished, this, &MainMenu::openTopPlayersWidget);

    QJsonObject jsonResponse = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();
    if (jsonResponse.contains("error")) {
        QString error_message = jsonResponse["error"].toString();
        return showMessage(error_message, QMessageBox::Icon::Critical);
    }

    QJsonArray leagues = jsonResponse["leagues"].toArray();
    QJsonObject playersByCups = jsonResponse["players_by_cups"].toObject();
    QJsonObject playersBySilverCoins = jsonResponse["players_by_silver_coins"].toObject();
    QJsonObject playersByWinstreak = jsonResponse["player_by_winstreak"].toObject();
    m_topPlayersWidget = new TopPlayers(leagues, playersByCups, playersBySilverCoins, playersByWinstreak);
    hide();

    disconnect(m_topPlayersWidget, &TopPlayers::widgetClosed, this, &MainMenu::show);
    connect(m_topPlayersWidget, &TopPlayers::widgetClosed, this, &MainMenu::show);

    m_topPlayersWidget->show();
}
