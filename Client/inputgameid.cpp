#include "inputgameid.h"
#include "ui_inputgameid.h"


InputGameID::InputGameID(QWidget *parent, QString userId) :
    QDialog(parent),
    ui(new Ui::InputGameID)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager(this);
    this->gameId = "";
    this->userId = userId;
}

InputGameID::~InputGameID()
{
    delete ui;
}

void InputGameID::on_connectToGameButton_clicked()
{
    this->gameId = ui->gameIDTextEdit->toPlainText();
    if (this->gameId == "")
        return;

    QNetworkRequest request(QUrl("http://127.0.0.1:8000/games/connect_to_game/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    // Ищем на сервере игру с таким айди
    QUrlQuery query;
    query.addQueryItem("game_id", ui->gameIDTextEdit->toPlainText().toUtf8());
    query.addQueryItem("user_id", this->userId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL(finished(QNetworkReply *)), SLOT(connectToGame(QNetworkReply *)));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));

}

void InputGameID::connectToGame(QNetworkReply *reply)
{
    QString strReply = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(strReply.toUtf8()).object();

    if (jsonObj.contains("game_id"))
    {
        // Подключены к игре
        emit acceptConnection(jsonObj["game_id"].toString());
        this->close();
    }
    else if(jsonObj.contains("errors"))
    {
        // Вывод ошибок
        QMessageBox msgBox;
        msgBox.setWindowTitle("Ошибка");
        msgBox.setText(jsonObj["errors"].toString());
        msgBox.exec();
    }
}

void InputGameID::on_cancelButton_clicked()
{
    this->close();
}
