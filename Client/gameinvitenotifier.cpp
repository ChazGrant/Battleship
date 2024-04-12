#include "gameinvitenotifier.h"
#include "ui_gameinvitenotifier.h"

GameInviteNotifier::GameInviteNotifier(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameInviteNotifier)
{
    ui->setupUi(this);

    connect(ui->acceptGameInviteButton, &QPushButton::clicked, this, [=] {
        emit gameInviteAccepted();
        close();
    });
    connect(ui->declineGameInviteButton, &QPushButton::clicked, this, [=] {
        emit gameInviteDeclined();
        close();
    });
}

GameInviteNotifier::~GameInviteNotifier()
{
    delete ui;
}
