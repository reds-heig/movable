#include "item_erythrocyte.h"

#include <QtWidgets>

ErythrocyteItem::ErythrocyteItem(Erythrocyte* e)
{
    this->erythrocyte = e;
    setAcceptHoverEvents(true);
}

QRectF ErythrocyteItem::boundingRect() const
{
    return QRectF(0, 0, this->erythrocyte->getRadius()*2, this->erythrocyte->getRadius()*2);
}

QPainterPath ErythrocyteItem::shape() const
{
    QPainterPath path;
    path.addRect(QRectF(0, 0, this->erythrocyte->getRadius()*2, this->erythrocyte->getRadius()*2));
    return path;
}

void ErythrocyteItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(Qt::NoBrush);

    QPen p = painter->pen();
    p.setWidth(2);
    p.setColor(Qt::red);
    p.setStyle(Qt::SolidLine);
    painter->setPen(p);

    painter->drawEllipse(QRect(0, 0, this->erythrocyte->getRadius()*2, this->erythrocyte->getRadius()*2));
}
