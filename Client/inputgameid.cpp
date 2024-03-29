#include "inputgameid.h"
#include "ui_inputgameid.h"


/*! @brief Конструктор класса
 *
 *  @param *parent Родительский класс
 *  @param userId Идентификатор пользователя
 *
 *  @details Создаёт объект класса QNetworkAccessManager, идентификатор игры и пользователя
 *
 *  @return InputGameID
*/
InputGameID::InputGameID(QWidget *parent, QString userId) :
    QDialog(parent),
    ui(new Ui::InputGameID)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager(this);
    this->gameId = "";
    this->userId = userId;
}

//! @brief Деструктор класса
InputGameID::~InputGameID()
{
    delete ui;
}

/*! @brief Обработка события кнопки "Подключиться"
 *
 *  @details Отправляет запрос POST на сервер, передавая в качестве параметров
 *  идентификатор пользователя и игры
 *
 *  @return void
*/
void InputGameID::on_connectToGameButton_clicked()
{
    this->gameId = ui->gameIDTextEdit->toPlainText();
    if (this->gameId == "")
        return;

    QNetworkRequest request(QUrl("http://127.0.0.1:8000/games/connect_to_game/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    // Ищем на сервере игру с таким айди
    QUrlQuery query;
    query.addQueryItem("game_id", this->gameId.toUtf8());
    query.addQueryItem("user_id", this->userId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL(finished(QNetworkReply *)), SLOT(connectToGame(QNetworkReply *)));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));

}

/*! @brief Подключается к игре с полученным идентификатором
 *
 *  @details Получает иденификатор игры от сервера.
 *  Закрывает окно и подтверждает сигнал о подключении к игре
 *
 *  @param *reply Указатель на ответ от сервера
 *
 *  @return void
 *
*/
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

//! @brief Закрывает окно
void InputGameID::on_cancelButton_clicked()
{
    this->close();
}
