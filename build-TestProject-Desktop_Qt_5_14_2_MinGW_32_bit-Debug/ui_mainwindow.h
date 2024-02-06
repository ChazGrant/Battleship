/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QLabel *label;
    QTableWidget *yourField;
    QLabel *shipsAmountLabel;
    QGroupBox *skillsBox;
    QPushButton *placePlaneButton;
    QPushButton *launchPlaneButton;
    QPushButton *placeBombButton;
    QPushButton *detonateBombButton;
    QLabel *gameIdLabel;
    QTableWidget *opponentField;
    QPushButton *placeShipButton;
    QPushButton *fireButton;
    QLabel *userIdLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 631);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 320, 111, 31));
        yourField = new QTableWidget(centralwidget);
        yourField->setObjectName(QString::fromUtf8("yourField"));
        yourField->setGeometry(QRect(10, 50, 341, 261));
        shipsAmountLabel = new QLabel(centralwidget);
        shipsAmountLabel->setObjectName(QString::fromUtf8("shipsAmountLabel"));
        shipsAmountLabel->setGeometry(QRect(20, 350, 111, 131));
        shipsAmountLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        skillsBox = new QGroupBox(centralwidget);
        skillsBox->setObjectName(QString::fromUtf8("skillsBox"));
        skillsBox->setGeometry(QRect(540, 350, 191, 161));
        placePlaneButton = new QPushButton(skillsBox);
        placePlaneButton->setObjectName(QString::fromUtf8("placePlaneButton"));
        placePlaneButton->setGeometry(QRect(10, 30, 171, 31));
        launchPlaneButton = new QPushButton(skillsBox);
        launchPlaneButton->setObjectName(QString::fromUtf8("launchPlaneButton"));
        launchPlaneButton->setGeometry(QRect(10, 60, 171, 31));
        placeBombButton = new QPushButton(skillsBox);
        placeBombButton->setObjectName(QString::fromUtf8("placeBombButton"));
        placeBombButton->setGeometry(QRect(10, 90, 171, 31));
        detonateBombButton = new QPushButton(skillsBox);
        detonateBombButton->setObjectName(QString::fromUtf8("detonateBombButton"));
        detonateBombButton->setGeometry(QRect(10, 120, 171, 31));
        gameIdLabel = new QLabel(centralwidget);
        gameIdLabel->setObjectName(QString::fromUtf8("gameIdLabel"));
        gameIdLabel->setGeometry(QRect(20, 10, 331, 16));
        opponentField = new QTableWidget(centralwidget);
        opponentField->setObjectName(QString::fromUtf8("opponentField"));
        opponentField->setGeometry(QRect(440, 50, 341, 261));
        placeShipButton = new QPushButton(centralwidget);
        placeShipButton->setObjectName(QString::fromUtf8("placeShipButton"));
        placeShipButton->setGeometry(QRect(150, 320, 141, 31));
        fireButton = new QPushButton(centralwidget);
        fireButton->setObjectName(QString::fromUtf8("fireButton"));
        fireButton->setGeometry(QRect(150, 320, 141, 31));
        userIdLabel = new QLabel(centralwidget);
        userIdLabel->setObjectName(QString::fromUtf8("userIdLabel"));
        userIdLabel->setGeometry(QRect(410, 20, 331, 16));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 20));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\320\232\320\276\320\273\320\270\321\207\320\265\321\201\321\202\320\262\320\276 \320\272\320\276\321\200\320\260\320\261\320\273\320\265\320\271: ", nullptr));
        shipsAmountLabel->setText(QString());
        skillsBox->setTitle(QCoreApplication::translate("MainWindow", "\320\241\320\277\320\276\321\201\320\276\320\261\320\275\320\276\321\201\321\202\320\270", nullptr));
        placePlaneButton->setText(QCoreApplication::translate("MainWindow", "\320\241\320\260\320\274\320\276\320\273\321\221\321\202", nullptr));
        launchPlaneButton->setText(QCoreApplication::translate("MainWindow", "\320\227\320\260\320\277\321\203\321\201\321\202\320\270\321\202\321\214 \321\201\320\260\320\274\320\276\320\273\321\221\321\202", nullptr));
        placeBombButton->setText(QCoreApplication::translate("MainWindow", "\320\221\320\276\320\274\320\261\320\260", nullptr));
        detonateBombButton->setText(QCoreApplication::translate("MainWindow", "\320\222\320\267\320\276\321\200\320\262\320\260\321\202\321\214", nullptr));
        gameIdLabel->setText(QCoreApplication::translate("MainWindow", "ID \320\270\320\263\321\200\321\213: ", nullptr));
        placeShipButton->setText(QCoreApplication::translate("MainWindow", "\320\237\320\276\321\201\321\202\320\260\320\262\320\270\321\202\321\214 \320\272\320\276\321\200\320\260\320\261\320\273\321\214", nullptr));
        fireButton->setText(QCoreApplication::translate("MainWindow", "\320\241\321\202\321\200\320\265\320\273\321\217\321\202\321\214", nullptr));
        userIdLabel->setText(QCoreApplication::translate("MainWindow", "\320\222\320\260\321\210\320\265 ID: ", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
