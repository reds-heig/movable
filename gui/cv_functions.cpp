
#include "cv_functions.h"

#include <iostream>

using namespace cv;
using namespace std;

int cv_found_erythrocytes(char *filename, vector<Vec3f> *circles)
{
    /* image reader */
    Mat img = imread(filename,0);
    medianBlur(img, img, 5);

    HoughCircles(img, *circles, HOUGH_GRADIENT, 1, 10, 50, 12, 11, 20);

    return circles->size();
}
