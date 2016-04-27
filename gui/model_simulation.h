#ifndef SIMULATION_H
#define SIMULATION_H

#include "model_blood_image.h"

#include <QImage>
#include <QList>

class Simulation
{
    public:
        Simulation();

        QList<BloodImage*> *getBloodImages();

        void addImage(QImage *image, QImage *simulation, QString path_img, QString path);

    private:
        QString name;
        QList<BloodImage*> *blood_images;
};

#endif
