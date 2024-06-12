#include "mainwindow.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "loginform.h"
#include "shop.h"
#include "friendadder.h"
#include "gameinvitenotifier.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginForm form;
    form.show();
    return a.exec();
}
