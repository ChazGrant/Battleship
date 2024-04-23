#ifndef SHOP_H
#define SHOP_H

#include <QMainWindow>

#include "additionalfunctions.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QJsonObject>
#include <QJsonArray>


namespace Ui {
class Shop;
}

class Shop : public QMainWindow
{
    Q_OBJECT

public:
    explicit Shop(int t_userId, QWidget *parent = nullptr);
    ~Shop();

private slots:
    void fillWeaponsComboBoxes(QNetworkReply *);
    void getWeaponBoughtStatus(QNetworkReply *);

private:
    Ui::Shop *ui;

    void fillWeaponsInStock(QJsonArray t_userWeapons);
    void fillWeaponsShop(QJsonArray t_shopWeapons);

    void getWeapons();
    void buyWeapon();
    void sellWeapon();

    int m_userId;
    QNetworkAccessManager *m_manager;
};

#endif // SHOP_H
