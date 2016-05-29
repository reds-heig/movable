#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include "model_blood_image.h"
#include "item_erythrocyte.h"
#include "item_parasit.h"
#include "item_image.h"
#include "widget_erythrocyte_editor.h"

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

    /* Panel (Manage image)*/
    QPushButton *save_image;
    ImageItem *drawing_area;
    QPushButton *previous_image;
    QPushButton *next_image;

public slots:
    void loadScene(BloodImage* image);
    void centerOn(int id);

private slots:
    void zoomIn();
    void zoomOut();

    /* Tools */
    void setPointerMode();
    void setBigPen();
    void setSmallPen();
    void showErythrocytesEditor();

private:
    void viewAllItems();
    void viewDrawedItems();

    /* Manage viewer */
    QPushButton *zoom_in;
    QPushButton *zoom_out;

    /* Panel (Viewer) */
    QToolButton *mouse;
    QToolButton *hand;
    QToolButton *mouse_erythrocytes;
    QToolButton *mouse_parasites;

    /* Panel (Erythrocytes) */
    QPushButton *settings;

    /* Panel (Parasites) */
    QToolButton *pen_undefined_item;
    QToolButton *pen_parasite_item;
    QToolButton *eraser;

    ErythrocyteEditor* editor;

    //Frame
    QGraphicsScene* scene;
    QList<ParasiteItem*>* parasits;
    QList<ErythrocyteItem*> *erythrocytes;

    qreal zoom_value;

    //Models
    BloodImage* image;
    int parasit_selected;
};

#endif
