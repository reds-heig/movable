#ifndef ERYTHROCYTEITEM_H
#define ERYTHROCYTEITEM_H

#include "model_erythrocyte.h"

#include <QColor>
#include <QGraphicsEllipseItem>

class ErythrocyteItem : public QGraphicsEllipseItem
{
public:
    ErythrocyteItem (Erythrocyte* e);

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) Q_DECL_OVERRIDE;

private:
    Erythrocyte *erythrocyte;
};

#endif
