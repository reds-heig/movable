
#include "widget_erythrocyte_editor.h"
#include <QtWidgets>

ErythrocyteEditor::ErythrocyteEditor(BloodImage *image, QWidget *parent)
    : QWidget(parent) {

    /* Initialisation */
    this->image = image;
    this->img_reader = new Mat(imread(image->getImagePath().toStdString().c_str(), 0));
    this->label = new QLabel;
    this->label->setPixmap(QPixmap::fromImage(*image->getImage()));

//    this->scroll_area = new QScrollArea;
//    this->scroll_area->setBackgroundRole(QPalette::Dark);
//    this->scroll_area->setWidget(this->label);
//    this->scroll_area->setVisible(false);

    /* Data */
    QVBoxLayout *img_layout = new QVBoxLayout();
    img_layout->addWidget(this->label);
//    img_layout->addWidget(this->scroll_area);

    /* Sliders */
    QGridLayout *sliders_layout = new QGridLayout;
    sliders_layout->addWidget(new QLabel("Param1 "),0,0);
    sliders_layout->addWidget(new QLabel("Param2 "),1,0);

    slider_param1 = new QSlider;
    slider_param1->setOrientation(Qt::Horizontal);
    slider_param1->setMinimum(1);
    slider_param1->setMaximum(300);
    slider_param1->setValue(image->param1);
    slider_param1->setTickPosition(QSlider::TicksRight);

    slider_param2 = new QSlider;
    slider_param2->setOrientation(Qt::Horizontal);
    slider_param2->setMinimum(1);
    slider_param2->setMaximum(25);
    slider_param2->setValue(image->param2);
    slider_param2->setTickPosition(QSlider::TicksRight);

    sliders_layout->addWidget(slider_param1,0,1);
    sliders_layout->addWidget(slider_param2,1,1);

    img_layout->addLayout(sliders_layout);

    /* Tools */
    QGridLayout *txt_layout = new QGridLayout;
    txt_layout->addWidget(new QLabel("Resolution [0.1, 1.0] "),0,0);
    txt_layout->addWidget(new QLabel("Min. distance between \ntwo erythrocytes [pixels] "),1,0);
    txt_layout->addWidget(new QLabel("Min. radius [pixels] "),4,0);
    txt_layout->addWidget(new QLabel("Max. radius [pixels] "),5,0);

    this->txt_dp = new QTextEdit(QString::number(image->dp));
    this->txt_min_dist = new QTextEdit(QString::number(image->min_dist));
    this->txt_min_radius = new QTextEdit(QString::number(image->min_radius));
    this->txt_max_radius = new QTextEdit(QString::number(image->max_radius));

    QSize size(50,30);
    this->txt_dp->setFixedSize(size);
    this->txt_min_dist->setFixedSize(size);
    this->txt_min_radius->setFixedSize(size);
    this->txt_max_radius->setFixedSize(size);

    txt_layout->addWidget(this->txt_dp,0,1);
    txt_layout->addWidget(this->txt_min_dist,1,1);
    txt_layout->addWidget(this->txt_min_radius,4,1);
    txt_layout->addWidget(this->txt_max_radius,5,1);

    btn_refresh = new QPushButton("Refresh");
    btn_save = new QPushButton("Save");

    QHBoxLayout *btn_layout = new QHBoxLayout;
    btn_layout->addWidget(btn_refresh);
    btn_layout->addWidget(btn_save);

    QVBoxLayout *tools_layout = new QVBoxLayout;
    tools_layout->addLayout(txt_layout);
    tools_layout->addLayout(btn_layout);
    tools_layout->addStretch();

    /* Main layout */
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addLayout(img_layout);
    layout->addLayout(tools_layout);

    drawImage();

    connect(btn_refresh, SIGNAL(clicked()), this, SLOT(refresh()));
    connect(btn_save, SIGNAL(clicked()), this, SLOT(save()));
    connect(slider_param1, SIGNAL(sliderMoved(int)), this, SLOT(refresh()));
    connect(slider_param2, SIGNAL(sliderMoved(int)), this, SLOT(refresh()));
}

double ErythrocyteEditor::getDoubleParams(QString str) {
    return str.toDouble();
}

int ErythrocyteEditor::getIntegerParams(QString str) {
    return str.toInt();
}

void ErythrocyteEditor::drawImage() {

    /* Image preview */
    QImage preview(*this->image->getImage());

    /* Erythorcytes */
    std::vector<cv::Vec3f> *circles = new std::vector<cv::Vec3f>;
    int s = cv_found_erythrocytes(*img_reader,
                                  circles,
                                  getDoubleParams(this->txt_dp->toPlainText()),
                                  getDoubleParams(this->txt_min_dist->toPlainText()),
                                  this->slider_param1->value(),
                                  this->slider_param2->value(),
                                  getIntegerParams(this->txt_min_radius->toPlainText()),
                                  getIntegerParams(this->txt_max_radius->toPlainText()));

    qDebug() << this->txt_dp->toPlainText() << "," <<
                this->txt_min_dist->toPlainText() << "," <<
                this->slider_param1->value() << "," <<
                this->slider_param2->value() << "," <<
                this->txt_min_radius->toPlainText() << "," <<
                this->txt_max_radius->toPlainText() << " :: " << s;

    /* Draw circles */
    if(s != -1) {
        QPainter painter(&preview);

        for(size_t i = 0; i < circles->size(); i++)
        {
            painter.setPen(QPen(QBrush(QColor(255,0,0)),2));
            painter.drawEllipse(circles->at(i)[0]-circles->at(i)[2],
                    circles->at(i)[1]-circles->at(i)[2],
                    circles->at(i)[2]*2,
                    circles->at(i)[2]*2);
        }

        label->setPixmap(QPixmap::fromImage(preview));
    }

    delete circles;
}

void ErythrocyteEditor::refresh() {
    drawImage();
}

void ErythrocyteEditor::save() {
    this->image->dp = getDoubleParams(this->txt_dp->toPlainText());
    this->image->min_dist = getDoubleParams(this->txt_min_dist->toPlainText());
    this->image->param1 = this->slider_param1->value();
    this->image->param2 = this->slider_param2->value();
    this->image->min_radius = getIntegerParams(this->txt_min_radius->toPlainText());
    this->image->max_radius = getIntegerParams(this->txt_max_radius->toPlainText());
}

