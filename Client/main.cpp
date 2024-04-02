#include "mainwindow.h"
#include "mainmenu.h"
#include "loginform.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainMenu form(1);
    form.show();
    return a.exec();
}
