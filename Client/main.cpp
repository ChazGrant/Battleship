#include "mainwindow.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "loginform.h"
#include "shop.h"
#include "friendadder.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // MainWindow form("410932554967672480492357", 1, "c20ad4d76fe97759aa27a0c99bff6710");
    // LoginForm form;
    MainMenu form(1, "user");
    // FriendAdder form;
    form.show();
    // form.show();
    return a.exec();
}
