#include "model_blood_image.h"

//#include "opencv2/imgcodecs.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"

#include "cv_functions.h"

#include <QDebug>
#include <QPointF>
BloodImage::BloodImage(int id, QImage *data_image, QImage *data_simulation, QString path_img, QString path) {
    this->id = id;
    this->image = data_image;
    this->data = data_simulation;
    this->parasits = NULL;
    this->data_path = path;
    this->image_path = path_img;

    this->dp = 1;
    this->min_dist = 10;
    this->param1 = 50;
    this->param2 = 12;
    this->min_radius = 11;
    this->max_radius = 20;
}

int BloodImage::getId() {
    return id;
}

QImage* BloodImage::getImage() {
    return image;
}

QImage* BloodImage::getData() {
    return data;
}

QString BloodImage::getImagePath() {
    return image_path;
}

QString BloodImage::getDataPath() {
    return data_path;
}

QList<Parasit*>* BloodImage::getParasits() {
    return parasits;
}

QList<Erythrocyte*>* BloodImage::getErythrocytes() {
    return erythrocytes;
}


void clustering(QList<QPointF>* cluster, int x, int y, QImage* image) {

    if(image->pixel(x,y) == PIXEL_NEGATIF) {
        return;
    }
    else {
        cluster->push_back(QPoint(x,y));
        image->setPixel(x,y,PIXEL_NEGATIF);
    }

    if(x > 0)
        clustering(cluster, x-1, y,     image);

    if(y > 0)
        clustering(cluster, x,   y-1,   image);

    if(y < image->height()-1)
        clustering(cluster, x,   y+1,   image);

    if(x < image->width()-1)
        clustering(cluster, x+1, y,     image);
}


int BloodImage::foundParasits() {

    if(image == NULL || data == NULL) {
        return -1;
    }

    if(parasits != NULL) {
        parasits->clear();
    }
    else {
        parasits = new QList<Parasit*>;
    }

    QImage base_img = this->image->convertToFormat(QImage::Format_RGB32);
    QImage img = this->data->convertToFormat(QImage::Format_RGB32);

    QImage data = img.copy(0,0,img.width(),img.height());

    QList<QList<QPointF>*> parasit_clusters;
    QList<QList<QPointF>*> undefined_clusters;

    for(int i = 0; i < data.width(); i ++) {
        for(int j = 0; j < data.height(); j++) {

            if(data.pixel(i,j) == PIXEL_POSITIF) {
                parasit_clusters.push_back(new QList<QPointF>());
                clustering(parasit_clusters.last(), i,j, &data);
            }
            else if(data.pixel(i,j) == PIXEL_UNDEFINED) {
                undefined_clusters.push_back(new QList<QPointF>());
                clustering(undefined_clusters.last(), i,j, &data);
            }
        }
    }

    for(int i=0 ; i < parasit_clusters.size(); i++) {
        parasits->push_back(new Parasit(i,
                                        base_img,
                                        parasit_clusters.at(i),
                                        0));
    }

    for(int i=0 ; i < undefined_clusters.size(); i++) {
        parasits->push_back(new Parasit(i + parasit_clusters.size(),
                                        base_img,
                                        undefined_clusters.at(i),
                                        200));
    }

    return parasits->size();
}

int BloodImage::foundErythrocytes() {

    std::vector<cv::Vec3f> *circles = new std::vector<cv::Vec3f>;

    int size = cv_found_erythrocytes(image_path.toLatin1().data(),
                                     circles,
                                     this->dp,
                                     this->min_dist,
                                     this->param1,
                                     this->param2,
                                     this->min_radius,
                                     this->max_radius);

//    qDebug() << size;

    erythrocytes = new QList<Erythrocyte*>();

    if(size != 0) {
        for(size_t i=0; i < circles->size(); i++) {
            erythrocytes->push_back(new Erythrocyte(i, QPointF(circles->at(i)[0],circles->at(i)[1]), circles->at(i)[2]));
        }
    }

    delete circles;

    return size;
}

int BloodImage::parasitemie() {

    int score = 0;

    //Parcours tous les parasites
    for(int i = 0; i < parasits->size(); i++) {

            //Cherche erythrocytes
            for(int k = 0; k < erythrocytes->size(); k++) {

                Erythrocyte* e = erythrocytes->at(k);

                //Pixels
                QPointF average = parasits->at(i)->getPixels()->first();

                for(int j=1; j < parasits->at(i)->getPixels()->size(); j++) {
                    average += parasits->at(i)->getPixels()->at(j);
                }

                average /= parasits->at(i)->getPixels()->size();

                if(e->getPos().rx() <= average.rx() && average.rx() <= e->getPos().rx() + e->getRadius()*2 &&
                        e->getPos().ry() <= average.ry() && average.ry() <= e->getPos().ry() + e->getRadius()*2) {
                    score++;
                    break;
                }
            }
    }


    return score;
}

void BloodImage::saveImage(QImage *data_simulation, QString filename) {
    this->data = data_simulation;
    this->data->save(filename);
    foundParasits();
}
