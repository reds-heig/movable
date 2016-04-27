#ifndef PARASIT_H
#define PARASIT_H

#define PIXEL_NEGATIF   4278190080
#define PIXEL_UNDEFINED 4286545791
#define PIXEL_POSITIF   4294967295

#define KERNEL_SIZE     51
#define DELTA           51

#include <QPixmap>

class Parasit
{
    public:
        Parasit(int id, QImage image, int pos_x, int pos_y, int size_kernel, unsigned int gt);
        Parasit(int id, QImage image, QList<QPointF>* pixels, unsigned int gt);

        int getId();
        int getX();
        int getY();
        int getImageX();
        int getImageY();
        int getSize();

        unsigned int getGT();
        void setGT(unsigned int gt);

        QPixmap getView();

        QList<QPointF>* getPixels();

    private:
        int id;

        QList<QPointF>* pixels;

        unsigned int gt;  /* Parasit / Negatif / Undefined */

        int pos_x;  /* Parasit */
        int pos_y;  /* Parasit */

        int pos_image_x;    /* position top left corner */
        int pos_image_y;    /* position top left corner */

        int size_kernel;
        int size_kernel_x;
        int size_kernel_y;

        QPixmap view;
};

#endif
