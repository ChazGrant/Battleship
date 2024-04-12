#ifndef GAMEINVITENOTIFIER_H
#define GAMEINVITENOTIFIER_H

#include <QWidget>

namespace Ui {
class GameInviteNotifier;
}

class GameInviteNotifier : public QWidget
{
    Q_OBJECT

public:
    explicit GameInviteNotifier(QWidget *parent = nullptr);
    ~GameInviteNotifier();

signals:
    void gameInviteAccepted();
    void gameInviteDeclined();

private:
    Ui::GameInviteNotifier *ui;
};

#endif // GAMEINVITENOTIFIER_H
