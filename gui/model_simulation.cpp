#include "model_simulation.h"

Simulation::Simulation() {
    blood_images = new QList<BloodImage*>();
}

QList<BloodImage*>* Simulation::getBloodImages() {
    return blood_images;
}

void Simulation::addImage(QImage *image, QImage *simulation, QString path_img, QString path) {
    blood_images->push_back(new BloodImage(blood_images->length(), image, simulation, path_img, path));
    blood_images->last()->foundParasits();
    blood_images->last()->foundErythrocytes();
}
