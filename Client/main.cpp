#include "mainwindow.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "loginform.h"
#include "shop.h"
#include "friendadder.h"
#include "gameinvitenotifier.h"

#include <QApplication>

#include <iostream>
#include <map>
#include <string>


void printSomething() {
    std::cout << "Something";
}

int main(int argc, char *argv[])
{
    QMap<QString, std::function<void()>> functionCallMap;

    functionCallMap["print"] = [] () { printSomething(); };
    functionCallMap["print"]();

    return 0;
    QApplication a(argc, argv);
    LoginForm form;
    form.show();
    return a.exec();
}
