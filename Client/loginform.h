#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include <QDebug>
#include <QMessageBox>

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>

#include <QJsonObject>
#include <QJsonDocument>


namespace Ui {
class LoginForm;
}

class LoginForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoginForm(QWidget *parent = nullptr);
    ~LoginForm();

private slots:
    void getLoginStatus(QNetworkReply *reply);
    void getRegistrateStatus(QNetworkReply *reply);

private:
    Ui::LoginForm *ui;

    QNetworkAccessManager *m_manager;

    void login();
    void registrate();
};

#endif // LOGINFORM_H
