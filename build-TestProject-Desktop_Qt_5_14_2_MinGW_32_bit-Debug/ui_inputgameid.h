/********************************************************************************
** Form generated from reading UI file 'inputgameid.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INPUTGAMEID_H
#define UI_INPUTGAMEID_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_InputGameID
{
public:
    QLabel *label;
    QTextEdit *gameIDTextEdit;
    QPushButton *connectToGameButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *InputGameID)
    {
        if (InputGameID->objectName().isEmpty())
            InputGameID->setObjectName(QString::fromUtf8("InputGameID"));
        InputGameID->resize(392, 87);
        label = new QLabel(InputGameID);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 20, 47, 13));
        gameIDTextEdit = new QTextEdit(InputGameID);
        gameIDTextEdit->setObjectName(QString::fromUtf8("gameIDTextEdit"));
        gameIDTextEdit->setGeometry(QRect(60, 10, 321, 31));
        connectToGameButton = new QPushButton(InputGameID);
        connectToGameButton->setObjectName(QString::fromUtf8("connectToGameButton"));
        connectToGameButton->setGeometry(QRect(10, 50, 91, 31));
        cancelButton = new QPushButton(InputGameID);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(290, 50, 91, 31));

        retranslateUi(InputGameID);

        QMetaObject::connectSlotsByName(InputGameID);
    } // setupUi

    void retranslateUi(QDialog *InputGameID)
    {
        InputGameID->setWindowTitle(QCoreApplication::translate("InputGameID", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("InputGameID", "ID \320\270\320\263\321\200\321\213:", nullptr));
        connectToGameButton->setText(QCoreApplication::translate("InputGameID", "\320\237\320\276\320\264\320\272\320\273\321\216\321\207\320\270\321\202\321\214\321\201\321\217", nullptr));
        cancelButton->setText(QCoreApplication::translate("InputGameID", "\320\236\321\202\320\274\320\265\320\275\320\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InputGameID: public Ui_InputGameID {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INPUTGAMEID_H
