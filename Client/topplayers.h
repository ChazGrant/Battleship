#ifndef TOPPLAYERS_H
#define TOPPLAYERS_H

#include <QWidget>

#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>


namespace Ui {
class TopPlayers;
}

class TopPlayers : public QWidget
{
    Q_OBJECT

public:
    explicit TopPlayers(QJsonArray t_leagues,
                        QJsonObject t_playersByCups,
                        QJsonObject t_playersBySilverCoins,
                        QJsonObject t_playersByWinstreak,
                        QWidget *parent = nullptr);
    ~TopPlayers();

private slots:
    void onFilterChanged();
    void initTopPlayersTable();

private:
    //! Указатель на виджет класса
    Ui::TopPlayers *ui;

    QJsonObject m_playersByCups;
    QJsonObject m_playersBySilverCoins;
    QJsonObject m_playersByWinstreak;

    QJsonArray m_selectedPlayers;

    void getTopPlayers();
    void showTopPlayers();

    void fillAllLeagues(QJsonArray t_leagues);

    void fillAvailableTopSorting();
    void setSortingKeys();
};

#endif // TOPPLAYERS_H
