#ifndef FRIENDADDER_H
#define FRIENDADDER_H

#include <QWidget>
#include <QMessageBox>

#include <QMouseEvent>

#include "additionalfunctions.h"

namespace Ui {
class FriendAdder;
}

class FriendAdder : public QWidget
{
    Q_OBJECT

public:
    explicit FriendAdder(QWidget *parent = nullptr);
    ~FriendAdder();

signals:
    //! @brief Сигнал срабатываемый когда нажимается кнопка добавить друга
    void friendAdded(int);

private:
    //! @brief Указатель на интерфейс класса
    Ui::FriendAdder *ui;

    //! Точка на экране
    QPoint m_mouse_point;
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);

    void addFriend();
};

#endif // FRIENDADDER_H
