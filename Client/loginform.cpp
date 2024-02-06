#include "loginform.h"
#include "ui_loginform.h"

LoginForm::LoginForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager(this);

    connect(ui->login_pushButton, &QPushButton::clicked, this, &LoginForm::login);
    // connect(ui->registrate_pushButton, &QPushButton::clicked, this, &LoginForm::registrate);
}

LoginForm::~LoginForm()
{
    delete ui;
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
    QNetworkRequest request(QUrl("http://127.0.0.1:8000/users/registrate/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;
    query.addQueryItem("user_name", user_id);
    query.addQueryItem("password", password);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL(finished(QNetworkReply *)), SLOT(getRegistrateStatus(QNetworkReply *)));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}

void LoginForm::getLoginStatus(QNetworkReply *reply)
{
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

    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getLoginStatus(QNetworkReply* )));
}

void LoginForm::getRegistrateStatus(QNetworkReply *reply)
{
    QString strReply = reply->readAll();
    QJsonObject jsonObj = QJsonDocument::fromJson(strReply.toUtf8()).object();

    bool registrate_successful = jsonObj["registrate_successful"].toBool();
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

    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getLoginStatus(QNetworkReply* )));
}
