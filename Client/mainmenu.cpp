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

/*!???*/
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

/*!???*/
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

/*!???*/
void MainMenu::interactWithFriend(QString t_friendUserName, int t_action)
{
    QMap<QString, QString> queryParams;
    queryParams["user_id"] = QString::number(m_userId);
    queryParams["friend_username"] = t_friendUserName;
    if (t_action == FriendAction::DELETE_FRIEND) {

        connect(m_manager, &QNetworkAccessManager::finished,
                this, &MainMenu::getDeleteFriendRequestStatus);
        sendServerRequest("http://127.0.0.1:8000/friends/delete_friend/", queryParams, m_manager);
    }
}

/*!???*/
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

/*!???*/
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
    QString uID = QString::number(m_userId);
    queryItems["user_id"] = QString::number(m_userId);

    connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), SLOT(fillFriendsTab(QNetworkReply* )));
    sendServerRequest("http://127.0.0.1:8000/friends/get_friends/", queryItems, m_manager);
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
    QMap<QString, QString> queryItems;
    queryItems["sender_id"] = QString::number(m_userId);
    queryItems["receiver_id"] = QString::number(t_friendId);

    connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(getFriendRequestStatus(QNetworkReply*)));
    sendServerRequest("http://127.0.0.1:8000/friends/send_friend_request/", queryItems, m_manager);
}

/*!???*/
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

/*! @brief Обрабатывание события при изменение активной вкладки
 *
 *  @param t_tabIndex Текущий идентификатор вкладки
 *
 *  @return void
*/
void MainMenu::updateFriendsTab(int t_tabIndex)
{
    QMap<QString, QString> queryParams;
    queryParams["user_id"] = QString::number(m_userId);
    if (t_tabIndex == 0) {
        connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fillFriendsTab(QNetworkReply*)));
        sendServerRequest("http://127.0.0.1:8000/friends/get_friends/", queryParams, m_manager);
    } else if (t_tabIndex == 1) {
        connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fillFriendsRequestsTab(QNetworkReply*)));
        sendServerRequest("http://127.0.0.1:8000/friends/get_incoming_friend_requests/", queryParams, m_manager);
    }
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
