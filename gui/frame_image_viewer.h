#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include "model_blood_image.h"
#include "item_erythrocyte.h"
#include "item_parasit.h"
#include "item_image.h"

#include <QFrame>
#include <QtWidgets>
#include <QGraphicsView>
#include <QLabel>
#include <QPlainTextEdit>

class ImageViewer : public QFrame
{
    Q_OBJECT
public:
    explicit ImageViewer(const QString &name, QWidget *parent = 0);

    BloodImage* getImage();
    void setImage(BloodImage* image);

    QList<ParasiteItem*> *getParasitViews();
    void setParasitViews(QList<ParasiteItem*> *views);

    QGraphicsView *view;

public slots:
    void loadScene(BloodImage* image);
    void centerOn(int id);

private slots:
    void selectPreviousImage();
    void selectNextImage();
    void zoomIn();
    void zoomOut();

    /* Tools */
    void setPointerMode();
//    void setPenPointer();
    void setBigPen();
    void setSmallPen();
    void saveImage();

private:

    void viewAllItems();
    void viewDrawedItems();

    /* Manage viewer */
    QPushButton *previous_image;
    QPushButton *next_image;
    QPushButton *zoom_in;
    QPushButton *zoom_out;

    /* Panel (Viewer)*/
    QToolButton *mouse;
    QToolButton *hand;
    QToolButton *mouse_erythrocytes;
    QToolButton *mouse_parasites;

    /* Panel (Parasites)*/
    QToolButton *pen_undefined_item;
    QToolButton *pen_parasite_item;
    QToolButton *eraser;
//    QPushButton *take_big_cursor;
//    QPushButton *take_small_cursor;

    /* Panel (Manage image)*/
    QPushButton *save_image;

    //Frame
    QGraphicsScene* scene;
    ImageItem *drawing_area;

    QList<ParasiteItem*>* parasits;
    QList<ErythrocyteItem*> *erythrocytes;

    qreal zoom_value;

    //Models
    BloodImage* image;
    int parasit_selected;
};

#endif
