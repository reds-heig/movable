
#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QColor>
#include <QGraphicsItem>

#include "model_blood_image.h"

class ImageItem : public QGraphicsItem
{
public:
    ImageItem(BloodImage *data);

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) Q_DECL_OVERRIDE;

    static bool parasite_pen_selected;
    static bool eraser_selected;

    void setParasitePen(bool value);
    void setEraserPen(bool value);
    void saveImage();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

private:

    BloodImage *data;
    QPixmap pixmap_cache;
    QVector<QPointF> stuff;
};

#endif
