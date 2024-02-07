#include "mainmenu.h"
#include "ui_mainmenu.h"

#include "inputgameid.h"
#include "mainwindow.h"

MainMenu::MainMenu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainMenu)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager(this);

    userId = generateUserId(15);
    ui->userID->setText("Ваш ID: " + userId);
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::on_createNewGameButton_clicked()
{
    QUrl url("http://127.0.0.1:8000/games/create_game/");
    QNetworkRequest request( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery query;
    QString user_id = ui->userID->text().split(": ")[1];

    query.addQueryItem("user_id", user_id);

    QUrl queryUrl;
    queryUrl.setQuery(query);

    QObject::connect(m_manager, SIGNAL( finished( QNetworkReply* ) ), SLOT(connectToCreatedGame(QNetworkReply* )));

    m_manager->post(request, queryUrl.toEncoded().remove(0, 1));
}

void MainMenu::connectToCreatedGame(QNetworkReply *reply)
{
    // Читаем ответ от сервера в строку
    QString strReply = reply->readAll();
    // Конвертируем строку в json object
    QJsonObject jsonResponse = QJsonDocument::fromJson(strReply.toUtf8()).object();


    // Если ответ содержит id игры, значит она создана и ошибок нет
    if (jsonResponse.contains("game_id"))
    {
        QString gameId = jsonResponse["game_id"].toString();
        QMessageBox msgBox;
        msgBox.setText("ID вашей игры:\n" + gameId);
        msgBox.exec();
        this->openMainWindow(gameId);
    } else if (jsonResponse.contains("Error")) {
        QString error = jsonResponse["Error"].toString();
        QMessageBox msgBox;
        msgBox.setText("Возникли ошибки:\n" + error);
        msgBox.exec();
    }
}

void MainMenu::on_connectToExistingGame_clicked()
{
    InputGameID *widget = new InputGameID(nullptr, this->userId);
    widget->show();

    QObject::connect(widget, &InputGameID::acceptConnection, this, &MainMenu::openMainWindow);
}

void MainMenu::openMainWindow(QString gameId)
{
    QString userId = ui->userID->text().split(": ")[1];
    this->window = new MainWindow (nullptr, gameId, userId);

    window->show();
    this->close();
}
