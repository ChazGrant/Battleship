/********************************************************************************
** Form generated from reading UI file 'mainmenu.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINMENU_H
#define UI_MAINMENU_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainMenu
{
public:
    QWidget *centralwidget;
    QPushButton *connectToExistingGame;
    QPushButton *createNewGameButton;
    QLabel *userID;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainMenu)
    {
        if (MainMenu->objectName().isEmpty())
            MainMenu->setObjectName(QString::fromUtf8("MainMenu"));
        MainMenu->resize(170, 172);
        centralwidget = new QWidget(MainMenu);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        connectToExistingGame = new QPushButton(centralwidget);
        connectToExistingGame->setObjectName(QString::fromUtf8("connectToExistingGame"));
        connectToExistingGame->setGeometry(QRect(10, 40, 151, 41));
        createNewGameButton = new QPushButton(centralwidget);
        createNewGameButton->setObjectName(QString::fromUtf8("createNewGameButton"));
        createNewGameButton->setGeometry(QRect(10, 90, 151, 41));
        userID = new QLabel(centralwidget);
        userID->setObjectName(QString::fromUtf8("userID"));
        userID->setGeometry(QRect(10, 10, 141, 16));
        MainMenu->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainMenu);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 170, 20));
        MainMenu->setMenuBar(menubar);
        statusbar = new QStatusBar(MainMenu);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainMenu->setStatusBar(statusbar);

        retranslateUi(MainMenu);

        QMetaObject::connectSlotsByName(MainMenu);
    } // setupUi

    void retranslateUi(QMainWindow *MainMenu)
    {
        MainMenu->setWindowTitle(QCoreApplication::translate("MainMenu", "MainWindow", nullptr));
        connectToExistingGame->setText(QCoreApplication::translate("MainMenu", "\320\237\321\200\320\270\321\201\320\276\320\265\320\264\320\270\320\275\320\270\321\202\321\214\321\201\321\217", nullptr));
        createNewGameButton->setText(QCoreApplication::translate("MainMenu", "\320\241\320\276\320\267\320\264\320\260\321\202\321\214 \320\275\320\276\320\262\321\203\321\216 \320\270\320\263\321\200\321\203", nullptr));
        userID->setText(QCoreApplication::translate("MainMenu", "\320\222\320\260\321\210 ID: 2281337", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainMenu: public Ui_MainMenu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINMENU_H
