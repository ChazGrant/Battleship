#include "friendadder.h"
#include "ui_friendadder.h"

FriendAdder::FriendAdder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendAdder)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);

    connect(ui->addFriendButton, &QPushButton::clicked, this, &FriendAdder::addFriend);
    connect(ui->closeWindowButton, &QPushButton::clicked, this, &FriendAdder::close);
}

FriendAdder::~FriendAdder()
{
    delete ui;
}

void FriendAdder::addFriend()
{
    bool ok;
    int friendId = ui->friendIdLineEdit->text().toInt(&ok);
    if (!ok)
        return showMessage("Введите число", QMessageBox::Icon::Critical);

    emit friendAdded(friendId);
    close();
}
