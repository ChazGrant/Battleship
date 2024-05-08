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

    connect(ui->addFriendButton, &QPushButton::clicked, this, &FriendAdder::addFriend);
    connect(ui->closeWindowButton, &QPushButton::clicked, this, &FriendAdder::close);

    connect(ui->hideButton, &QPushButton::clicked, this, &FriendAdder::showMinimized);
    connect(ui->closeButton, &QPushButton::clicked, this, &FriendAdder::close);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
}

//! @brief Деструктор класса FriendAdder
FriendAdder::~FriendAdder()
{
    delete ui;
}

/*! @brief Обработка перемещения мыши
 *
 *  @details Меняет переменную delta и перемещает окно к её координатам
 *
 *  @param *event Указатель на событие поведения мыши
 *
 *  @return void
*/
void FriendAdder::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF delta = event->globalPos() - m_mouse_point;
    move(delta.toPoint());

    event->accept();
}

/*! @brief Обработка нажатия кнопки мыши
 *
 *  @details При нажатии на левую, правую или среднюю кнопку мыши
 *  меняет приватную переменную m_mouse_point
 *
 *  @param *event Указатель на событие поведения мыши
 *
 *  @return void
*/
void FriendAdder::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton ||
        event->button() == Qt::RightButton ||
        event->button() == Qt::MiddleButton) {
        m_mouse_point = event->pos();
        event->accept();
    }
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
