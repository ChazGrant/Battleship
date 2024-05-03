#include "topplayers.h"
#include "ui_topplayers.h"


inline void swap(QJsonValueRef t_firstItem, QJsonValueRef t_secondItem) {
    QJsonValue temp(t_firstItem);
    t_secondItem = t_firstItem;
    t_firstItem = temp;
}

TopPlayers::TopPlayers(QJsonArray t_leagues,
                       QJsonObject t_playersByCups,
                       QJsonObject t_playersBySilverCoins,
                       QJsonObject t_playersByWinstreak,
                       QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TopPlayers),
    m_playersByCups(t_playersByCups),
    m_playersBySilverCoins(t_playersBySilverCoins),
    m_playersByWinstreak(t_playersByWinstreak)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);

    connect(ui->availableTopSortingComboBox, &QComboBox::currentTextChanged,
            this, &TopPlayers::onFilterChanged);
    connect(ui->leagueComboBox, &QComboBox::currentTextChanged, this, &TopPlayers::onFilterChanged);

    fillAvailableTopSorting();
    fillAllLeagues(t_leagues);

    initTopPlayersTable();
    showTopPlayers();
}

TopPlayers::~TopPlayers()
{
    delete ui;
}

void TopPlayers::onFilterChanged()
{
    const QString selectedCriteria = ui->availableTopSortingComboBox->currentText();
    const QString selectedLeague = ui->leagueComboBox->currentText();

    if (selectedCriteria == "Топ по кубкам") {
        m_selectedPlayers = m_playersByCups[selectedLeague].toArray();
    } else if (selectedCriteria == "Топ по монетам") {
        m_selectedPlayers = m_playersBySilverCoins[selectedLeague].toArray();
    } else if (selectedCriteria == "Топ по количеству побед подряд") {
        m_selectedPlayers = m_playersByWinstreak[selectedLeague].toArray();
    }

    initTopPlayersTable();
    showTopPlayers();
}

void TopPlayers::initTopPlayersTable()
{
    ui->topPlayersTableWidget->clear();

    int totalRows = m_selectedPlayers.size();
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

    showTopPlayers();
}

void TopPlayers::showTopPlayers()
{
    int row = 0;
    for (QJsonValueRef player : m_selectedPlayers) {
        QJsonObject currentPlayer = player.toObject();
        QString playerName = currentPlayer["user_name"].toString();
        QString playerValue = QString::number(currentPlayer["value"].toDouble());
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
