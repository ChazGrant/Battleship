#include "mainwindow.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "loginform.h"
#include "shop.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // MainWindow form("410932554967672480492357", 1, "c20ad4d76fe97759aa27a0c99bff6710");
    LoginForm form;
    // Shop form(1);
    form.show();
    return a.exec();
}
