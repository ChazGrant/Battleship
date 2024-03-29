#include "loginform.h"
#include "ui_loginform.h"

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

LoginForm::~LoginForm()
{
    delete ui;
}

void LoginForm::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF delta = event->globalPos() - m_mouse_point;
    move(delta.toPoint());

    event->accept();
}

void LoginForm::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton ||
        event->button() == Qt::RightButton ||
        event->button() == Qt::MiddleButton) {
        m_mouse_point = event->pos();
        event->accept();
    }
}

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

void LoginForm::getLoginStatus(QNetworkReply *reply)
{
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getLoginStatus(QNetworkReply* )));

    QString strReply = reply->readAll();
    QJsonObject jsonObj = QJsonDocument::fromJson(strReply.toUtf8()).object();

    bool login_successful = jsonObj["login_successful"].toBool();
    if (login_successful) {
        QString username = jsonObj["username"].toString();
        QMessageBox msgBox;
        msgBox.setWindowTitle("Вход успешен");
        msgBox.setText("Ваше имя пользователя: " + username);
        msgBox.exec();
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Вход не успешен");
        msgBox.setText("Введён неверный айди или пароль");
        msgBox.exec();
    }

}

void LoginForm::getRegistrateStatus(QNetworkReply *reply)
{
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getRegistrateStatus(QNetworkReply* )));

    QString strReply = reply->readAll();
    QJsonObject jsonObj = QJsonDocument::fromJson(strReply.toUtf8()).object();

    if (jsonObj.keys().size() == 0) {
        return showMessage("Во время обращения к серверу возникли ошибки", QMessageBox::Icon::Critical);
    }
    bool registrate_successful = jsonObj["registrate_successful"].toBool();
    if (registrate_successful) {
        int user_id = jsonObj["user_id"].toInt();
        QMessageBox msgBox;
        msgBox.setWindowTitle("Вход успешен");
        msgBox.setText("Вам присвоен id: " + QString::number(user_id));
        msgBox.exec();
    } else {
        QString error_message = jsonObj["error_message"].toString();
        QMessageBox msgBox;
        msgBox.setWindowTitle("Вход не успешен");
        msgBox.setText(error_message);
        msgBox.exec();
    }
}
