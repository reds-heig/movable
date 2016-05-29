#include "movable_ui.h"
#include "widget_erythrocyte_editor.h"
#include "dialog_new_simulation.h"

#include <QtWidgets>

#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#endif

bool FileFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    QFileSystemModel* fileModel = qobject_cast<QFileSystemModel*>(sourceModel());
    return fileModel->fileName(index0).indexOf(".*.") < 0;
    //uncomment to call the default implementation
    //return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

MovableUI::MovableUI()
{
    image_viewer = new ImageViewer("Large viewer");

    image_viewer->setDisabled(true);

    list_images_viewer = new QListWidget(this);
    list_images_viewer->setViewMode(QListWidget::ListMode);
    list_images_viewer->setIconSize(QSize(200,200));
    list_images_viewer->setResizeMode(QListWidget::Adjust);
    list_images_viewer->setFixedWidth(220);

    list_parasits_viewer = new QListWidget(this);
    list_parasits_viewer->setViewMode(QListWidget::ListMode);
    list_parasits_viewer->setIconSize(QSize(50,50));
    list_parasits_viewer->setResizeMode(QListWidget::Adjust);
    list_parasits_viewer->setFlow(QListWidget::LeftToRight);
    list_parasits_viewer->setFixedHeight(80);

    //Layout
    QVBoxLayout *v_layout = new QVBoxLayout;
    v_layout->addWidget(list_parasits_viewer);
    v_layout->addWidget(image_viewer);

    QWidget *widget = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->addWidget(list_images_viewer);
    layout->addLayout(v_layout);

    setCentralWidget(widget);

    infos = new QLabel;
    statusBar()->addWidget(infos);

    createActions();
    createMenus();
}

bool MovableUI::loadFile(const QString &fileName) {

    qDebug() << "Open simulation : " << fileName;
    simulation = new Simulation;

    //Read files
    QFile inputFile(fileName);
    QString main_path;

    if (inputFile.open(QIODevice::ReadOnly)) {

        QTextStream in(&inputFile);

        QList<QString> path_imgs;
        QList<QString> path_gts;

        bool imgs_loading = true;

        while (!in.atEnd()) {

            QString path_tmp = in.readLine();

            if(path_tmp.isEmpty())
                continue;

            if(path_tmp.compare("[imgs]") == 0) {
                imgs_loading = true;
                continue;
            }

            if(path_tmp.compare("[gts]") == 0) {
                imgs_loading = false;
                continue;
            }

            if(path_tmp.compare("[relative_path]") == 0) {
                main_path = fileName.left(fileName.size()-fileName.split("/").last().size()-1);
                main_path.append("/");
                continue;
            }

            if(path_tmp.compare("[absolute_path]") == 0) {
                main_path = "";
                continue;
            }

            path_tmp.insert(0, main_path);

            if(imgs_loading) {
                path_imgs.append(path_tmp);
                qDebug() << "add " << path_tmp << " in [imgs]";
            }
            else {
                path_gts.append(path_tmp);
                qDebug() << "add " << path_tmp << " in [gts]";
            }

        }

        for(int i=0; i < path_imgs.size(); i++) {
            simulation->addImage(new QImage(path_imgs.at(i)), new QImage(path_gts.at(i)), path_imgs.at(i), path_gts.at(i));
            image_viewer->setDisabled(false);
        }

        inputFile.close();
    }

    //Load and process the first image
    if(simulation->getBloodImages()->size() == 0) {
        return false;
    }
    else
    {
        image_selected = 0;
        parasit_selected = 0;

        image_viewer->loadScene(simulation->getBloodImages()->at(0));

        for(int i = 0; i < simulation->getBloodImages()->size(); i++)
            list_images_viewer->addItem(new QListWidgetItem(QIcon(QPixmap::fromImage(*simulation->getBloodImages()->at(i)->getImage())), ""));

        for(int i = 0; i < simulation->getBloodImages()->at(0)->getParasits()->size(); i++)
            list_parasits_viewer->addItem(new QListWidgetItem(QIcon(simulation->getBloodImages()->at(0)->getParasits()->at(i)->getView()), ""));

        infos->setText(simulation->getInfos(0));
    }

    //Enable options
    /*printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();
    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();
    setWindowFilePath(fileName);*/

    return true;
}

void MovableUI::openCreatedSimulation(int /*result*/) {

    qDebug() << "Open created simulation : " << *simulation_file;

//    qDebug() << result << QDialog::Accepted;
//    if(result == QDialog::Accepted){
//        loadFile(*simulation_file);
//        return;
//    }

    if(!simulation_file->isEmpty()) {
        loadFile(*simulation_file);
        return;
    }

}

void MovableUI::create()
{
    simulation_file = new QString();
    new_simulation_editor = new FindDialog(simulation_file);
    new_simulation_editor->show();
    QObject::connect(new_simulation_editor, SIGNAL(finished (int)), this, SLOT(openCreatedSimulation(int)));
}

