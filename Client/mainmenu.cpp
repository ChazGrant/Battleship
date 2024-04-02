#include "mainmenu.h"
#include "ui_mainmenu.h"

#include "inputgameid.h"
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

    connect(ui->addFriendButton, &QPushButton::clicked, this, &MainMenu::openFriendAdder);
    connect(ui->exitButton, &QPushButton::clicked, this, &MainMenu::close);

    this->getFriends();
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
    QUrl url("http://127.0.0.1:8000/games/create_game/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;
    query.addQueryItem("user_id", QString::number(m_userId));

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), SLOT(connectToCreatedGame(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
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
void MainMenu::connectToCreatedGame(QNetworkReply *reply)
{
    // Читаем ответ от сервера в строку
    QString strReply = reply->readAll();
    // Конвертируем строку в json object
    QJsonObject jsonResponse = QJsonDocument::fromJson(strReply.toUtf8()).object();


    // Если ответ содержит id игры, значит она создана и ошибок нет
    if (jsonResponse.contains("game_id"))
    {
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

/*! @brief Обработчик нажатия кнопки "Подключиться"
 *
 *  @details Открывает окно для ввода идентификатора игры
 *
 *  @return void
*/
void MainMenu::on_connectToExistingGame_clicked()
{
    InputGameID *widget = new InputGameID(this->m_userId);
    widget->show();

    QObject::connect(widget, &InputGameID::acceptConnection, this, &MainMenu::openMainWindow);
}

/*! @brief Открытие окна с игрой
 *
 *  @param gameId Идентификатор игры
 *
 *  @return void
*/
void MainMenu::openMainWindow(QString t_gameId)
{
    this->m_mainWindow = new MainWindow (nullptr, t_gameId, m_userId);

    m_mainWindow->show();
    this->close();
}

/*! @brief Получение друзей пользователя
 *
 *  @return void
*/
void MainMenu::getFriends()
{
    QUrl url("http://127.0.0.1:8000/friends/get_friends/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;
    QString userId = ui->userID->text().split(": ")[1];

    query.addQueryItem("user_id", userId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), SLOT(fillFriendsTab(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
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

    connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), SLOT(getFriendRequestStatus(QNetworkReply* )));
    sendServerRequest("http://127.0.0.1:8000/friends/send_friend_request/", queryItems);
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

/*! @brief Заполнение списка пользователями
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainMenu::fillFriendsTab(QNetworkReply *reply)
{
    QString strReply = reply->readAll();
    QJsonObject jsonResponse = QJsonDocument::fromJson(strReply.toUtf8()).object();

    if (jsonResponse.contains("error")) {
        QString error_message = jsonResponse["error"].toString();
        return showMessage(error_message, QMessageBox::Icon::Critical);
    }
    QJsonArray friends = jsonResponse["friends"].toArray();
    for (int i = 0; i < friends.size(); ++i) {
        QListWidgetItem *friend_name = new QListWidgetItem(friends[i].toString());
        ui->friendsListWidget->addItem(friend_name);
    }
}

/*! @brief Получение результата отправки запроса в друзья
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void MainMenu::getFriendRequestStatus(QNetworkReply *reply)
{

}

/*! @brief Отправка POST запроса на указанный адрес сервера
 *
 *  @param t_requestUrl Адрес на который нужно отправить запрос
 *  @param t_queryItems Хэш-таблица, содержащая имя параметра и значение
 *
 *  @return void
*/
void MainMenu::sendServerRequest(QString t_requestUrl, QMap<QString, QString> t_queryItems)
{
    qDebug() << t_requestUrl;

    QUrl url(t_requestUrl);
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    for (QString queryKey : t_queryItems.keys())
        query.addQueryItem(queryKey, t_queryItems[queryKey]);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}
