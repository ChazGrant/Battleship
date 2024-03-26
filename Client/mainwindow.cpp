#include "mainwindow.h"
#include "ui_mainwindow.h"


const QColor WHITE = QColor("white");
const QColor GREEN = QColor("darkgreen");
const QColor YELLOW = QColor("darkyellow");
const QColor RED = QColor("darkred");
const QColor GRAY = QColor("darkgray");

const int ROW_COUNT = 10;
const int COLUMN_COUNT = 10;


MainWindow::MainWindow(QWidget *parent, QString gameId, QString userId)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->gameId = gameId;
    this->userId = userId;
    this->closeEventIsAccepted = false;

    showMessage("Ваше айди: " + this->userId, QMessageBox::Icon::Information);

    ui->setupUi(this);

    ui->gameIdLabel->setText("ID игры: " + gameId);
    ui->userIdLabel->setText("Ваш ID: " + userId);

    connect(ui->fireButton, &QPushButton::released, this, &MainWindow::shoot);

    ui->fireButton->hide();
    ui->opponentField->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    timerForUserTurn = new QTimer();
    timerForGameStart = new QTimer();

    timerForUserTurn->setInterval(400);
    timerForUserTurn->connect(timerForUserTurn, &QTimer::timeout, this, &MainWindow::waitForTurn);

    timerForGameStart->setInterval(400);
    timerForGameStart->connect(timerForGameStart, &QTimer::timeout, this, &MainWindow::waitForGameStart);

    m_manager = new QNetworkAccessManager(this);

    this->setTable();
    this->setOpponentTable();

    this->getShipsAmountResponse();
}

////////////////////////////////////////Ожидание конца хода противника////////////////////////////////////////
void MainWindow::getUserIdTurn(QNetworkReply *reply)
{
    QString replyStr = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    if (this->getErrorMessage(jsonObj))
    {
        return;
    }

    if (jsonObj["user_id_turn"].toString() == this->userId)
    {
        // Отключаем таймер и событие
        timerForUserTurn->stop();
        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getUserIdTurn(QNetworkReply* )));
        // Получаем клетки, по которым попал соперник
        this->getDamagedCells();
    }
}

void MainWindow::waitForTurn()
{
    QUrl url("http://127.0.0.1:8000/games/get_user_id_turn/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("game_id", this->gameId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getUserIdTurn(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////Заполняем UI повреждёнными клетками//////////////////////////////////////////
void MainWindow::fillField(QNetworkReply *reply)
{
    QString replyStr = reply->readAll();
    qDebug() << "GET FILL FIELD";
    qDebug() << replyStr;

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    /*
     * Костыль, т.к. непонятно почему иногда принимает нужный ответ, а иногда ответ от getUserIdTurn
    */

    // Если ответ пришёл от нужного url, то заполняем данные
    // Иначе послыаем ещё один запрос
    if (!(jsonObj.contains("dead_parts")))
    {
        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(fillField(QNetworkReply* )));
        return this->getDamagedCells();
    }

    // Закрашиваем повреждённые клетки
    QJsonArray marked_cells = jsonObj["marked_cells"].toArray();
    qDebug() << marked_cells;
    if (!marked_cells.isEmpty())
    {
        for (int i = 0; i < marked_cells.size(); ++i)
        {
            QStringList currentCell = marked_cells[i].toString().split(" ");

            int x = currentCell[0].toInt();
            int y = currentCell[1].toInt();

            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setBackground(GRAY);
            ui->yourField->setItem(y, x, item);
        }
    }

    // Закрашиваем мёртвые корабли
    QJsonArray deadParts = jsonObj["dead_parts"].toArray();
    qDebug() << deadParts;
    if (!deadParts.isEmpty())
    {
        for (int i = 0; i < deadParts.size(); ++i)
        {
            QStringList currentCell = deadParts[i].toString().split(" ");

            int x = currentCell[0].toInt();
            int y = currentCell[1].toInt();

            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setBackground(RED);
            ui->yourField->setItem(y, x, item);
        }
    }

    // Закрашиваем повреждённые корабли
    QJsonArray damagedParts = jsonObj["damaged_parts"].toArray();
    qDebug() << damagedParts;
    if (!damagedParts.isEmpty())
    {
        for (int i = 0; i < damagedParts.size(); ++i)
        {
            QStringList currentCell = damagedParts[i].toString().split(" ");

            int x = currentCell[0].toInt();
            int y = currentCell[1].toInt();

            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setBackground(YELLOW);
            ui->yourField->setItem(y, x, item);
        }
    }

    // Отписываемся от события
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(fillField(QNetworkReply* )));

    // Проверяем, игра окончена или нет
    this->checkForWinner();
    // "Включаем интерфейс"
    this->setDisabled(false);
}

