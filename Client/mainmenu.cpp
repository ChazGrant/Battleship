#include "mainmenu.h"
#include "ui_mainmenu.h"

#include "mainwindow.h"


/*! @brief Конструктор класса
 *
 *  @details Инициализирует свойство класса m_manager
 *
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

    m_manager = new QNetworkAccessManager(this);
    ui->userID->setText("Ваш ID: " + QString::number(t_userId));

    ui->tabWidget->setStyleSheet("QTabWidget::pane { border: 0; }");

    connect(ui->addFriendButton, &QPushButton::clicked, this, &MainMenu::openFriendAdder);
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateFriendsTab(int)));
    connect(ui->exitButton, &QPushButton::clicked, this, &MainMenu::close);

    ui->friendRequestsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->friendsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->friendRequestsListWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showFriendRequestsContextMenu(const QPoint&)));
    connect(ui->friendsListWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showFriendsContextMenu(const QPoint&)));

    menu = new QMenu("Ответ на запрос в друзья", this);

    initSocket();
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

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), SLOT(connectToCreatedGame(QNetworkReply* )));
    sendServerRequest("http://127.0.0.1:8000/games/create_game/", queryItems, m_manager);
}

/*! @brief Подключение к указанной игре
 *
 *  @details Получает идентификатор игры из ответа сервера, выдаёт ошибку
 *  или открывает окно с игрой, передавая айди игры
 *
 *  @param *reply Указатель  на ответ от сервера
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
 *  @param &point Ссылка на точку, где была нажата кнопка ЛКМ
 *
 *  @return void
*/
void MainMenu::showFriendRequestsContextMenu(const QPoint &point)
{
    QListWidgetItem *item = ui->friendRequestsListWidget->itemAt(point);
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

    menu->exec(actions, ui->friendRequestsListWidget->mapToGlobal(point));
}

