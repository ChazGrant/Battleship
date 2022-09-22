#ifndef SHIP_H
#define SHIP_H

#include <QList>
#include <QDebug>

class Ship
{
public:
    Ship(int shipLength);

    QList<QList<int>> getPartsPositions();
    // Устанавливает позиции всех частей корабля
    void setPosition(QList<int> head, QList<int> tail);
    // Если в позиции есть часть корабля, то получает урон
    bool takeDamage(QList<int> position);
    // Возвращает true если все части корабля повреждены
    bool isDead();

private:
    int shipLength;
    int partsDamaged;

    // Части корабля
    QList<QList<int>> shipPartsPosition;
    // Начало и конец корабля
    QList<int> head;
    QList<int> tail;
};

#endif // SHIP_H
