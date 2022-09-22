#ifndef INPUTGAMEID_H
#define INPUTGAMEID_H

#include <QDialog>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>

#include <QMessageBox>

namespace Ui {
class InputGameID;
}

class InputGameID : public QDialog
{
    Q_OBJECT

public:
    explicit InputGameID(QWidget *parent = nullptr, QString userId="");
    ~InputGameID();

signals:
    void acceptConnection(QString gameId);

private slots:
    void on_connectToGameButton_clicked();
    void connectToGame(QNetworkReply* reply);

    void on_cancelButton_clicked();

private:
    Ui::InputGameID *ui;
    QNetworkAccessManager *m_manager;
    QString gameId;
    QString userId;
};

#endif // INPUTGAMEID_H
