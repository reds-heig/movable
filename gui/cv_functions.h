
#ifndef CVFUNCTIONS_H
#define CVFUNCTIONS_H

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int cv_found_erythrocytes(char *filename, vector<Vec3f> *circles);

#endif
