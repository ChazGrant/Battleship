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

#include <QMenu>

#include "additionalfunctions.h"
#include "mainwindow.h"
#include "friendadder.h"
#include "inputgameid.h"


enum FriendRequestAction {
    DECLINE_REQUEST,
    ACCEPT_REQUEST
};

enum FriendAction {
    SEND_FRIENDLY_DUEL_REQUEST,
    DELETE_FRIEND
};

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

    void showFriendRequestsContextMenu(const QPoint &point);
    void showFriendsContextMenu(const QPoint &point);

    void interactWithFriend(QString t_friendUserName, int t_action);
    void processFriendRequestAction(QString t_friendUserName, int t_action);

    void on_connectToExistingGame_clicked();
    void openMainWindow(QString t_gameId);

    void updateFriendsTab(int t_tabIndex);

    void connectToCreatedGame(QNetworkReply* t_reply);
    void fillFriendsTab(QNetworkReply *t_reply);
    void fillFriendsRequestsTab(QNetworkReply *t_reply);
    void getFriendRequestStatus(QNetworkReply *t_reply);
    void getIncomingFriendRequestProcessStatus(QNetworkReply *t_reply);
    void getDeleteFriendRequestStatus(QNetworkReply *t_reply);
private:
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
    //! Указатель на класс InputGameID
    InputGameID *m_inputGameIdWindow;
    //! Идентификатор пользователя
    int m_userId;

    QStringList friendRequestsActionsText;
    QStringList friendsActionsText;
    QList<FriendRequestAction> friendRequestsActionsData;
    QList<FriendAction> friendsActionsData;
    QMenu *menu;

    void setActionsLists();

    void openFriendAdder();    
};

#endif // MAINMENU_H
