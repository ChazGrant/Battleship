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

#include <string>
#include "additionalfunctions.cpp"

class QNetworkReply;
class QNetworkAccessManager;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


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

    // Обработка ответов с сервера
    void setShipsAmountLabel(QNetworkReply* t_reply);
    void placeShip(QNetworkReply* t_reply);
    void getGameState(QNetworkReply* t_reply);
    void getUserIdTurn(QNetworkReply* t_reply);
    void fillField(QNetworkReply* t_reply);
    void getFireStatus(QNetworkReply* t_reply);
    void getWinner(QNetworkReply* t_reply);
    void acceptCloseEvent(QNetworkReply* t_reply);

    void setWeaponsUsesLeftLabel(QString t_currentText);
    // Слоты сокетов
//    void onGameSocketConnected();
//    void onGameSocketDisconnected();
//    void onGameSocketMessageReceived(QString t_textMessage);
//    void onGameSocketErrorOccurred(QAbstractSocket::SocketError t_socketError);

//    void onChatSocketConnected();
//    void onChatSocketDisconnected();
//    void onChatSocketMessageReceived(QString t_textMessage);
//    void onChatSocketErrorOccurred(QAbstractSocket::SocketError t_socketError);

private:
    //! Указатель на виджет класса
    Ui::MainWindow *ui;
    //! Указатель на обработчик запросов
    QNetworkAccessManager* m_manager;

    // Параметры подключения к игре
    //! Идентификатор игры
    const QString m_gameId;
    //! Идентификатор пользователя
    const int m_userId;
    //! Идентификатор приглашения в игру(если есть)
    const QString m_gameInviteId;

    //! Было нажато подтверждение закрытия окна
    bool m_closeEventIsAccepted;

    // Сокеты
    //! Сокет для обработки игровых действий
    QWebSocket *m_gameSocket;
    //! Сокет для общения с противником
    QWebSocket *m_ChatSocket;

    //! Последняя ячейка, на которую навёл пользователь
    QTableWidgetItem *m_lastHighlightedItem;
    //! Последняя ячейка, на которую нажал пользователь
    QTableWidgetItem *m_lastMarkedItem;
    //! Координаты ячейки, по которой нужно стрелять
    QMap<QString, int> m_firePosition;
    void highlightOpponentCell(QTableWidgetItem *t_item);
    void markOpponentCell(QTableWidgetItem *t_item);

    // Таймеры
    //! Указатель на таймер ожидания подключения противника
    QTimer *m_oponnentConnectionTimer;
    //! Указатель на таймер ожидания хода противника
    QTimer *m_userTurnTimer;

    bool getErrorMessage(QJsonObject t_jsonObj);

    // Устанавливает количество рядов и колонок
    void createTablesWidgets();
    void createUserTable();
    void createOpponentTable();
    void fillWeaponsComboBox();

    // Запросы на сервер
    void getShipsAmountResponse();

    // ----------------------------- //
    void waitForGameStart();
    void waitForTurn();
    void getDamagedCells();
    void checkForWinner();
    // ----------------------------- //

    // DEBUG
    QMap<QString, int> m_availableWeapons = {
        {"Самолёт", 1},
        {"Ядерная бомба", 2},
        {"Бомба", 4}
    };

    QMap<QString, QMap<QString, int>> m_weaponSelection = []() {
      QMap<QString, QMap<QString, int>> result;
      QMap<QString, int> innerResult;
      innerResult["x"] = 8;
      innerResult["y"] = 0;
      result["Самолёт"] = innerResult;

      innerResult["x"] = 4;
      innerResult["y"] = 4;
      result["Ядерная бомба"] = innerResult;

      innerResult["x"] = 2;
      innerResult["y"] = 2;
      result["Бомба"] = innerResult;

      return result;
    }();

    bool m_weaponActivated;

    void makeTurn();

};
#endif // MAINWINDOW_H
