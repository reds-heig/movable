
#include "model_parasit.h"

#include <QDebug>
#include <QtMath>

Parasit::Parasit(int id, QImage image, int pos_x, int pos_y, int size_kernel, unsigned int gt) {

    this->id = id;

    this->pos_x = pos_x;
    this->pos_y = pos_y;

    this->gt = gt;

    this->used = false;

    QPixmap pixmap = QPixmap::fromImage(image);

    int delta = (size_kernel-1)/2;

    this->pos_image_x = pos_x - delta;
    this->pos_image_y = pos_y - delta;

    this->size_kernel = size_kernel;

    view = pixmap.copy(pos_image_x, pos_image_y, size_kernel, size_kernel);
}

Parasit::Parasit(int id, QImage image, QList<QPointF>* pixels, unsigned int gt) {

    this->id = id;

     this->used = false;

    QPixmap pixmap = QPixmap::fromImage(image);

    this->pixels = pixels;

    pos_x = (this->pixels->first().x() + this->pixels->last().x())/2 + 10;
    pos_y = (this->pixels->first().y() + this->pixels->last().y())/2 + 10;

    this->size_kernel = 40;

    int delta = (size_kernel-1)/2;

    this->pos_image_x = pos_x - delta;
    this->pos_image_y = pos_y - delta;

    view = pixmap.copy(pos_image_x, pos_image_y, size_kernel, size_kernel);

    this->gt = gt;
}

int Parasit::getId() {
        return id;
}

int Parasit::getX() {
    return pos_x;
}

int Parasit::getY() {
    return pos_y;
}

int Parasit::getImageX() {
    return pos_image_x;
}

int Parasit::getImageY() {
    return pos_image_y;
}

int Parasit::getSize() {
    return size_kernel;
}

unsigned int Parasit::getGT() {
    return gt;
}

void Parasit::setGT(unsigned int gt) {
    this->gt = gt;
}

QList<QPointF>* Parasit::getPixels() {
    return pixels;
}

QPixmap Parasit::getView() {
    return view;
}

