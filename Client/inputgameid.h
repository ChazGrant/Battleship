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
    explicit InputGameID(int t_userId, QWidget *parent = nullptr);
    ~InputGameID();

signals:
    /*! @brief Сигнал о том, что подключение было принято
     *
     * @param gameId Идентификатор игры
     *
     * @return void
    */
    void connectionAccepted(QString t_gameId);

private slots:
    void on_connectToGameButton_clicked();
    void connectToGame(QNetworkReply* t_reply);

    void on_cancelButton_clicked();

private:
    //! Указатель на виджет класса
    Ui::InputGameID *ui;
    //! Указатель на обработчик запросов
    QNetworkAccessManager *m_manager;
    //! Идентификатор игры
    QString m_gameId;
    //! Идентификатор пользователя
    QString m_userId;
};

#endif // INPUTGAMEID_H
