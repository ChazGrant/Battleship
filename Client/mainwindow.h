#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QDebug>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QTimer>

#include <QCloseEvent>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QHttpPart>
#include <QUrlQuery>

#include <QtWebSockets/QWebSocket>

#include "ACTIONS_ENUMS.cpp"
#include "additionalfunctions.cpp"

class QNetworkReply;
class QNetworkAccessManager;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

const QMap<QString, QStringList> GAME_OVER_CLAUSES = {
    {"opponent_disconnected", {"", "Противник покинул игру"}},
    {"all_ships_are_dead", {"Ваши корабли были уничтожены", "Вы уничтожили все корабли"}}
};

const int FIELD_ROW_COUNT = 10;
const int FIELD_COLUMN_COUNT = 10;

const QColor WHITE = QColor("white");
const QColor GREEN = QColor("darkgreen");
const QColor YELLOW = QColor("yellow");
const QColor RED = QColor("darkred");
const QColor GRAY = QColor("darkgray");
const QColor ORANGE = QColor("orange");


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString t_gameId, const int t_userId,
                        const QString t_gameInviteId="", QWidget *parent = nullptr);
    ~MainWindow();

    virtual void closeEvent(QCloseEvent *event) override;

private slots:
    void on_placeShipButton_clicked();

    void acceptCloseEvent(QNetworkReply* t_reply);

    void setCurrentWeaponAmount(QString t_currentWeaponText);
    // Слоты сокетов
    void onGameSocketConnected();
    void onGameSocketDisconnected();
    void onGameSocketMessageReceived(QString t_textMessage);
    void onGameSocketErrorOccurred(QAbstractSocket::SocketError t_socketError);

    void onChatSocketConnected();
    void onChatSocketMessageReceived(QString t_textMessage);
    void onChatSocketErrorOccurred(QAbstractSocket::SocketError t_socketError);

    void onTimeOut(QTimer *t_timer);
private:
    //! Указатель на виджет класса
    Ui::MainWindow *ui;
    //! Указатель на обработчик запросов
    QNetworkAccessManager* m_manager;

    void showCurrentStateGame(QString t_gameState);
    void setShipsAmountLabel(QJsonObject t_jsonResponse);
    void placeShip(QJsonArray t_cells);

    // Параметры подключения к игре
    //! Идентификатор игры
    const QString m_gameId;
    //! Идентификатор пользователя
    const int m_userId;
    //! Идентификатор приглашения в игру(если есть)
    const QString m_gameInviteId;

    //! Было нажато подтверждение закрытия окна
    bool m_closeEventIsAccepted;

    //! Состояние игры началась или нет
    bool m_gameStarted;

    // Сокеты
    //! Сокет для обработки игровых действий
    QWebSocket *m_gameSocket;
    //! Сокет для общения с противником
    QWebSocket *m_chatSocket;
    //! Адрес для обработки чата
    QUrl m_chatSocketUrl;
    //! Адрес для обработки игры
    QUrl m_gameSocketUrl;

    void initSockets();
    void initGameSocket();
    void initChatSocket();

    void connectToGame();

    int secondsToConnectPassed;
    int secondsToMakeTurnPassed;

    //! Последняя ячейка, на которую навёл пользователь
    QTableWidgetItem *m_lastHighlightedItem = nullptr;
    //! Последняя ячейка, на которую нажал пользователь
    QTableWidgetItem *m_lastMarkedItem = nullptr;
    //! Координаты ячейки, по которой нужно стрелять
    QJsonArray m_firePosition;

    void paintOpponentCells(int t_xStart, int t_yStart, QColor t_color);
    void highlightOpponentCell(QTableWidgetItem *t_item);
    void setFirePosition(QTableWidgetItem *t_item);
    void clearHighlightedCells(QColor t_avoidColor=ORANGE);

    // Таймеры
    //! Указатель на таймер ожидания подключения противника
    QTimer *m_opponentConnectionTimer;
    //! Указатель на таймер ожидания хода противника
    QTimer *m_userTurnTimer;

    bool getErrorMessage(QJsonObject t_jsonObj);

    // Устанавливает количество рядов и колонок
    void createTablesWidgets();
    void createUserTable();
    void createOpponentTable();

    void getAvailableWeapons();
    void fillWeaponsComboBox(QJsonObject jsonObj);

    //! @brief Оружия, которые в наличии у пользователя
    QMap<QString, int> m_availableWeapons;
    //! @brief Радиус поражения оружий
    QMap<QString, QList<int>> m_weaponRange;

    void autoPlaceShips();

    //! @brief Оружие активировано
    bool m_weaponActivated = false;
    //! @brief Размещение кораблей автоматически
    bool m_shipsAutoPlacing = false;

    void makeTurn();
    void markOpponentField(QJsonObject t_jsonObj);
    void markOwnerField(QJsonObject t_jsonObj);

};
#endif // MAINWINDOW_H
