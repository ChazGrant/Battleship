#ifndef ADDITIONALFUNCTIONS_H
#define ADDITIONALFUNCTIONS_H

#include <QString>
#include <QMessageBox>

#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>


inline void showMessage(const QString t_messageText, QMessageBox::Icon t_messageIcon);
inline void sendServerRequest(const QString t_requestUrl, QMap<QString, QString> t_queryParams, QNetworkAccessManager *t_manager);
inline QString jsonObjectToQString(const QJsonObject t_jsonObj);

#endif // ADDITIONALFUNCTIONS_H
