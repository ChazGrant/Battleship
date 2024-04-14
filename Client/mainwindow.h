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
    explicit MainWindow(const QString t_gameId, int t_userId, QString t_gameInviteId, QWidget *parent = nullptr);
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

    // Слоты сокетов
    void onGameSocketConnected();
    void onGameSocketDisconnected();
    void onGameSocketMessageReceived(QString t_textMessage);
    void onGameSocketErrorOccurred(QAbstractSocket::SocketError t_socketError);

    void onChatSocketConnected();
    void onChatSocketDisconnected();
    void onChatSocketMessageReceived(QString t_textMessage);
    void onChatSocketErrorOccurred(QAbstractSocket::SocketError t_socketError);

private:
    //! Указатель на виджет класса
    Ui::MainWindow *ui;
    //! Указатель на обработчик запросов
    QNetworkAccessManager* m_manager;
    //! Идентификатор игры
    const QString m_gameId;
    //! Идентификатор пользователя
    const int m_userId;
    //! Было нажато подтверждение закрытия окна
    bool m_closeEventIsAccepted;

    // Сокеты
    //! Сокет для обработки игровых действий
    QWebSocket *m_gameSocket;
    //! Сокет для общения с противником
    QWebSocket *m_ChatSocket;

    // Таймеры
    //! Указатель на таймер ожидания начала игры
    QTimer *m_timerForGameStart;
    //! Указатель на таймер ожидания начала хода
    QTimer *m_timerForUserTurn;

    bool getErrorMessage(QJsonObject t_jsonObj);

    // Устанавливает количество рядов и колонок
    void setTable();
    void setOpponentTable();

    // Запросы на сервер
    void getShipsAmountResponse();

    // ----------------------------- //
    void waitForGameStart();
    void waitForTurn();
    void getDamagedCells();
    void checkForWinner();
    // ----------------------------- //

    void shoot();

};
#endif // MAINWINDOW_H
