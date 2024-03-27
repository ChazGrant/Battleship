#include "additionalfunctions.h"


inline void showMessage(QString t_message_text, QMessageBox::Icon t_message_icon) {
    QMessageBox msgbox;
    msgbox.setWindowTitle("Ошибка");
    msgbox.setText(t_message_text);
    msgbox.exec();
}
