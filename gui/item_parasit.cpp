#include "item_parasit.h"

#include <QtWidgets>

#include "movable_ui.h"

ParasiteItem::ParasiteItem(Parasit* p)
{
    this->parasit = p;

    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
}

Parasit* ParasiteItem::getParasit()
{
    return parasit;
}

QRectF ParasiteItem::boundingRect() const
{
    return QRectF(0, 0, this->parasit->getSize(), this->parasit->getSize());
}

QPainterPath ParasiteItem::shape() const
{
    QPainterPath path;
    path.addRect(0, 0, this->parasit->getSize(), this->parasit->getSize());
    return path;
}

void ParasiteItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    QColor color(255,this->parasit->getGT(),0,80);

    QColor fillColor = (option->state & QStyle::State_Selected) ? color.dark(300) : color;
    if (option->state & QStyle::State_MouseOver)
        fillColor = fillColor.light(200);

    QBrush b = painter->brush();
    painter->setBrush(b);

    painter->setBrush(QBrush(fillColor.dark(option->state & QStyle::State_Sunken ? 120 : 100)));

    painter->drawRect(QRect(0, 0, this->parasit->getSize(), this->parasit->getSize()));

    QFont font;
    font.setPixelSize(7);

    painter->setFont(font);
    painter->drawText(1,11,QString("parasite_") + QString::number(this->parasit->getId()));
}

void ParasiteItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();

    emit parasiteSelected(parasit->getId());
}

