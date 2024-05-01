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
    explicit TopPlayers(QJsonObject t_topPlayersByLeague,
                        QJsonArray t_leagues,
                        QJsonObject t_sortingKeys,
                        QWidget *parent = nullptr);
    ~TopPlayers();

private slots:
    void onFilterCriteriaChanged();

private:
    Ui::TopPlayers *ui;

    void initTopPlayersTable(const QString t_leagueName);

    QJsonObject m_playersByLeague;
    QJsonObject m_sortingKeys;

    void sortPlayers();
    QJsonArray m_sortedPlayers;
    QString m_currentSortingKey;

    void getTopPlayers();
    void showTopPlayers();

    void initTopPlayersTable();

    void fillAllLeagues(QJsonArray t_leagues);

    void fillAvailableTopSorting();
    void setSortingKeys();
};

#endif // TOPPLAYERS_H
