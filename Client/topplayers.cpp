#include "topplayers.h"
#include "ui_topplayers.h"


inline void swap(QJsonValueRef t_firstItem, QJsonValueRef t_secondItem) {
    QJsonValue temp(t_firstItem);
    t_secondItem = t_firstItem;
    t_firstItem = temp;
}

TopPlayers::TopPlayers(QJsonObject t_topPlayersByLeague,
                       QJsonArray t_leagues,
                       QJsonObject t_sortingKeys,
                       QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TopPlayers),
    m_playersByLeague(t_topPlayersByLeague),
    m_sortingKeys(t_sortingKeys)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);

    fillAllLeagues(t_leagues);
    setSortingKeys();

    m_currentSortingKey = m_sortingKeys[ui->availableTopSortingComboBox->currentText()].toString();
    sortPlayers();
    initTopPlayersTable();

    showTopPlayers();

    connect(ui->availableTopSortingComboBox, &QComboBox::currentTextChanged,
            this, &TopPlayers::onFilterCriteriaChanged);
    connect(ui->leagueComboBox, &QComboBox::currentTextChanged, this, &TopPlayers::onFilterCriteriaChanged);
}

TopPlayers::~TopPlayers()
{
    delete ui;
}

void TopPlayers::onFilterCriteriaChanged() {
    m_currentSortingKey = m_sortingKeys[ui->availableTopSortingComboBox->currentText()].toString();
    sortPlayers();
    initTopPlayersTable();

    showTopPlayers();
}

void TopPlayers::initTopPlayersTable(const QString t_leagueName)
{

}

void TopPlayers::sortPlayers()
{

    const QString selecetedLeague = ui->leagueComboBox->currentText();
    QJsonArray selectedPlayers = m_playersByLeague[selecetedLeague].toArray();
    m_sortedPlayers = selectedPlayers;
    std::sort(m_sortedPlayers.begin(), m_sortedPlayers.end(), [=](const QJsonValue &v1, const QJsonValue &v2) {
        return v1.toObject()[m_currentSortingKey].toInt() > v2.toObject()[m_currentSortingKey].toInt();
    });
}

void TopPlayers::initTopPlayersTable()
{
    ui->topPlayersTableWidget->clear();

    int totalRows = m_sortedPlayers.size();
    ui->topPlayersTableWidget->setRowCount(totalRows);
    ui->topPlayersTableWidget->setColumnCount(2);
    ui->topPlayersTableWidget->setHorizontalHeaderLabels(QStringList() << "Имя игрока" << "Значение");
}

void TopPlayers::getTopPlayers()
{
    const QString selectedSorting = ui->availableTopSortingComboBox->currentText();
    QString sortBy;
    if (selectedSorting == "Топ по кубкам") {
        sortBy = "player_cups";
    } else if (selectedSorting == "Топ по монетам") {
        sortBy = "player_silver_coins";
    } else if (selectedSorting == "Топ по количеству побед подряд") {
        sortBy = "player_winstreak";
    }

    sortPlayers();
}

void TopPlayers::showTopPlayers()
{
    int row = 0;
    for (QJsonValueRef sortedPlayer : m_sortedPlayers) {
        QJsonObject currentPlayer = sortedPlayer.toObject();
        QString playerName = currentPlayer["player_name"].toString();
        QString playerValue = QString::number(currentPlayer[m_currentSortingKey].toDouble());
        QStringList playerValues = {playerName, playerValue};
        for (int column = 0; column < playerValues.size(); ++column) {
            QTableWidgetItem *item = new QTableWidgetItem(playerValues[column]);
            ui->topPlayersTableWidget->setItem(row, column, item);
        }
        ++row;
    }

    ui->topPlayersTableWidget->resizeColumnsToContents();
}

void TopPlayers::fillAllLeagues(QJsonArray t_leagues)
{
    for (QJsonValueRef leagueName : t_leagues) {
        ui->leagueComboBox->addItem(leagueName.toString());
    }
}

void TopPlayers::fillAvailableTopSorting()
{
    for (QString topSorting : {"Топ по кубкам", "Топ по монетам", "Топ по количеству побед подряд"}) {
        ui->availableTopSortingComboBox->addItem(topSorting);
    }
}

void TopPlayers::setSortingKeys()
{
    for (const QString &key : m_sortingKeys.keys()) {
        ui->availableTopSortingComboBox->addItem(key);
    }
}
