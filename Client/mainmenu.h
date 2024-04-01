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

namespace Ui {
class MainMenu;
}

class MainMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainMenu(QWidget *parent = nullptr);
    ~MainMenu();

private slots:
    void on_createNewGameButton_clicked();
    void connectToCreatedGame(QNetworkReply* t_reply);

    void on_connectToExistingGame_clicked();
    void openMainWindow(QString t_gameId);

private:
    //! Указатель на виджет класса
    Ui::MainMenu *ui;
    //! Указатель на обработчик запросов
    QNetworkAccessManager* m_manager;
    //! Указатель на класс MainWindow
    MainWindow *m_window;
    //! Идентификатор пользователя
    QString m_userId;
};

#endif // MAINMENU_H
