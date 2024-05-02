#include "mainwindow.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "loginform.h"
#include "shop.h"

#include <QApplication>


inline void swap(QJsonValueRef t_firstItem, QJsonValueRef t_secondItem) {
    QJsonValue temp(t_firstItem);
    t_secondItem = t_firstItem;
    t_firstItem = temp;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // MainWindow form("410932554967672480492357", 1, "c20ad4d76fe97759aa27a0c99bff6710");
    // LoginForm form;
    // Shop form(1);
    MainMenu form(1, "user");
    form.show();
    return a.exec();
}
