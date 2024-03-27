#include "mainwindow.h"
#include "mainmenu.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainMenu form;
    form.show();
    return a.exec();
}