void MainWindow::getDamagedCells()
{
    QUrl url("http://127.0.0.1:8000/fields/get_damaged_cells/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("user_id", this->userId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(fillField(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::getGameOverState(QNetworkReply *reply)
{
    QString replyStr = reply->readAll();
    qDebug() << "GET GAME OVER STATE";
    qDebug() << replyStr;

    QJsonObject jsonObject = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    if (jsonObject["game_is_over"].toBool())
    {
        qDebug() << "WINNER OF THE GAME";
        qDebug() << "YOUR ID " << this->userId;
        qDebug() << "WINNER ID " << jsonObject["winner"].toString();

        if (jsonObject["winner"].toString() == this->userId)
        {
            showMessage("Игра закончена и победу одержали Вы", QMessageBox::Icon::Information);
        }
        else
        {
            showMessage("Игра закончена и Вы проиграли", QMessageBox::Icon::Information);
        }
        this->close();
    }

    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getGameOverState(QNetworkReply* )));
}

void MainWindow::checkForWinner()
{
    QUrl url("http://127.0.0.1:8000/games/game_is_over/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("game_id", this->gameId);
    query.addQueryItem("user_id", this->userId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getGameOverState(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}

//////////////////////////////////////////Обработка выхода из игры//////////////////////////////////////////

void MainWindow::acceptCloseEvent(QNetworkReply *reply)
{
    this->closeEventIsAccepted = true;
    QString replyStr = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    this->getErrorMessage(jsonObj);

    this->close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Пока не нашёл альтернативы как это сделать более грамотно,
    // поэтому пока так
    if (this->closeEventIsAccepted)
        return event->accept();

    qDebug() << "closeEvent called";
    QUrl url("http://127.0.0.1:8000/games/disconnect/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("user_id", this->userId);
    query.addQueryItem("game_id", this->gameId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::acceptCloseEvent);
    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
    event->ignore();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool MainWindow::getErrorMessage(QJsonObject jsonObj)
{
    if (jsonObj.contains("Error"))
    {
        showMessage(jsonObj["Error"].toString(), QMessageBox::Icon::Critical);
        return true;
    }
    if (jsonObj.contains("Critical Error"))
    {
        timerForUserTurn->stop();
        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getUserIdTurn(QNetworkReply* )));

        showMessage(jsonObj["Critical Error"].toString(), QMessageBox::Icon::Critical);
        this->close();
        return true;
    }

    return false;
}



//////////////////////////////////////////////Ожидание начала игры//////////////////////////////////////////////
void MainWindow::getGameState(QNetworkReply* reply)
{
    QString replyStr = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    if (jsonObj["game_is_started"].toBool())
    {
        // Останавливаем прошлый таймер и отключаем сигнал
        timerForGameStart->stop();
        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getGameState(QNetworkReply* )));
        delete timerForGameStart;
        //Запускаем таймер на ожидание хода
        timerForUserTurn->start();

        ui->fireButton->show();
        ui->placeShipButton->hide();
        showMessage("Игра начата, ожидайте свой ход", QMessageBox::Icon::Information);
    }

}

void MainWindow::waitForGameStart()
{
    QUrl url("http://127.0.0.1:8000/games/game_is_started/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("game_id", this->gameId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getGameState(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////Вывод в UI сколько кораблей осталось поставить///////////////////////////////
void MainWindow::setShipsAmountLabel(QNetworkReply* reply)
{
    QString replyStr = reply->readAll();
    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    if (this->getErrorMessage(jsonObj))
        return;

    int fourDeckLeft = jsonObj["four_deck"].toInt();
    int threeDeckLeft = jsonObj["three_deck"].toInt();
    int twoDeckLeft = jsonObj["two_deck"].toInt();
    int oneDeckLeft = jsonObj["one_deck"].toInt();

    QString shipsAmountStr = "";
    shipsAmountStr = "4: " + QString::number(fourDeckLeft) + "\n" +\
                     "3: " + QString::number(threeDeckLeft) + "\n" +\
                     "2: " + QString::number(twoDeckLeft) + "\n" +\
                     "1: " + QString::number(oneDeckLeft);

    ui->shipsAmountLabel->setText(shipsAmountStr);
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* )), this, SLOT(setShipsAmountLabel(QNetworkReply* )));

    if ((fourDeckLeft + threeDeckLeft + twoDeckLeft + oneDeckLeft) == 0)
    {
        showMessage("Корабли закончились, ожидаем соперника", QMessageBox::Icon::Information);
        this->setDisabled(true);
        // Ставим таймер на функцию, в которой ожидаем когда игра будет начата

        timerForGameStart->start();
    }
}

