#include "gameinvitenotifier.h"
#include "ui_gameinvitenotifier.h"


/*! @brief Конструктор класса
 *
 *  @param *parent Указатель на родительский класс виджета
 *
 *  @return GameInviteNotifier
*/
GameInviteNotifier::GameInviteNotifier(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameInviteNotifier)
{
    ui->setupUi(this);

    connect(ui->hideButton, &QPushButton::clicked, this, &GameInviteNotifier::showMinimized);
    connect(ui->closeButton, &QPushButton::clicked, this, &GameInviteNotifier::close);

    connect(ui->acceptGameInviteButton, &QPushButton::clicked, this, [=] {
        emit gameInviteAccepted();
        close();
    });
    connect(ui->declineGameInviteButton, &QPushButton::clicked, this, [=] {
        emit gameInviteDeclined();
        close();
    });
}

//! @brief Деструктор класса
GameInviteNotifier::~GameInviteNotifier()
{
    delete ui;
}
