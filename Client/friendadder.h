#ifndef FRIENDADDER_H
#define FRIENDADDER_H

#include <QWidget>
#include <QMessageBox>

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
    void friendAdded(int);

private:
    Ui::FriendAdder *ui;

    void addFriend();
};

#endif // FRIENDADDER_H