void MovableUI::open()
{
    QStringList mimeTypeFilters;

    foreach (const QByteArray &mimeTypeName, QImageReader::supportedMimeTypes())
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();

    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    QFileDialog dialog(this, tr("Open File"),
                       picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    //dialog.setMimeTypeFilters(mimeTypeFilters);
    //dialog.selectMimeTypeFilter("image/jpeg");

    dialog.setProxyModel(new FileFilterProxyModel());
    dialog.setNameFilter("SIM (*.sim)");

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
    simulation_file = new QString(dialog.selectedFiles().first());
}

void MovableUI::save()
{
    image_viewer->drawing_area->saveImage();
    selectBloodImage(image_selected);
}

void MovableUI::close()
{
    QCoreApplication::quit();
}

void MovableUI::about()
{
    QMessageBox::about(this, tr("About Image Viewer"),
                       tr("<p>MOVABLE project - REDS Institute, HEIG-VD, Yverdon-les-Bains (CH) - 2016</p>"
                          "<p>This file is part of MOVABLE.</p>"
                          "<p>MOVABLE is free software: you can redistribute it and/or modify it under the terms "
                          "of the GNU General Public License as published by the Free Software Foundation, either "
                          "version 3 of the License, or (at your option) any later version.<p>"
                          "<p>MOVABLE is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
                          "without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
                          "See the GNU General Public License for more details."
                          "<p>You should have received a copy of the GNU General Public License along with MOVABLE. "
                          "If not, see <http://www.gnu.org/licenses/>.</p>"));
}

void MovableUI::print()
{
//#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
//    QPrinter printer;
//    QPrintDialog dialog(&printer, this);
//    if (dialog.exec() == QDialog::Accepted) {
//        QPainter painter(&printer);
//        image_viewer->view->render(&painter);
//    }
//#endif

    QString filename = QFileDialog::getSaveFileName(this, "Save file", "", ".png");

    if(!filename.isNull() && !filename.isEmpty()) {
        qDebug() << filename;
        filename.append(".png");

        QImage newImage(image_viewer->getImage()->getImage()->size(), QImage::Format_RGB32);
        newImage.fill(qRgb(255, 255, 255));

        QPainter painter(&newImage);
        painter.drawPixmap(0,0,
                           image_viewer->getImage()->getImage()->size().width(),
                           image_viewer->getImage()->getImage()->size().height(),
                           QPixmap::fromImage(*image_viewer->getImage()->getImage()));

        for(int i=0; i < image_viewer->getImage()->getParasits()->size();i++)
        {
            QColor fillColor = QColor(255,image_viewer->getImage()->getParasits()->at(i)->getGT(),0,80);

            QBrush b = painter.brush();
            painter.setBrush(b);

            int x =  image_viewer->getImage()->getParasits()->at(i)->getImageX();
            int y = image_viewer->getImage()->getParasits()->at(i)->getImageY();

            painter.setBrush(QBrush(fillColor.dark(100)));
            painter.drawRect(QRect(x, y, image_viewer->getImage()->getParasits()->at(i)->getSize(), image_viewer->getImage()->getParasits()->at(i)->getSize()));

            painter.drawText(x+1,y+11,QString("parasite_") + QString::number( image_viewer->getImage()->getParasits()->at(i)->getId()));
        }

        newImage.save(filename);
    }
}

void MovableUI::selectNextBloodImage()
{
    selectBloodImage(image_selected+1);
}

void MovableUI::selectPreviousBloodImage()
{
    selectBloodImage(image_selected-1);
}

void MovableUI::selectBloodImage(int id)
{
    if(id >= 0 && id < simulation->getBloodImages()->length())
    {
        image_selected = id;
        image_viewer->loadScene(simulation->getBloodImages()->at(image_selected));

        list_parasits_viewer->clear();

        for(int i = 0; i < simulation->getBloodImages()->at(image_selected)->getParasits()->size(); i++)
            list_parasits_viewer->addItem(new QListWidgetItem(QIcon(simulation->getBloodImages()->at(image_selected)->getParasits()->at(i)->getView()), ""));

        selectParasit(0);

        infos->setText(simulation->getInfos(image_selected));
    }
}

void MovableUI::selectParasit(int id)
{
    if(id >= 0 && id < simulation->getBloodImages()->at(image_selected)->getParasits()->length())
    {
        //Selection of paraist in the list and in the image
        parasit_selected = id;
        image_viewer->centerOn(id);
    }
}

void MovableUI::createActions()
{
    newAct = new QAction(tr("&New ..."), this);
    newAct->setShortcut(tr("Ctrl+N"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(create()));

    openAct = new QAction(tr("&Open ..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    exitAct = new QAction(tr("&Exit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    printAct = new QAction(tr("&Print ..."), this);
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    connect(list_images_viewer, SIGNAL(currentRowChanged(int)), this, SLOT(selectBloodImage(int)));
    connect(list_parasits_viewer, SIGNAL(currentRowChanged(int)), this, SLOT(selectParasit(int)));

    connect(image_viewer->save_image, SIGNAL(clicked()), this, SLOT(save()));

    connect(image_viewer->previous_image, SIGNAL(clicked()), this, SLOT(selectPreviousBloodImage()));
    connect(image_viewer->next_image, SIGNAL(clicked()), this, SLOT(selectNextBloodImage()));
}

void MovableUI::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(printAct);

    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(helpMenu);
}