/*! @brief Вывод контекстного меню на виджет
 *
 *  @details Заполняет контекстное меню действиями, которые можно совершить с имеющимися друзьями
 *
 *  @param &point Ссылка на точку, где была нажата кнопка ЛКМ
 *
 *  @return void
*/
void MainMenu::showFriendsContextMenu(const QPoint &point)
{
    QListWidgetItem *item = ui->friendsListWidget->itemAt(point);
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

    menu->exec(actions, ui->friendsListWidget->mapToGlobal(point));
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
    if (t_action == FriendAction::DELETE_FRIEND) {
        deleteFriend(m_userId, t_friendUserName);
    } else if (t_action == FriendAction::SEND_FRIENDLY_DUEL_REQUEST) {
        return;
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
    QMap<QString, QString> queryParams;
    queryParams["user_id"] = QString::number(m_userId);
    queryParams["friend_username"] = t_friendUserName;
    queryParams["process_status"] = QString::number(t_action == FriendRequestAction::ACCEPT_REQUEST);

    connect(m_manager, &QNetworkAccessManager::finished,
            this, &MainMenu::getIncomingFriendRequestProcessStatus);
    sendServerRequest("http://127.0.0.1:8000/friends/process_friend_request/", queryParams, m_manager);
}

/*! @brief Получение статуса ответа на заявку в друзья
 *
 *  @param *t_reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainMenu::getIncomingFriendRequestProcessStatus(QNetworkReply *t_reply)
{
    disconnect(m_manager, &QNetworkAccessManager::finished,
               this, &MainMenu::getIncomingFriendRequestProcessStatus);

    QJsonObject jsonResponse = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();
    if (jsonResponse.contains("error")) {
        QString error = jsonResponse["error"].toString();
        return showMessage(error, QMessageBox::Icon::Critical);
    }

    showMessage("Готово", QMessageBox::Icon::Information);
    updateFriendsTab(ui->tabWidget->currentIndex());
}

/*! @brief Получение статуса удаления друга
 *
 *  @param *t_reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainMenu::getDeleteFriendRequestStatus(QNetworkReply *t_reply)
{
    QJsonObject jsonResponse = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();

    if (jsonResponse.contains("error")) {
        QString error_message = jsonResponse["error"].toString();
        return showMessage(error_message, QMessageBox::Icon::Critical);
    }

    showMessage("Друг был успешно удалён", QMessageBox::Icon::Information);
    disconnect(m_manager, &QNetworkAccessManager::finished,
            this, &MainMenu::getDeleteFriendRequestStatus);
    updateFriendsTab(ui->tabWidget->currentIndex());
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
    m_mainWindow = new MainWindow (nullptr, t_gameId, m_userId);

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

    connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), SLOT(fillFriendsTab(QNetworkReply* )));
    sendServerRequest("http://127.0.0.1:8000/friends/get_friends/", queryItems, m_manager);
}

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

    m_friendsUpdateSocket->sendTextMessage(jsonObjectToQstring(queryItems));
}

void MainMenu::deleteFriend(int t_userId, QString t_friendUserName)
{
    QJsonObject queryParams;
    queryParams["action_type"] = "delete_friend";
    queryParams["user_id"] = QString::number(t_userId);
    queryParams["friend_username"] = t_friendUserName;

    m_friendsUpdateSocket->sendTextMessage(jsonObjectToQstring(queryParams));
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
                                << FriendRequestAction::ACCEPT_REQUEST
                                << FriendRequestAction::DECLINE_REQUEST;

    friendsActionsText = QStringList() << "Вызвать на дуэль" << "Удалить друга";
    friendsActionsData = QList<FriendAction>()
                         << FriendAction::SEND_FRIENDLY_DUEL_REQUEST
                         << FriendAction::DELETE_FRIEND;
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
void MainMenu::initSocket()
{
    m_friendsUpdateSocket = new QWebSocket();

    m_friendsUpdateUrl.setPort(8080);
    m_friendsUpdateUrl.setHost("127.0.0.1");
    m_friendsUpdateUrl.setPath("/friends_update/");
    m_friendsUpdateUrl.setScheme("ws");

    m_friendsUpdateSocket->open(m_friendsUpdateUrl);

    connect(m_friendsUpdateSocket, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    connect(m_friendsUpdateSocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    connect(m_friendsUpdateSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onSocketErrorOccurred(QAbstractSocket::SocketError)));
    connect(m_friendsUpdateSocket, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onSocketMessageReceived(QString)));
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
 *  @return void
*/
void MainMenu::onSocketConnected()
{
    QJsonObject jsonObj;
    jsonObj["user_id"] = m_userId;
    jsonObj["action_type"] = "subscribe";
    m_friendsUpdateSocket->sendTextMessage(jsonObjectToQstring(jsonObj));
}

/*! @brief Обработчик отключения сокета от сервера
 *
 *  @return void
*/
void MainMenu::onSocketDisconnected()
{

}

/*! @brief Обработчик получения информации с сервера через сокет
 *
 *  @param t_textMessage Текст полученного сообщения
 *
 *  @return void
*/
void MainMenu::onSocketMessageReceived(QString t_textMessage)
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

    } else if  (actionType == "deleted_by_friend"){
        getFriends();
    } else if (actionType == "friend_deleted") {
        showMessage("Друг был успешно удалён", QMessageBox::Icon::Information);
        getFriends();
    } else if (actionType == "new_friend_request") {
        getFriendsRequests();
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
void MainMenu::onSocketErrorOccurred(QAbstractSocket::SocketError t_socketError)
{
    showMessage("Возникла ошибка при подключении к серверу", QMessageBox::Icon::Critical);
    close();
}

/*! @brief Заполнение списка пользователями
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainMenu::fillFriendsTab(QNetworkReply *t_reply)
{
    disconnect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fillFriendsTab(QNetworkReply*)));
    QJsonObject jsonResponse = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();

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
}

/*! @brief Заполнение списка заявок в друзья
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainMenu::fillFriendsRequestsTab(QNetworkReply *t_reply)
{
    disconnect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fillFriendsRequestsTab(QNetworkReply*)));
    QJsonObject jsonResponse = QJsonDocument::fromJson(QString(t_reply->readAll()).toUtf8()).object();

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

/*! @brief Получение результата отправки запроса в друзья
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainMenu::getFriendRequestStatus(QNetworkReply *t_reply)
{
    disconnect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(getFriendRequestStatus(QNetworkReply*)));
    QString strReply = t_reply->readAll();
    QJsonObject jsonResponse = QJsonDocument::fromJson(strReply.toUtf8()).object();

    if (jsonResponse.contains("error")) {
        QString errorMessage = jsonResponse["error"].toString();
        return showMessage(errorMessage, QMessageBox::Icon::Critical);
    }

    showMessage("Заявка в друзья была успешно отправлена", QMessageBox::Icon::Information);
}
