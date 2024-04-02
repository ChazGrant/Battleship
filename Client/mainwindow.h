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
    explicit MainWindow(QWidget *parent = nullptr, QString t_game_id="", int t_user_id=0);
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

private:
    //! Указатель на виджет класса
    Ui::MainWindow *ui;
    //! Указатель на обработчик запросов
    QNetworkAccessManager* m_manager;
    //! Идентификатор игры
    QString m_gameId;
    //! Идентификатор пользователя
    int m_userId;
    //! Было нажато подтверждение закрытия окна
    bool m_closeEventIsAccepted;

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
    void waitForGameStart();
    void waitForTurn();
    void getDamagedCells();
    void checkForWinner();
    void shoot();

};
#endif // MAINWINDOW_H
