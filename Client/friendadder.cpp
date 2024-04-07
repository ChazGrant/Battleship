#include "friendadder.h"
#include "ui_friendadder.h"

/*! @brief Конструктор класса
 *
 *  @param *parent Указатель на родительский класс виджета
 *
 *  @return FriendAdder
*/
FriendAdder::FriendAdder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendAdder)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);

    connect(ui->addFriendButton, &QPushButton::clicked, this, &FriendAdder::addFriend);
    connect(ui->closeWindowButton, &QPushButton::clicked, this, &FriendAdder::close);
}

//! @brief Деструктор класса FriendAdder
FriendAdder::~FriendAdder()
{
    delete ui;
}

/*! @brief Добавление нового друга
 *
 *  @details Берёт айди друга из lineEdit и если оно является числом активирует сигнал friendAdded
 *
 *  @return void
*/
void FriendAdder::addFriend()
{
    bool ok;
    int friendId = ui->friendIdLineEdit->text().toInt(&ok);
    if (!ok)
        return showMessage("Введите число", QMessageBox::Icon::Critical);

    emit friendAdded(friendId);
    close();
}
