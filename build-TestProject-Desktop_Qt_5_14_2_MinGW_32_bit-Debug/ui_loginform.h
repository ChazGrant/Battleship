/********************************************************************************
** Form generated from reading UI file 'loginform.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINFORM_H
#define UI_LOGINFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginForm
{
public:
    QTabWidget *authenticate_tabWidget;
    QWidget *tab;
    QLineEdit *loginUserID_lineEdit;
    QLineEdit *loginPassword_lineEdit;
    QPushButton *login_pushButton;
    QWidget *tab_2;
    QPushButton *registrate_pushButton;
    QLineEdit *registratePassword_lineEdit;
    QLineEdit *registrateUsername_lineEdit;
    QLineEdit *registrateEmail_lineEdit;

    void setupUi(QWidget *LoginForm)
    {
        if (LoginForm->objectName().isEmpty())
            LoginForm->setObjectName(QString::fromUtf8("LoginForm"));
        LoginForm->resize(219, 175);
        authenticate_tabWidget = new QTabWidget(LoginForm);
        authenticate_tabWidget->setObjectName(QString::fromUtf8("authenticate_tabWidget"));
        authenticate_tabWidget->setGeometry(QRect(0, 0, 221, 181));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        loginUserID_lineEdit = new QLineEdit(tab);
        loginUserID_lineEdit->setObjectName(QString::fromUtf8("loginUserID_lineEdit"));
        loginUserID_lineEdit->setGeometry(QRect(50, 10, 113, 24));
        loginUserID_lineEdit->setEchoMode(QLineEdit::Normal);
        loginPassword_lineEdit = new QLineEdit(tab);
        loginPassword_lineEdit->setObjectName(QString::fromUtf8("loginPassword_lineEdit"));
        loginPassword_lineEdit->setGeometry(QRect(50, 50, 113, 24));
        loginPassword_lineEdit->setEchoMode(QLineEdit::Password);
        login_pushButton = new QPushButton(tab);
        login_pushButton->setObjectName(QString::fromUtf8("login_pushButton"));
        login_pushButton->setGeometry(QRect(50, 100, 111, 24));
        authenticate_tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        registrate_pushButton = new QPushButton(tab_2);
        registrate_pushButton->setObjectName(QString::fromUtf8("registrate_pushButton"));
        registrate_pushButton->setGeometry(QRect(40, 100, 131, 24));
        registratePassword_lineEdit = new QLineEdit(tab_2);
        registratePassword_lineEdit->setObjectName(QString::fromUtf8("registratePassword_lineEdit"));
        registratePassword_lineEdit->setGeometry(QRect(40, 40, 131, 24));
        registrateUsername_lineEdit = new QLineEdit(tab_2);
        registrateUsername_lineEdit->setObjectName(QString::fromUtf8("registrateUsername_lineEdit"));
        registrateUsername_lineEdit->setGeometry(QRect(40, 10, 131, 24));
        registrateEmail_lineEdit = new QLineEdit(tab_2);
        registrateEmail_lineEdit->setObjectName(QString::fromUtf8("registrateEmail_lineEdit"));
        registrateEmail_lineEdit->setGeometry(QRect(40, 70, 131, 24));
        authenticate_tabWidget->addTab(tab_2, QString());

        retranslateUi(LoginForm);

        authenticate_tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(LoginForm);
    } // setupUi

    void retranslateUi(QWidget *LoginForm)
    {
        LoginForm->setWindowTitle(QCoreApplication::translate("LoginForm", "Form", nullptr));
        loginUserID_lineEdit->setPlaceholderText(QCoreApplication::translate("LoginForm", "\320\222\320\262\320\265\320\264\320\270\321\202\320\265 ID", nullptr));
#if QT_CONFIG(tooltip)
        loginPassword_lineEdit->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        loginPassword_lineEdit->setWhatsThis(QString());
#endif // QT_CONFIG(whatsthis)
        loginPassword_lineEdit->setText(QString());
        loginPassword_lineEdit->setPlaceholderText(QCoreApplication::translate("LoginForm", "\320\222\320\262\320\265\320\264\320\270\321\202\320\265 \320\277\320\260\321\200\320\276\320\273\321\214", nullptr));
        login_pushButton->setText(QCoreApplication::translate("LoginForm", "\320\222\320\276\320\271\321\202\320\270", nullptr));
        authenticate_tabWidget->setTabText(authenticate_tabWidget->indexOf(tab), QCoreApplication::translate("LoginForm", "\320\222\321\205\320\276\320\264", nullptr));
        registrate_pushButton->setText(QCoreApplication::translate("LoginForm", "\320\227\320\260\321\200\320\265\320\263\320\270\321\201\321\202\321\200\320\270\321\200\320\276\320\262\320\260\321\202\321\214\321\201\321\217", nullptr));
        registratePassword_lineEdit->setPlaceholderText(QCoreApplication::translate("LoginForm", "\320\237\321\200\320\270\320\264\321\203\320\274\320\260\320\271\321\202\320\265 \320\277\320\260\321\200\320\276\320\273\321\214", nullptr));
        registrateUsername_lineEdit->setPlaceholderText(QCoreApplication::translate("LoginForm", "\320\237\321\200\320\270\320\264\321\203\320\274\320\260\320\271\321\202\320\265 \320\270\320\274\321\217", nullptr));
        registrateEmail_lineEdit->setPlaceholderText(QCoreApplication::translate("LoginForm", "\320\222\320\262\320\265\320\264\320\270\321\202\320\265 \320\222\320\260\321\210\321\203 \320\277\320\276\321\207\321\202\321\203", nullptr));
        authenticate_tabWidget->setTabText(authenticate_tabWidget->indexOf(tab_2), QCoreApplication::translate("LoginForm", "\320\240\320\265\320\263\320\270\321\201\321\202\321\200\320\260\321\206\320\270\321\217", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginForm: public Ui_LoginForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINFORM_H
