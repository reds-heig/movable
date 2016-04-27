
#include "frame_image_viewer.h"

#ifndef QT_NO_PRINTER
#include <QPrinter>
#include <QPrintDialog>
#endif

#ifndef QT_NO_OPENGL
#include <QtOpenGL>
#else
#include <QtWidgets>
#endif

#include <qmath.h>

#include "movable_ui.h"
#include "item_image.h"

ImageViewer::ImageViewer(const QString &name, QWidget *parent)
    : QFrame(parent) {

    setFrameStyle(Sunken | StyledPanel);

    view = new QGraphicsView(this);
    view->setRenderHint(QPainter::Antialiasing, false);
    view->setDragMode(QGraphicsView::RubberBandDrag);

    /* Actions buttons */
    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    /* Manage viewer */
    previous_image = new QPushButton;
    previous_image->setIcon(QPixmap(":/images/prev.png"));
    previous_image->setIconSize(iconSize);

    next_image = new QPushButton;
    next_image->setIcon(QPixmap(":/images/next.png"));
    next_image->setIconSize(iconSize);

    zoom_in = new QPushButton;
    zoom_in->setIcon(QPixmap(":/images/Zoom_in.png"));
    zoom_in->setIconSize(iconSize);

    zoom_out = new QPushButton;
    zoom_out->setIcon(QPixmap(":/images/Zoom_out_2.png"));
    zoom_out->setIconSize(iconSize);

    /* Panel (Viewer)*/
    QButtonGroup *button_group = new QButtonGroup;
    button_group->setExclusive(true);

    mouse = new QToolButton;
    mouse->setIcon(QPixmap(":/images/outil_selecteur.png"));
    mouse->setIconSize(iconSize);
    mouse->setCheckable(true);
    mouse->setChecked(true);

    button_group->addButton(mouse);

    hand = new QToolButton;
    hand->setIcon(QPixmap(":/images/outil_hand.png"));
    hand->setIconSize(iconSize);
    hand->setCheckable(true);
    hand->setChecked(false);

    button_group->addButton(hand);

    /* Panel (Erythrocytes)*/
    mouse_erythrocytes = new QToolButton;
    mouse_erythrocytes->setIcon(QPixmap(":/images/erythrocyte_mouse.jpg"));
    mouse_erythrocytes->setIconSize(iconSize);
    mouse_erythrocytes->setCheckable(true);
    mouse_erythrocytes->setChecked(true);

    mouse_parasites = new QToolButton;
    mouse_parasites->setIcon(QPixmap(":/images/parasite_mouse.jpg"));
    mouse_parasites->setIconSize(iconSize);
    mouse_parasites->setCheckable(true);
    mouse_parasites->setChecked(true);

    /* Panel (Parasites)*/
    pen_parasite_item = new QToolButton;
    pen_parasite_item->setIcon(QPixmap(":/images/pen_parasite.jpg"));
    pen_parasite_item->setIconSize(iconSize);
    pen_parasite_item->setCheckable(true);
    pen_parasite_item->setChecked(false);

    button_group->addButton(pen_parasite_item);

    pen_undefined_item = new QToolButton;
    pen_undefined_item->setIcon(QPixmap(":/images/pen_undefined_item.jpg"));
    pen_undefined_item->setIconSize(iconSize);
    pen_undefined_item->setCheckable(true);
    pen_undefined_item->setChecked(false);

    button_group->addButton(pen_undefined_item);

    eraser = new QToolButton;
    eraser->setIcon(QPixmap(":/images/eraser.png"));
    eraser->setIconSize(iconSize);
    eraser->setCheckable(true);
    eraser->setChecked(false);

    button_group->addButton(eraser);

//    /* Panel (Cursor size) */
//    take_big_cursor = new QPushButton;
//    take_big_cursor->setIcon(QPixmap(":/images/pen_plus.jpg"));
//    take_big_cursor->setIconSize(iconSize);

//    take_small_cursor = new QPushButton;
//    take_small_cursor->setIcon(QPixmap(":/images/pen_minus.jpg"));
//    take_small_cursor->setIconSize(iconSize);

    /* Panel (Manage image) */
    save_image = new QPushButton("Save");

    /* Top layout */
    QHBoxLayout *top_layout = new QHBoxLayout;
    top_layout->addWidget(new QLabel(name));
    top_layout->addStretch();

    top_layout->addWidget(zoom_in);
    top_layout->addWidget(zoom_out);
    top_layout->addWidget(previous_image);
    top_layout->addWidget(next_image);

    /* Grid layout */
    QGridLayout *tools_layout = new QGridLayout;
    tools_layout->addWidget(new QLabel(tr("Tools image viewer")),0,0);

    QHBoxLayout *tools_image_panel = new QHBoxLayout;
    tools_image_panel->addWidget(mouse);
    tools_image_panel->addWidget(hand);
    tools_image_panel->addWidget(new QLabel(tr("|")));
    tools_image_panel->addWidget(mouse_erythrocytes);
    tools_image_panel->addWidget(mouse_parasites);

    tools_image_panel->addStretch();

    tools_layout->addLayout(tools_image_panel,1,0);

    QHBoxLayout *layout_parasites = new QHBoxLayout;
    layout_parasites->addWidget(pen_parasite_item);
    layout_parasites->addWidget(pen_undefined_item);
    layout_parasites->addWidget(eraser);
//    layout_parasites->addWidget(new QLabel(tr("|")));
//    layout_parasites->addWidget(take_big_cursor);
//    layout_parasites->addWidget(take_small_cursor);
    layout_parasites->addStretch();

    tools_layout->addWidget(new QLabel(tr("Draw parasites")),0,2);
    tools_layout->addLayout(layout_parasites,1,2);

    tools_layout->addWidget(save_image,1,3);

    /* Main layout */
    QGridLayout *main_layout = new QGridLayout;
    main_layout->addLayout(top_layout, 0, 0);
    main_layout->addWidget(view, 1, 0);
    main_layout->addLayout(tools_layout, 2, 0);

    setLayout(main_layout);

    zoom_value = 1;

    connect(previous_image, SIGNAL(clicked()), this, SLOT(selectPreviousImage()));
    connect(next_image, SIGNAL(clicked()), this, SLOT(selectNextImage()));

    connect(zoom_in, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(zoom_out, SIGNAL(clicked()), this, SLOT(zoomOut()));

    connect(mouse, SIGNAL(toggled(bool)), this, SLOT(setPointerMode()));
    connect(hand, SIGNAL(toggled(bool)), this, SLOT(setPointerMode()));
    connect(mouse_erythrocytes, SIGNAL(toggled(bool)), this, SLOT(setPointerMode()));
    connect(mouse_parasites, SIGNAL(toggled(bool)), this, SLOT(setPointerMode()));

    connect(pen_parasite_item, SIGNAL(toggled(bool)), this, SLOT(setPointerMode()));
    connect(pen_undefined_item, SIGNAL(toggled(bool)), this, SLOT(setPointerMode()));
    connect(eraser, SIGNAL(toggled(bool)), this, SLOT(setPointerMode()));

    connect(save_image, SIGNAL(clicked()), this, SLOT(saveImage()));

}

BloodImage* ImageViewer::getImage() {
    return image;
}

QList<ParasiteItem*> *ImageViewer::getParasitViews() {
    return parasits;
}

void ImageViewer::loadScene(BloodImage* image)
{
    this->image = image;
    scene = new QGraphicsScene;
    view->setScene(scene);

    /* Background */
    QGraphicsPixmapItem *background = new QGraphicsPixmapItem(QPixmap::fromImage(*image->getImage()));
    scene->addItem(background);

    /* Drawing Area */
    drawing_area = new ImageItem(image);
    drawing_area->setPos(0,0);
    scene->addItem(drawing_area);
    drawing_area->setVisible(false);
    drawing_area->setActive(false);

    /* Erythrocytes */
    erythrocytes = new QList<ErythrocyteItem*>();

    foreach (Erythrocyte *e, *image->getErythrocytes())
    {
        ErythrocyteItem *item = new ErythrocyteItem(e);
        erythrocytes->push_back(item);
        item->setPos(e->getPos());
        scene->addItem(item);
    }

    /* Detected leukocytes */
//    foreach (ErythrocyteItem *e, *erythrocytes)
//    {
//        QList<QGraphicsItem *> list = scene->collidingItems(e);

//        if(list.size() > 6) {

//            foreach (QGraphicsItem *ery, list) {
//                if(ery->pos().rx() != 0 && ery->pos().ry() != 0)
//                    scene->removeItem(ery);
//            }
//        }
//    }

     /* Parasites */
    parasits = new QList<ParasiteItem*>();

    foreach (Parasit* p, *image->getParasits())
    {
        ParasiteItem *item = new ParasiteItem(p);
        parasits->push_back(item);
        item->setPos(p->getImageX(),p->getImageY());        
        scene->addItem(item);
    }

    parasit_selected = 0;

    QMatrix matrix;
    matrix.scale(zoom_value, zoom_value);
    view->setMatrix(matrix);
}

void ImageViewer::centerOn(int id)
{
    parasits->at(parasit_selected)->setSelected(false);

    parasit_selected = id;

    parasits->at(parasit_selected)->setSelected(true);

    qreal x = parasits->at(parasit_selected)->getParasit()->getX();
    qreal y = parasits->at(parasit_selected)->getParasit()->getY();

    view->centerOn(x,y);
}

void ImageViewer::selectPreviousImage()
{
    MovableUI* movable = (MovableUI*)parentWidget()->parentWidget()->parentWidget()->parentWidget();
    movable->selectBloodImage(image->getId()-1);
}

void ImageViewer::selectNextImage()
{
    MovableUI* movable = (MovableUI*)parentWidget()->parentWidget()->parentWidget()->parentWidget();
    movable->selectBloodImage(image->getId()+1);
}

void ImageViewer::zoomIn()
{
    zoom_value += 0.2;

    QMatrix matrix;
    matrix.scale(zoom_value, zoom_value);
    view->setMatrix(matrix);
}

void ImageViewer::zoomOut()
{
    zoom_value -= 0.2;

    QMatrix matrix;
    matrix.scale(zoom_value, zoom_value);
    view->setMatrix(matrix);
}


void ImageViewer::setPointerMode()
{
    if(pen_parasite_item->isChecked() || pen_undefined_item->isChecked() || eraser->isChecked()) {

        view->setDragMode(QGraphicsView::RubberBandDrag);
        view->setInteractive(true);

        // Drawing Mode
        drawing_area->setVisible(true);
        drawing_area->setActive(true);

        foreach (ErythrocyteItem* e, *erythrocytes)
            e->setVisible(false);

        foreach (ParasiteItem* p, *parasits)
            p->setVisible(false);

        drawing_area->setEraserPen(eraser->isChecked());
        drawing_area->setParasitePen(pen_parasite_item->isChecked());
    }
    else {

        // Edition Mode
        view->setDragMode(mouse->isChecked()
                                     ? QGraphicsView::RubberBandDrag
                                     : QGraphicsView::ScrollHandDrag);

        view->setInteractive(mouse->isChecked());

        drawing_area->setVisible(false);
        drawing_area->setActive(false);

        foreach (ErythrocyteItem* e, *erythrocytes)
            e->setVisible(mouse_erythrocytes->isChecked());

        foreach (ParasiteItem* p, *parasits)
            p->setVisible(mouse_parasites->isChecked());
    }
}

void ImageViewer::setBigPen() {

}

void ImageViewer::setSmallPen() {

}

void ImageViewer::saveImage() {
    drawing_area->saveImage();
    MovableUI* movable = (MovableUI*)parentWidget()->parentWidget()->parentWidget()->parentWidget();
    movable->selectBloodImage(image->getId());
}
