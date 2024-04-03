#ifndef ADDITIONALFUNCTIONS_H
#define ADDITIONALFUNCTIONS_H

#include <QString>
#include <QMessageBox>

#include <QNetworkRequest>
#include <QNetworkAccessManager>

inline void showMessage(const QString t_messageText, QMessageBox::Icon t_messageIcon);
inline void sendServerRequest(const QString t_requestUrl, QMap<QString, QString> t_queryParams, QNetworkAccessManager *t_manager);

#endif // ADDITIONALFUNCTIONS_H
