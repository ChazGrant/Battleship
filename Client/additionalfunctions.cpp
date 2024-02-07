#include "additionalfunctions.h"


inline void showMessage(QString t_message_text, QMessageBox::Icon t_message_icon) {
    QMessageBox msgbox;
    msgbox.setWindowTitle("Ошибка");
    msgbox.setText(t_message_text);
    msgbox.exec();
}

inline QString generateUserId(int ch) {
    srand(time(NULL));
    char alpha[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                      'h', 'i', 'j', 'k', 'l', 'm', 'n',
                      'o', 'p', 'q', 'r', 's', 't', 'u',
                      'v', 'w', 'x', 'y', 'z' };
    QString result = "";
    for (int i = 0; i<ch; i++)
        result = result + alpha[rand() % 26];

    return result;
}
