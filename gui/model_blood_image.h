#ifndef BLOOD_IMAGE_H
#define BLOOD_IMAGE_H

#define SIZE_VIEW_IMAGE 60

#include "model_parasit.h"
#include "model_erythrocyte.h"

#include <QImage>
#include <QList>

class BloodImage
{
    public:
        BloodImage(int id, QImage *image, QImage *data, QString path_image, QString path_data);

        int getId();

        QImage *getImage();
        QImage *getData();

        QString getImagePath();
        QString getDataPath();

        QList<Parasit*> *getParasits();
        QList<Erythrocyte*> *getErythrocytes();


        int foundParasits();
        int foundErythrocytes();
        int parasitemie();

        void saveImage(QImage *data_simulation, QString filename) ;

        /* Configuration */
        double dp;
        double min_dist;
        double param1;
        double param2;
        int min_radius;
        int max_radius;

    private:

        int id;

        QImage *image;
        QImage *data;

        QString image_path;
        QString data_path;

        QList<Parasit*> *parasits;
        QList<Erythrocyte*> *erythrocytes;
};

#endif
