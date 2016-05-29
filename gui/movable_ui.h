#ifndef MOVABLEUI_H
#define MOVABLEUI_H

#include "model_simulation.h"
#include "frame_image_viewer.h"
#include "dialog_new_simulation.h"

#include <QMainWindow>
#include <QtWidgets>

#include <QSortFilterProxyModel>

#ifndef QT_NO_PRINTER
#include <QPrinter>
#endif

class FileFilterProxyModel : public QSortFilterProxyModel
{
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
};

class MovableUI : public QMainWindow
{
    Q_OBJECT

public:
    MovableUI();
    bool loadFile(const QString &);
    QSplitter* v1Splitter;
    QSplitter* v2Splitter;
    QSplitter* hSplitter;

public slots:
    void selectNextBloodImage();
    void selectPreviousBloodImage();
    void selectBloodImage(int id);
    void selectParasit(int id);

private slots:
    void create();
    void open();
    void close();
    void about();
    void print();
    void save();
    void openCreatedSimulation(int result);

private:
    void createActions();
    void createMenus();

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    QAction *newAct;
    QAction *openAct;
    QAction *resizeWindowAct;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *printAct;

    QMenu *fileMenu;
    QMenu *helpMenu;

    //Frames
    FindDialog *new_simulation_editor;

    ImageViewer* image_viewer;

    QListWidget* list_images_viewer;
    QListWidget* list_parasits_viewer;

    QLabel *infos;

    //Models
    Simulation* simulation;
    QString *simulation_file;

    int image_selected;
    int parasit_selected;
};

#endif
