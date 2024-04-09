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

    ui->loginUserIdLineEdit->setText("1");
    ui->loginPasswordLineEdit->setText("password");

    m_manager = new QNetworkAccessManager(this);

    this->setWindowFlags(Qt::FramelessWindowHint);

    connect(ui->loginButton, &QPushButton::clicked, this, &LoginForm::login);
    connect(ui->registrateButton, &QPushButton::clicked, this, &LoginForm::registrate);

    connect(ui->exitButton, &QPushButton::clicked, this, &LoginForm::close);
    connect(ui->exit2Button, &QPushButton::clicked, this, &LoginForm::close);
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
    QMap<QString, QString> queryParams;

    queryParams["user_id"] = ui->loginUserIdLineEdit->text();
    queryParams["password"] = ui->loginPasswordLineEdit->text();

    QObject::connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(getLoginStatus(QNetworkReply*)));
    sendServerRequest("http://127.0.0.1:8000/users/login/", queryParams, m_manager);
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
    QMap<QString, QString> queryParams;

    QString email = ui->registrateEmailLineEdit->text();
    QString userName = ui->registrateUsernameLineEdit->text();
    QString password = ui->registratePasswordLineEdit->text();
    if (email == "" || userName == "" || password == "")
        return showMessage("Одно из полей не было заполнено", QMessageBox::Icon::Critical);

    queryParams["user_name"] = ui->registrateUsernameLineEdit->text();
    queryParams["password"] = ui->registratePasswordLineEdit->text();
    queryParams["email"] = ui->registrateEmailLineEdit->text();

    QObject::connect(m_manager, SIGNAL(finished(QNetworkReply *)),
                     SLOT(getRegistrateStatus(QNetworkReply *)));
    sendServerRequest("http://127.0.0.1:8000/users/registrate/", queryParams, m_manager);
}

/*! @brief Вход в систему
 *
 *  @details Получает ответ от сервера и в зависимости от него выдаёт ошибку или
 *  открывает форму Лобби
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
*/
void LoginForm::getLoginStatus(QNetworkReply *reply)
{
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getLoginStatus(QNetworkReply* )));

    const QString strReply = reply->readAll();
    QJsonObject jsonObj = QJsonDocument::fromJson(strReply.toUtf8()).object();

    if (jsonObj.isEmpty()) {
        showMessage("Сервер недоступен", QMessageBox::Icon::Critical);
        close();
        return;
    }

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
*/
void LoginForm::getRegistrateStatus(QNetworkReply *reply)
{
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getRegistrateStatus(QNetworkReply* )));

    const QString strReply = reply->readAll();
    QJsonObject jsonObj = QJsonDocument::fromJson(strReply.toUtf8()).object();

    if (jsonObj.isEmpty()) {
        showMessage("Сервер недоступен", QMessageBox::Icon::Critical);
        close();
        return;
    }

    bool registrate_successful = jsonObj["registration_successful"].toBool();
    if (registrate_successful) {
        int userId = jsonObj["user_id"].toInt();
        window = new MainMenu(userId);
        window->show();
        close();
    } else {
        const QString error_message = jsonObj["error"].toString();
        QMessageBox msgBox;
        msgBox.setWindowTitle("Вход не успешен");
        msgBox.setText(error_message);
        msgBox.exec();
    }
}
