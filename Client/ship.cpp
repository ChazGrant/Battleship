#include "ship.h"

Ship::Ship(int shipLength)
{
    this->shipLength = shipLength;
    this->partsDamaged = 0;
}

QList<QList<int> > Ship::getPartsPositions()
{
    return this->shipPartsPosition;
}

void Ship::setPosition(QList<int> head, QList<int> tail)
{
    for (int x = head[0]; x <= tail[0]; ++x)
        for (int y = head[1]; y <= tail[1]; ++y)
            this->shipPartsPosition.append(QList<int>{x ,y});
}

bool Ship::isDead()
{
    return this->partsDamaged == this->shipPartsPosition.length();
}

bool Ship::takeDamage(QList<int> position)
{
    qDebug() << this->partsDamaged;
    if (this->isDead())
        return false;

    if (this->shipPartsPosition.contains(position))
    {
        this->partsDamaged++;
        return true;
    }
    return false;
}
