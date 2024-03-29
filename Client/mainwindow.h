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
    explicit MainWindow(QWidget *parent = nullptr, QString t_game_id="", QString t_user_id="");
    ~MainWindow();

    void fire(QTableWidgetItem *item);
    bool gameIsOver();

    virtual void closeEvent(QCloseEvent *event) override;

private slots:
    void on_placeShipButton_clicked();

    // Обработка ответов с сервера
    void setShipsAmountLabel(QNetworkReply* reply);
    void placeShip(QNetworkReply* reply);
    void getGameState(QNetworkReply* reply);
    void getUserIdTurn(QNetworkReply* reply);
    void fillField(QNetworkReply* reply);
    void getFireStatus(QNetworkReply* reply);
    void getWinner(QNetworkReply* reply);
    void acceptCloseEvent(QNetworkReply* reply);

private:
    Ui::MainWindow *ui;

    QNetworkAccessManager* m_manager;
    QString gameId, userId;
    bool closeEventIsAccepted;

    // Таймеры
    QTimer *timerForGameStart;
    QTimer *timerForUserTurn;

    bool getErrorMessage(QJsonObject jsonObj);

    // Устанавливает количество рядов и колонок
    void setTable();
    void setOpponentTable();

    // Убивает корабль, закрашивая его части красным цветом
    void findDeadShips();

    void lockAllButtons(bool lockState);

    // Запросы на сервер
    void getShipsAmountResponse();
    void waitForGameStart();
    void waitForTurn();
    void getDamagedCells();
    void checkForWinner();
    void shoot();

};
#endif // MAINWINDOW_H
