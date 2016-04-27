#include "item_image.h"

#include <QtWidgets>

bool ImageItem::parasite_pen_selected = true;
bool ImageItem::eraser_selected = false;

ImageItem::ImageItem(BloodImage *data)
{
    setFlags(ItemIsSelectable);
    //setAcceptHoverEvents(true);

    parasite_pen_selected = true;
    eraser_selected = false;

    this->data = data;

    /* New layer to draw */
    QPixmap* new_empty_pixmap = new QPixmap(data->getImage()->size());
    new_empty_pixmap->fill(Qt::transparent);
    pixmap_cache = *new_empty_pixmap;

    /* Draw parasites */
    QPainter painter(&pixmap_cache);

    for(int i = 0; i < data->getParasits()->size(); i++) {

        if(data->getParasits()->at(i)->getGT() == 0)
            painter.setPen(QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        else
           painter.setPen(QPen(Qt::gray, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        QList<QPointF> pixels = *data->getParasits()->at(i)->getPixels();

        QPainterPath path;
        path.moveTo(pixels.first());

        for (int i = 1; i < pixels.size(); ++i)
            path.lineTo(pixels.at(i));

        painter.drawPath(path);
    }

    update();
}

QRectF ImageItem::boundingRect() const
{
    return QRectF(0, 0, pixmap_cache.width(), pixmap_cache.height());
}

QPainterPath ImageItem::shape() const
{
    QPainterPath path;
    path.addRect(0, 0, pixmap_cache.width(), pixmap_cache.height());
    return path;
}

void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainter image_painter(&pixmap_cache);

    // Draw red ink
    if (stuff.size() > 1) {

        if(eraser_selected) {

            QPen p = image_painter.pen();

            image_painter.setCompositionMode(QPainter::CompositionMode_Source);
            image_painter.setRenderHint(QPainter::Antialiasing);
            image_painter.setPen(QPen(Qt::transparent, 10));
            image_painter.setBrush(Qt::NoBrush);

            QPainterPath path;
            path.moveTo(stuff.first());

            for (int i = 1; i < stuff.size(); ++i)
                path.lineTo(stuff.at(i));

            image_painter.drawPath(path);
            image_painter.setPen(p);
        }
        else {

            QPen p = image_painter.pen();

            if(parasite_pen_selected)
                image_painter.setPen(QPen(Qt::white, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            else
                image_painter.setPen(QPen(QColor(PIXEL_UNDEFINED), 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

            image_painter.setBrush(Qt::NoBrush);

            QPainterPath path;
            path.moveTo(stuff.first());

            for (int i = 1; i < stuff.size(); ++i)
                path.lineTo(stuff.at(i));

            image_painter.drawPath(path);
            image_painter.setPen(p);
        }
    }

    painter->drawPixmap(0, 0, pixmap_cache.width(), pixmap_cache.height(), pixmap_cache);
}

void ImageItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        stuff << event->pos();
        update();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void ImageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    stuff.clear();
    update();
}

void ImageItem::setParasitePen(bool value)
{
    parasite_pen_selected = value;
}

void ImageItem::setEraserPen(bool value)
{
    eraser_selected = value;
}

void ImageItem::saveImage()
{
    QImage* new_image = new QImage(data->getData()->size(),QImage::Format_RGB32);
    new_image->fill(Qt::black);

    QPainter painter (new_image);
    painter.drawPixmap(0,0,pixmap_cache.width(),pixmap_cache.height(),pixmap_cache);

    data->saveImage(new_image,data->getDataPath());
}
