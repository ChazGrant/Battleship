#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include <QDebug>
#include <QMessageBox>

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>

#include <QMouseEvent>

#include <QJsonObject>
#include <QJsonDocument>

#include <QCryptographicHash>

#include "additionalfunctions.h"
#include "mainmenu.h"

// Сокеты
#include <QtWebSockets/QWebSocket>


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
    void getLoginStatus(QNetworkReply *t_reply);
    void getRegistrateStatus(QNetworkReply *t_reply);

    void slotConnected();
    void slotDisconnected();
    void slotError(QAbstractSocket::SocketError);
    void slotReceiveTextMessage(QString t_textMessage);

private:
    //! Указатель на виджет класса
    Ui::LoginForm *ui;
    //! Точка на экране
    QPoint m_mouse_point;
    //! Указатель на обработчик запросов
    QNetworkAccessManager *m_manager;

    void sendSocketRequest();

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent* event);


    QString generateSalt(QString t_firstPart, QString t_secondPart);

    MainMenu *window;
    QWebSocket  *m_pWebSocket;

    void login();
    void registrate();
};

#endif // LOGINFORM_H