void MainWindow::getShipsAmountResponse()
{
    QUrl url("http://127.0.0.1:8000/fields/return_field/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;

    query.addQueryItem("owner_id", this->userId);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(setShipsAmountLabel(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::setTable()
{
    ui->yourField->setRowCount(ROW_COUNT);
    ui->yourField->setColumnCount(COLUMN_COUNT);
    for (int y = 0; y < ROW_COUNT; ++y)
        for (int x = 0;x < COLUMN_COUNT; ++x) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setText(QString(" "));

            item->setBackground(WHITE);
            ui->yourField->setItem(y, x, item);
        }

    ui->yourField->resizeColumnsToContents();
    ui->yourField->resizeRowsToContents();
    ui->yourField->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
}

void MainWindow::setOpponentTable()
{
    ui->opponentField->setRowCount(ROW_COUNT);
    ui->opponentField->setColumnCount(COLUMN_COUNT);
    for (int y = 0; y < ROW_COUNT; ++y)
        for (int x = 0;x < COLUMN_COUNT; ++x) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setText(QString(" "));

            item->setBackground(WHITE);
            ui->opponentField->setItem(y, x, item);
        }

    ui->opponentField->resizeColumnsToContents();
    ui->opponentField->resizeRowsToContents();
    ui->opponentField->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//////////////////////////////////Расстановка кораблей//////////////////////////////////
void MainWindow::placeShip(QNetworkReply* reply)
{
    QString replyStr = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    // Могут возникнуть ошибки, такие как коллизии, корабль слишком длинный, широкий, или корабли закончились
    if (getErrorMessage(jsonObj))
        return;

    QJsonArray cells = jsonObj["cells"].toArray();

    for (int i = 0; i < cells.size(); ++i)
    {
        QJsonArray currentCell = cells[i].toArray();
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setBackground(GREEN);
        ui->yourField->setItem(currentCell[1].toInt(), currentCell[0].toInt(), item);
        item->setSelected(false);
    }
    QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(placeShip(QNetworkReply* )));
    this->getShipsAmountResponse();
    this->setDisabled(false);
}

void MainWindow::on_placeShipButton_clicked()
{
    if (!(ui->yourField->selectedItems().length()))
        return showMessage("Ячейки не выбраны", QMessageBox::Icon::Critical);

    this->setDisabled(true);

    QUrl url("http://127.0.0.1:8000/fields/place_ship/");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QListIterator<QTableWidgetItem *> iter(ui->yourField->selectedItems());

    // В запрос передаётся json, по примеру
    /*
     * cells : [[2,3],[4,5],[1,2]]
     * owner_id: foaishf0bhas9-f09
     * game_id: 15812571257
    */
    QString values = "";
    while (iter.hasNext())
    {
        QTableWidgetItem* currentItem = iter.next();

        QString value = QString::number(currentItem->column()) + "," + QString::number(currentItem->row());
        values += value;
        values += " ";
    }
    QByteArray json = "{ \"cells\":\"" + values.toUtf8() + \
            "\", \"owner_id\":\"" + this->userId.toUtf8() + \
            "\", \"game_id\":\"" +  this->gameId.toUtf8() + "\" }";


    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(placeShip(QNetworkReply* )));
    m_manager->post(request, json);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////Стрельба по кораблям////////////////////////////////////////////
void MainWindow::getFireStatus(QNetworkReply *reply)
{
    /*
     * Получаем клетки, по которым промахнулись
     * Получаем клетки, по которым попали
     * Получаем корабли, которые убили
     * Если убили, то проверяем на конец хода
    */
    ui->opponentField->clearSelection();
    QString replyStr = reply->readAll();

    QJsonObject jsonObj = QJsonDocument::fromJson(replyStr.toUtf8()).object();

    if (getErrorMessage(jsonObj))
        return this->setDisabled(false);

    // Не попали, закрашиваем серым
    if (jsonObj.contains("missed"))
    {
        QStringList missedCell = jsonObj["missed_cell"].toString().split(" ");

        int x = missedCell[0].toInt();
        int y = missedCell[1].toInt();

        QTableWidgetItem* item = new QTableWidgetItem("");
        item->setBackground(GRAY);

        ui->opponentField->setItem(y, x, item);

        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getFireStatus(QNetworkReply* )));
        // Ожидаем пока оппонент сделает ход
        timerForUserTurn->start();
    }

    // Закрашиваем либо жёлтым цветом
    else if (jsonObj.contains("ship_is_damaged"))
    {
        QStringList damagedPart = jsonObj["damaged_part"].toString().split(" ");

        int x = damagedPart[0].toInt();
        int y = damagedPart[1].toInt();

        QTableWidgetItem* item = new QTableWidgetItem("");
        item->setBackground(YELLOW);
        ui->opponentField->setItem(y, x, item);
        this->setDisabled(false);
    }

    // Либо весь корабль красным
    else if (jsonObj.contains("ship_is_killed"))
    {
        QJsonArray deadParts = jsonObj["dead_parts"].toArray();
        QJsonArray missedCells = jsonObj["missed_cells"].toArray();

        /* Сначала закрашиваем серым цветом, потому
         * что они почему-то перекрывают красные клетки */
        for (int i = 0; i < missedCells.size(); ++i)
        {
            QStringList currentCell = missedCells[i].toString().split(" ");

            int x = currentCell[0].toInt();
            int y = currentCell[1].toInt();

            QTableWidgetItem* item = new QTableWidgetItem("");
            item->setBackground(GRAY);
            ui->opponentField->setItem(y, x, item);
        }

        /* Затем закрашиваем мёртвые части корабля */
        for (int i = 0; i < deadParts.size(); ++i)
        {
            QStringList currentPart = deadParts[i].toString().split(" ");

            int x = currentPart[0].toInt();
            int y = currentPart[1].toInt();

            QTableWidgetItem* item = new QTableWidgetItem("");
            item->setBackground(RED);
            ui->opponentField->setItem(y, x, item);
        }



        this->setDisabled(false);

        // Отписываемся от события, т.к. иначе в checkForWinner будет передаваться информация о кораблях
        QObject::disconnect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getFireStatus(QNetworkReply* )));
        // Проверяем является ли этот корабль последним, т.е. закончена ли игра
        this->checkForWinner();
    }
}

void MainWindow::shoot()
{
    /*
     * Отправляем запрос на сервер, блокируя интерфейс
     * Если ошибок нет, то проверяем на попадание
     * Если не попал, то ожидаем своего хода
     * Если попал, то даём ещё один ход
     * Если убил, то проверяем на конец игры
    */

    if (!ui->opponentField->selectedItems().length())
        return showMessage("Клетки не были выбраны", QMessageBox::Icon::Critical);

    QTableWidgetItem* selectedCell = ui->opponentField->selectedItems()[0];

    this->setDisabled(true);

    QUrl url("http://127.0.0.1:8000/games/fire/");
    QNetworkRequest request(url);


    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery jsonData;
    jsonData.addQueryItem("user_id", this->userId);
    jsonData.addQueryItem("game_id", this->gameId);

    jsonData.addQueryItem("x", QString::number(selectedCell->column()));
    jsonData.addQueryItem("y", QString::number(selectedCell->row()));

    QUrl data;
    data.setQuery(jsonData);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT(getFireStatus(QNetworkReply* )));
    m_manager->post(request, data.toEncoded().remove(0, 1));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
