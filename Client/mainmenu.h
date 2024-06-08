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
#include "shop.h"
#include "gameinvitenotifier.h"
#include "topplayers.h"


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
    explicit MainMenu(int t_userId, QString t_userName, QWidget *parent = nullptr);
    ~MainMenu();

signals:
    void friendsPulled();
    void widgetClosed();

private slots:
    void showFriendRequestsContextMenu(const QPoint &t_point);
    void showFriendsContextMenu(const QPoint &t_point);

    void interactWithFriend(QString t_friendUserName, int t_action);
    void processFriendRequestAction(QString t_friendUserName, int t_action);

    void openMainWindow(QString t_gameId, QString t_gameInviteId = "");
    void openShopWidget();

    void updateFriendsTab(int t_tabIndex);

    // Слоты сокетов
    void onFriendsUpdateSocketConnected();
    void onFriendsUpdateSocketMessageReceived(QString t_textMessage);
    void onFriendsUpdateSocketErrorOccurred(QAbstractSocket::SocketError t_socketError);

    void onGameCreatorSocketConnected();
    void onGameCreatorSocketDisconnected();
    void onGameCreatorSocketMessageReceived(QString t_textMessage);
    void onGameCreatorSocketErrorOccurred(QAbstractSocket::SocketError t_socketError);

    void fillFriendsTab(QNetworkReply *t_reply);
    void fillFriendsRequestsTab(QNetworkReply *t_reply);
    void openTopPlayersWidget(QNetworkReply *t_reply);
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

    void getTopPlayers();

    void createGame(bool t_opponentIsAI=false);
    void inputExistingGameId();
    void connectToRandomGame();

    void showNotImplementedFeature();

    //! Указатель на виджет класса
    Ui::MainMenu *ui;
    //! Указатель на обработчик запросов
    QNetworkAccessManager* m_manager;
    //! Указатель на класс MainWindow
    MainWindow *m_mainWindow;
    //! Указатель на класс FriendAdder
    FriendAdder *m_friendAdderWindow;
    //! Указатель на класс Shop
    Shop *m_shopWindow;
    //! Указатель на класс GameInviteNotifier
    GameInviteNotifier *m_gameInviteNotifier;
    //! Указатель на класс TopPlayers
    TopPlayers *m_topPlayersWidget;
    //! Идентификатор пользователя
    const int m_userId;
    //! Имя пользователя
    const QString m_userName;
    //! Если пользователь отключился сам
    bool m_disconnectedByUser = false;

    void closeEvent(QCloseEvent *event);

    // Сокеты
    //! Сокет обновления друзей
    QWebSocket *m_friendsUpdateSocket;
    //! Сокет создания игр
    QWebSocket *m_gameCreatorSocket;

    //! Адрес, обрабатывающий обновления друзей
    QUrl m_friendsUpdateSocketUrl;
    //! Адрес, обрабатывающий создание игр
    QUrl m_gameCreatorSocketUrl;

    void initSockets();
    void initFriendsUpdateSocket();
    void initGameCreatorSocket();

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
