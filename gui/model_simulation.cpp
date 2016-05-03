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

QString Simulation::getInfos(int image_id) {
    QString resul;
    resul.append("Erythrocytes : ");
    resul.append(QString::number(blood_images->at(image_id)->getErythrocytes()->size()));
    resul.append("\tParasites : ");
    resul.append(QString::number(blood_images->at(image_id)->getParasits()->size()));
    resul.append("\tErythrocytes with parasites : ");
    resul.append(QString::number(blood_images->at(image_id)->parasitemie()));
    resul.append("\tParasitemia : ");
    qreal score = (float)blood_images->at(image_id)->parasitemie() / (float)blood_images->at(image_id)->getErythrocytes()->size() * 100;
    QString str_score;
    str_score.setNum(score);
    resul.append(str_score);
    resul.append(" %");

    return resul;
}
