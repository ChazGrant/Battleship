#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMainWindow>

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

#include <QMessageBox>
#include <QCloseEvent>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QHttpPart>

#include "mainwindow.h"
#include "additionalfunctions.h"
#include "friendadder.h"


namespace Ui {
class MainMenu;
}

class MainMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainMenu(int t_userId, QWidget *parent = nullptr);
    ~MainMenu();

private slots:
    void on_createNewGameButton_clicked();
    void connectToCreatedGame(QNetworkReply* t_reply);

    void on_connectToExistingGame_clicked();
    void openMainWindow(QString t_gameId);

    void fillFriendsTab(QNetworkReply *reply);
    void getFriendRequestStatus(QNetworkReply *reply);
private:
    void sendServerRequest(QString t_requestUrl, QMap<QString, QString> t_queryItems);

    void getFriends();
    void sendFriendRequest(int t_friendId);
    //! Указатель на виджет класса
    Ui::MainMenu *ui;
    //! Указатель на обработчик запросов
    QNetworkAccessManager* m_manager;
    //! Указатель на класс MainWindow
    MainWindow *m_mainWindow;
    //! Указатель на класс FriendAdder
    FriendAdder *m_friendAdderWindow;
    //! Идентификатор пользователя
    int m_userId;

    void openFriendAdder();
};

#endif // MAINMENU_H
