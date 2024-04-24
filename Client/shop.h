#ifndef SHOP_H
#define SHOP_H

#include <QMainWindow>

#include "additionalfunctions.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QTableWidgetItem>
#include <QSpinBox>

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
    void getWeaponSoldStatus(QNetworkReply *);
    void setCoinsAmount(QNetworkReply *);
    void setTotalPrices(int);

private:
    Ui::Shop *ui;

    void fillWeaponsInStock(QJsonArray t_userWeapons);
    void fillWeaponsShop(QJsonArray t_shopWeapons);

    void getCoinsAmount();
    void getWeapons();

    void renderCoinsAmount(double t_coins);

    void buyWeapon(bool t_buyAll = false);
    void sellWeapon(bool t_sellAll = false);

    void initShootingRange();
    void setCurrentWeaponRange();
    void clearHighlightedCells();
    void highlightCell(QTableWidgetItem *t_item);

    void rearrangeWeaponsInStockTable(int t_fromRow);

    int m_userId;
    QMap<QString, QList<int>> m_weaponsRange;
    QList<int> m_currentRange = {1, 1};
    QNetworkAccessManager *m_manager;
};

#endif // SHOP_H
