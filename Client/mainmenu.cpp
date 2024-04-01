#include "mainmenu.h"
#include "ui_mainmenu.h"

#include "inputgameid.h"
#include "mainwindow.h"


/**
 *  @brief Конструктор класса
 *
 *  @details Инициализирует свойство класса m_manager
 *
 *  @param *parent Указатель на родительский виджет
 *
 *  @return MainMenu
*/
MainMenu::MainMenu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainMenu)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager(this);
}

//! @brief Деструктор класса
MainMenu::~MainMenu()
{
    delete ui;
}

/*! @brief Метод для обработки нажатия на кнопку "Подключиться к игре"
 *
 *  @details Делает запрос к адресу по подключению к игре.
 *  В параметры передаёт идентификатор пользователя
 *  после ответа от сервера передаёт управление методу connectToCreatedGame
 *
 *  @return void
*/
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

/*! @brief Подключение к указанной игре
 *
 *  @details Получает идентификатор игры из ответа сервера, выдаёт ошибку
 *  или открывает окно с игрой, передавая айди игры
 *
 *  @param *reply Указатель  на ответ от сервера
 *
 *  @return void
*/
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

/*!
 * @brief Обработчик нажатия кнопки "Подключиться"
 *
 * @details Открывает окно для ввода идентификатора игры
 *
 * @return void
*/
void MainMenu::on_connectToExistingGame_clicked()
{
    InputGameID *widget = new InputGameID(nullptr, this->m_userId);
    widget->show();

    QObject::connect(widget, &InputGameID::acceptConnection, this, &MainMenu::openMainWindow);
}

/*!
 * @brief Открытие окна с игрой
 *
 * @param gameId Идентификатор игры
 *
 * @return void
*/
void MainMenu::openMainWindow(QString gameId)
{
    QString userId = ui->userID->text().split(": ")[1];
    this->m_window = new MainWindow (nullptr, gameId, userId);

    m_window->show();
    this->close();
}
