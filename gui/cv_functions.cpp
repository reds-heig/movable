
#include "cv_functions.h"

#include <iostream>

using namespace cv;
using namespace std;

int cv_found_erythrocytes(char *filename,
                          vector<Vec3f> *circles,
                          double dp,
                          double min_dist,
                          double param1,
                          double param2,
                          int min_radius,
                          int max_radius)
{
    try
    {
        /* image reader */
        Mat img = imread(filename,0);
        medianBlur(img, img, 5);
        HoughCircles(img, *circles, HOUGH_GRADIENT, dp, min_dist, param1, param2, min_radius, max_radius);
        return circles->size();
    }
    catch( cv::Exception& e )
    {
        return -1;
    }

}

int cv_found_erythrocytes(Mat img,
                          vector<Vec3f> *circles,
                          double dp,
                          double min_dist,
                          double param1,
                          double param2,
                          int min_radius,
                          int max_radius)
{
    try
    {
        /* image reader */
        medianBlur(img, img, 5);
        HoughCircles(img, *circles, HOUGH_GRADIENT, dp, min_dist, param1, param2, min_radius, max_radius);
        return circles->size();
    }
    catch( cv::Exception& e )
    {
        return -1;
    }

}

