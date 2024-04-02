#include "loginform.h"
#include "ui_loginform.h"

/*! @brief Констркутор класса
 *
 *  @details Создаёт экземпляр класса, делая форму безрамочной
 *
 *  @param *parent Указатель на родительский виджет
 *
 *  @return LoginForm
*/
LoginForm::LoginForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager(this);

    this->setWindowFlags(Qt::FramelessWindowHint);

    connect(ui->login_pushButton, &QPushButton::clicked, this, &LoginForm::login);
    connect(ui->registrate_pushButton, &QPushButton::clicked, this, &LoginForm::registrate);

    connect(ui->exit_pushButton, &QPushButton::clicked, this, &LoginForm::close);
    connect(ui->exit2_pushButton, &QPushButton::clicked, this, &LoginForm::close);
}

//! @brief Дестркутор класса
LoginForm::~LoginForm()
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
void LoginForm::mouseMoveEvent(QMouseEvent* event)
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
void LoginForm::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton ||
        event->button() == Qt::RightButton ||
        event->button() == Qt::MiddleButton) {
        m_mouse_point = event->pos();
        event->accept();
    }
}

/*! @brief Авторизация пользователя
 *
 *  @details Отправляет запрос на сервер, передавая идентификатор пользователя и пароль
 *
 *  @return void
 *
 *  @todo Зашифровывать пароль перед отправкой
*/
void LoginForm::login()
{
    QString user_id = ui->loginUserID_lineEdit->text();
    QString password = ui->loginPassword_lineEdit->text();

    QNetworkRequest request(QUrl("http://127.0.0.1:8000/users/login/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;
    query.addQueryItem("user_id", user_id);
    query.addQueryItem("password", password);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL(finished(QNetworkReply *)), SLOT(getLoginStatus(QNetworkReply *)));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}

/*! @brief Регистрация пользователя
 *
 *  @details Отправляет запрос на сервер, передавая в параметрах
 *  имя пользователя, пароль и почту
 *
 *  @return void
 *
 *  @todo Зашифровывать пароль
*/
void LoginForm::registrate()
{
    QString user_name = ui->registrateUsername_lineEdit->text();
    QString password = ui->registratePassword_lineEdit->text();
    QString email = ui->registrateEmail_lineEdit->text();
    QNetworkRequest request(QUrl("http://127.0.0.1:8000/users/registrate/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;
    query.addQueryItem("user_name", user_name);
    query.addQueryItem("password", password);
    query.addQueryItem("email", email);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
    QObject::connect(m_manager, SIGNAL(finished(QNetworkReply *)), SLOT(getRegistrateStatus(QNetworkReply *)));
}

/*! @brief Вход в систему
 *
 *  @details Получает ответ от сервера и в зависимости от него выдаёт ошибку или
 *  открывает форму Лобби
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
 *
 *  @todo Открывать форму InputGameId
*/
void LoginForm::getLoginStatus(QNetworkReply *reply)
{
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getLoginStatus(QNetworkReply* )));

    QString strReply = reply->readAll();
    QJsonObject jsonObj = QJsonDocument::fromJson(strReply.toUtf8()).object();

    bool login_successful = jsonObj["login_successful"].toBool();
    if (login_successful) {
        int userId = jsonObj["user_id"].toInt();

        window = new MainMenu(userId);
        window->show();
        close();
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Вход не успешен");
        msgBox.setText("Введён неверный айди или пароль");
        msgBox.exec();
    }
}

/*! @brief Получение статуса регистрации пользователя
 *
 *  @details Получает ответ от сервера и открывает форму для ввода идентификатора игры
 *  или выводит текст ошибки
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
 *
 *  @todo Открывать InputGameID
*/
void LoginForm::getRegistrateStatus(QNetworkReply *reply)
{
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getRegistrateStatus(QNetworkReply* )));

    QString strReply = reply->readAll();
    QJsonObject jsonObj = QJsonDocument::fromJson(strReply.toUtf8()).object();

    if (jsonObj.keys().size() == 0) {
        return showMessage("Во время обращения к серверу возникли ошибки", QMessageBox::Icon::Critical);
    }
    bool registrate_successful = jsonObj["registration_successful"].toBool();
    qDebug() << registrate_successful;
    if (registrate_successful) {
        int userId = jsonObj["user_id"].toInt();
        window = new MainMenu(userId);

    } else {
        QString error_message = jsonObj["error"].toString();
        QMessageBox msgBox;
        msgBox.setWindowTitle("Вход не успешен");
        msgBox.setText(error_message);
        msgBox.exec();
    }
}
