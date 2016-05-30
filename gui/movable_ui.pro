
#Qt version check
!contains(QT_VERSION, ^5\\.*\\..*) {
    message("Cannot build Quotations with Qt version $${QT_VERSION}.")
    error("Use at least Qt 5.0.")
}

UI_DIR = tmp/ui
MOC_DIR = tmp/moc
OBJECTS_DIR = tmp/obj
RCC_DIR = tmp/rcc

RESOURCES += images.qrc

HEADERS += movable_ui.h frame_image_viewer.h item_parasit.h item_erythrocyte.h item_image.h widget_erythrocyte_editor.h dialog_new_simulation.h process_test_movable.h
HEADERS += cv_functions.h
HEADERS += model_simulation.h model_blood_image.h model_parasit.h model_erythrocyte.h

SOURCES += main.cpp movable_ui.cpp frame_image_viewer.cpp item_parasit.cpp item_erythrocyte.cpp item_image.cpp widget_erythrocyte_editor.cpp dialog_new_simulation.cpp
SOURCES += cv_functions.cpp
SOURCES += model_simulation.cpp model_blood_image.cpp model_parasit.cpp model_erythrocyte.cpp

QT += widgets
QT += gui
qtHaveModule(printsupport): QT += printsupport
qtHaveModule(opengl): QT += opengl

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

INCLUDEPATH += /usr/local/include/opencv2
#LIBS += -L/usr/local/lib
#LIBS += -lopencv_cudabgsegm
#LIBS += -lopencv_cudaobjdetect
#LIBS += -lopencv_cudastereo
#LIBS += -lopencv_shape
#LIBS += -lopencv_stitching
#LIBS += -lopencv_cudafeatures2d
#LIBS += -lopencv_superres
#LIBS += -lopencv_cudacodec
#LIBS += -lopencv_videostab
#LIBS += -lopencv_cudaoptflow
#LIBS += -lopencv_cudalegacy
#LIBS += -lopencv_calib3d
#LIBS += -lopencv_features2d
#LIBS += -lopencv_objdetect
 LIBS += -lopencv_highgui
#LIBS += -lopencv_videoio
#LIBS += -lopencv_photo
 LIBS += -lopencv_imgcodecs
#LIBS += -lopencv_cudawarping
#LIBS += -lopencv_cudaimgproc
#LIBS += -lopencv_cudafilters
#LIBS += -lopencv_video
#LIBS += -lopencv_ml
 LIBS += -lopencv_imgproc
#LIBS += -lopencv_flann
#LIBS += -lopencv_cudaarithm
LIBS += -lopencv_core
#LIBS += -lopencv_cudev

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/imageviewer
INSTALLS += target

wince {
   DEPLOYMENT_PLUGIN += qjpeg qgif qpng
}
