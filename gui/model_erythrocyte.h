#ifndef ERYTHROCYTE_H
#define ERYTHROCYTE_H

#include <QGraphicsItem>

class Erythrocyte
{
    public:
        Erythrocyte(int id, QPointF center, qreal radius);

        int getId();
        QPointF getCenter();
        QPointF getPos();
        qreal getRadius();

    private:

        int id;
        QPointF center;
        qreal radius;
};

#endif
