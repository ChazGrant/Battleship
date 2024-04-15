#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMainWindow>

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

#include <QMessageBox>
#include <QCloseEvent>

// HTTP запросы
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QHttpPart>

#include <QMenu>

// Сокеты
#include <QtWebSockets/QWebSocket>

#include "ACTIONS_ENUMS.cpp"

#include "additionalfunctions.h"
#include "mainwindow.h"
#include "friendadder.h"
#include "inputgameid.h"
#include "gameinvitenotifier.h"


enum FriendRequestAction {
    DECLINE_REQUEST_ACTION,
    ACCEPT_REQUEST_ACTION
};

enum FriendAction {
    SEND_FRIENDLY_DUEL_REQUEST_ACTION,
    DELETE_FRIEND_ACTION
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

signals:
    void friendsPulled();

private slots:
    void on_createNewGameButton_clicked();

    void showFriendRequestsContextMenu(const QPoint &t_point);
    void showFriendsContextMenu(const QPoint &t_point);

    void interactWithFriend(QString t_friendUserName, int t_action);
    void processFriendRequestAction(QString t_friendUserName, int t_action);

    void on_connectToExistingGame_clicked();
    void openMainWindow(QString t_gameId);

    void updateFriendsTab(int t_tabIndex);

    // Слоты сокетов
    void onFriendsUpdateSocketConnected();
    void onFriendsUpdateSocketDisconnected();
    void onFriendsUpdateSocketMessageReceived(QString t_textMessage);
    void onFriendsUpdateSocketErrorOccurred(QAbstractSocket::SocketError t_socketError);

    void onFriendlyDuelSocketConnected();
    void onFriendlyDuelSocketDisconnected();
    void onFriendlyDuelSocketMessageReceived(QString t_textMessage);
    void onFriendlyDuelSocketErrorOccurred(QAbstractSocket::SocketError t_socketError);

    void connectToCreatedGame(QNetworkReply* t_reply);
    void fillFriendsTab(QNetworkReply *t_reply);
    void fillFriendsRequestsTab(QNetworkReply *t_reply);
private:
    //! Точка на экране
    QPoint m_mouse_point;
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);

    void getFriends();
    void getFriendsRequests();
    void sendFriendRequest(int t_friendId);
    void sendFriendlyDuelRequest(QString t_friendUsername);
    void deleteFriend(int t_userId, QString t_friendUserName);
    void openFriendAdder();

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

    // Сокеты
    QWebSocket *m_friendsUpdateSocket;
    QWebSocket *m_friendlyDuelSocket;

    QUrl m_friendsUpdateSocketUrl;
    QUrl m_friendlyDuelSocketUrl;

    void initSockets();
    void initFriendsUpdateSocket();
    void initFriendlyDuelSocket();
    void sendSocketMessage();

    //! Подписи для меню обработки запросов в друзья
    QStringList friendRequestsActionsText;
    //! Подписи для меню друзей
    QStringList friendsActionsText;
    //! Действия для меню обработки запросов в друзья
    QList<FriendRequestAction> friendRequestsActionsData;
    //! Действия для меню друзей
    QList<FriendAction> friendsActionsData;
    //! Указатель на контекстное меню
    QMenu *menu;

    void setActionsLists();
};

#endif // MAINMENU_H
