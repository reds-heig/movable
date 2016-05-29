#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    FindDialog(QString *simulation_file, QWidget *parent = 0);

public slots:
    void cancelSimulation();
    void createSimulation();

    void browseSimulation();
    void browseImages();
    void browseMasks();
    void browseClassifier();

private:
    QString *parent_simulation_file;

    QLineEdit *simulation_name;
    QLineEdit *simulation_path;
    QLineEdit *images_path;
    QLineEdit *masks_path;
    QLineEdit *classifier_path;

    QPushButton *btn_cancel;
    QPushButton *btn_create;

    QPushButton *btn_browse_simulation;
    QPushButton *btn_browse_images;
    QPushButton *btn_browse_masks;
    QPushButton *btn_browse_classifier;
};

#endif

//#ifndef SIMULATIONEDITOR_H
//#define SIMULATIONEDITOR_H

//#include <QtWidgets>

//class SimulationEditor : public QWidget
//{
//    Q_OBJECT

//public:
//    explicit SimulationEditor(QString title, QWidget *parent = 0);

//private slots:
//    void createSimulation();
//    void cancelSimulation();

//private:
//    QLineEdit *simulation_name;
//    QLineEdit *simulation_path;
//    QLineEdit *images_path;
//    QLineEdit *masks_path;

//    QPushButton *btn_cancel;
//    QPushButton *btn_create;
//};

//#endif


////#ifndef DIALOGNEWSIMULATION_H
////#define DIALOGNEWSIMULATION_H

////#include <QtWidgets>

////class NewSimulation : public QDialog
////{
////    //Q_OBJECT
////public:
//////    NewSimulation(QWidget *parent = 0);
//////    ~NewSimulation();
////    NewSimulation();

////public slots:
////    void cancelSimulation();
////    void createSimulation();

////private:
////    QLineEdit *simulation_name;
////    QLineEdit *simulation_path;
////    QLineEdit *images_path;
////    QLineEdit *masks_path;

////    QPushButton *btn_cancel;
////    QPushButton *btn_create;
////};

////#endif
