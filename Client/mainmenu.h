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
    void connectToCreatedGame(QNetworkReply* reply);

    void on_connectToExistingGame_clicked();
    void openMainWindow(QString gameId);

private:
    QNetworkAccessManager* m_manager;
    Ui::MainMenu *ui;
    MainWindow *window;

    QString userId;
};

#endif // MAINMENU_H
