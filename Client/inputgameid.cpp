#include "inputgameid.h"
#include "ui_inputgameid.h"


/*! @brief Конструктор класса
 *
 *  @details Создаёт объект класса QNetworkAccessManager, идентификатор игры и пользователя
 *
 *  @param t_userId Идентификатор пользователя
 *  @param *parent Родительский класс
 *
 *  @return InputGameID
*/
InputGameID::InputGameID(int t_userId, QWidget *parent) :
    QDialog(parent)
    , ui(new Ui::InputGameID)
    , m_userId(t_userId)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager(this);
    this->m_gameId = "";
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
    this->m_gameId = ui->gameIDTextEdit->toPlainText();
    if (this->m_gameId == "")
        return;

    QNetworkRequest request(QUrl("http://127.0.0.1:8000/games/connect_to_game/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    // Ищем на сервере игру с таким айди
    QUrlQuery query;
    query.addQueryItem("game_id", this->m_gameId.toUtf8());
    query.addQueryItem("user_id", this->m_userId);

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
    const QString strReply = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(strReply.toUtf8()).object();

    if (jsonObj.contains("game_id"))
    {
        // Подключены к игре
        emit connectionAccepted(jsonObj["game_id"].toString());
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

/*! @brief Закрывает окно
 *
 *  @return void
*/
void InputGameID::on_cancelButton_clicked()
{
    this->close();
}
