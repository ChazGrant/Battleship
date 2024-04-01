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
    /*! @brief Сигнал о том, что подключение было принято
     *
     * @param gameId Идентификатор игры
     *
     * @return void
    */
    void acceptConnection(QString gameId);

private slots:
    void on_connectToGameButton_clicked();
    void connectToGame(QNetworkReply* reply);

    void on_cancelButton_clicked();

private:
    //! Указатель на виджет класса
    Ui::InputGameID *ui;
    //! Указатель на обработчик запросов
    QNetworkAccessManager *m_manager;
    //! Идентификатор игры
    QString gameId;
    //! Идентификатор пользователя
    QString userId;
};

#endif // INPUTGAMEID_H
