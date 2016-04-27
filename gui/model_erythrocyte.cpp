
#include "model_erythrocyte.h"

Erythrocyte::Erythrocyte(int id, QPointF center, qreal radius) {
    this->id = id;
    this->center = center;
    this->radius = radius;
}

int Erythrocyte::getId() {
    return id;
}

QPointF Erythrocyte::getCenter() {
    return center;
}

QPointF Erythrocyte::getPos() {
    return QPointF(center.rx()-radius, center.ry()-radius);
}

qreal Erythrocyte::getRadius() {
    return radius;
}
