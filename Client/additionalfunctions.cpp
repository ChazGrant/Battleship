#include "additionalfunctions.h"


/*! @brief Выводит сообщение на экран
 *
 *  @param t_message_text Текст сообщения
 *  @param t_message_icon Иконка для окна сообщения
 *
 *  @return void
*/
inline void showMessage(QString t_message_text, QMessageBox::Icon t_message_icon) {
    QMessageBox msgbox;
    msgbox.setWindowTitle("Ошибка");
    msgbox.setText(t_message_text);
    msgbox.exec();
}
