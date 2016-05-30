#ifndef ERYTHROCYTEEDITOR_H
#define ERYTHROCYTEEDITOR_H

#include "model_blood_image.h"
#include "cv_functions.h"

#include <QtWidgets>

class ErythrocyteEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ErythrocyteEditor(BloodImage *image, QWidget *parent = 0);
    double getDoubleParams(QString str);
    int getIntegerParams(QString str);
    void drawImage();

private slots:
    void refresh();
    void save();

private:

    /* Label */
    QLabel *label;
    QScrollArea *scroll_area;

    /* Image */
    BloodImage *image;
    Mat *img_reader;

    /* Sliders */
    QSlider *slider_param1;
    QSlider *slider_param2;

    /* TextEdit */
    QTextEdit *txt_dp;
    QTextEdit *txt_min_dist;
    QTextEdit *txt_param1;
    QTextEdit *txt_param2;
    QTextEdit *txt_min_radius;
    QTextEdit *txt_max_radius;

    /* Buttons */
    QPushButton *btn_refresh;
    QPushButton *btn_save;
    QPushButton *btn_save_for_all;
};

#endif
