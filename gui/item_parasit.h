#ifndef PARASITEITEM_H
#define PARASITEITEM_H

#include "model_parasit.h"

#include <QColor>
#include <QGraphicsItem>
#include <QPixmap>

class ParasiteItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    explicit ParasiteItem (Parasit* p);

    Parasit* getParasit();

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) Q_DECL_OVERRIDE;

signals:
    void parasiteSelected(int id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

private:
    Parasit *parasit;

};

#endif
