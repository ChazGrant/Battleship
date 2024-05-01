#include "additionalfunctions.h"


/*! @brief Выводит сообщение на экран
 *
 *  @param t_message_text Текст сообщения
 *  @param t_message_icon Иконка для окна сообщения
 *
 *  @return void
*/
inline void showMessage(const QString t_messageText, const QMessageBox::Icon t_messageIcon) {
    QMessageBox msgbox;
    msgbox.setText(t_messageText);
    msgbox.setIcon(t_messageIcon);
    if (t_messageIcon == QMessageBox::Icon::Critical) {
        msgbox.setWindowTitle("Ошибка");
    } else if (t_messageIcon == QMessageBox::Icon::Information) {
        msgbox.setWindowTitle("Информация");
    }
    msgbox.exec();
}

/*! @brief Отправка POST запроса на указанный адрес сервера
 *
 *  @param t_requestUrl Адрес на который нужно отправить запрос
 *  @param t_queryItems Хэш-таблица, содержащая имя параметра и значение
 *  @param *t_manager Указатель на менеджер отправки запросов
 *
 *  @return void
*/
inline void sendServerRequest(const QString t_requestUrl,
                              QMap<QString, QString> t_queryParams,
                              QNetworkAccessManager *t_manager)
{
    QUrl url(t_requestUrl);
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    for (QString queryKey : t_queryParams.keys()) {
        query.addQueryItem(queryKey, t_queryParams[queryKey]);
    }

    t_manager->post(request, query.toString(QUrl::FullyEncoded).toUtf8());
}

/*! @brief Превращает json объект в строку
 *
 *  @param t_jsonObj json объект который нужно конвертировать
 *
 *  @return QString
*/
QString convertJsonObjectToString(const QJsonObject t_jsonObj)
{
    return QString(QJsonDocument(t_jsonObj).toJson(
           QJsonDocument::Compact).toStdString().c_str());
}
