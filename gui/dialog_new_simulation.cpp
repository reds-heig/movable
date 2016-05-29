#include <QtWidgets>

#include "dialog_new_simulation.h"

FindDialog::FindDialog(QString *simulation_file, QWidget *parent) : QDialog(parent)
{
    parent_simulation_file = simulation_file;

    setModal(true);

    simulation_name = new QLineEdit("untitled");
    simulation_path = new QLineEdit(
                QStandardPaths::standardLocations(
                    QStandardPaths::DocumentsLocation).first());
    images_path = new QLineEdit;
    masks_path = new QLineEdit;
    classifier_path = new QLineEdit;

    btn_browse_simulation = new QPushButton("...");
    btn_browse_images = new QPushButton("...");
    btn_browse_masks = new QPushButton("...");
    btn_browse_classifier = new QPushButton("...");

    btn_cancel = new QPushButton("Cancel");
    btn_create = new QPushButton("Create");

    QVBoxLayout *layout = new QVBoxLayout();

    /* Configuration of simulation */
    QGridLayout *config_layout = new QGridLayout();
    config_layout->addWidget(new QLabel("Simulation's name :"),0,0);
    config_layout->addWidget(simulation_name,0,1);
    config_layout->addWidget(new QLabel("Save as ..."),1,0);
    config_layout->addWidget(simulation_path,1,1);
    config_layout->addWidget(btn_browse_simulation,1,2);
    config_layout->addWidget(new QLabel("Path of image's directory :"),2,0);
    config_layout->addWidget(images_path,2,1);
    config_layout->addWidget(btn_browse_images,2,2);
    config_layout->addWidget(new QLabel("Path of mask's directory :"),3,0);
    config_layout->addWidget(masks_path,3,1);
    config_layout->addWidget(btn_browse_masks,3,2);
    config_layout->addWidget(new QLabel("Path of classifier :"),4,0);
    config_layout->addWidget(classifier_path,4,1);
    config_layout->addWidget(btn_browse_classifier,4,2);

    /* Push buttons */
    QHBoxLayout *btn_layout = new QHBoxLayout;
    btn_layout->addStretch();
    btn_layout->addWidget(btn_cancel);
    btn_layout->addWidget(btn_create);

    layout->addLayout(config_layout);
    layout->addLayout(btn_layout);

    setLayout(layout);

    connect(btn_browse_simulation, SIGNAL(clicked()), this, SLOT(browseSimulation()));
    connect(btn_browse_images, SIGNAL(clicked()), this, SLOT(browseImages()));
    connect(btn_browse_masks, SIGNAL(clicked()), this, SLOT(browseMasks()));
    connect(btn_browse_classifier, SIGNAL(clicked()), this, SLOT(browseClassifier()));

    connect(btn_cancel, SIGNAL(clicked()), this, SLOT(cancelSimulation()));
    connect(btn_create, SIGNAL(clicked()), this, SLOT(createSimulation()));
}

void FindDialog::cancelSimulation() {
    parent_simulation_file->clear();
    this->setResult(QDialog::Rejected);
    this->close();
}

void FindDialog::createSimulation() {

    if(simulation_name->text().isEmpty() ||
            simulation_path->text().isEmpty() ||
            images_path->text().isEmpty() ||
            masks_path->text().isEmpty() ||
            classifier_path->text().isEmpty()) {
        qDebug() << "Missing paths ...";
        return;
    }

    qDebug() << "Create simulation ...";

    /* Create directory of project */
    QDir project_dir(simulation_path->text());
    project_dir.mkdir(simulation_name->text());
    project_dir.cd(simulation_name->text());

    qDebug() << "Simulation's project :" << project_dir.absolutePath();

    /* Create directory of results */
    project_dir.mkdir("results_test");

    qDebug() << "Results of analysis :" << project_dir.absolutePath().append("/results_test");

    /* Get paths of dataset */
    QDir images_dir(images_path->text());
    QStringList images = images_dir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
    for(int i=0; i < images.size(); i++) {
        images.replace(i, images_dir.absolutePath().append("/").append(images.at(i)));
    }
    qDebug() << "Images : \n" << images;

    QDir masks_dir(masks_path->text());
    QStringList masks = masks_dir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
    for(int i=0; i < masks.size(); i++) {
        masks.replace(i, masks_dir.absolutePath().append("/").append(masks.at(i)));
    }
    qDebug() << "Masks : \n" << masks;

    /* Create config file */
    QFile config_file(project_dir.absolutePath().append("/config.json"));
    qDebug() << "File of configuration : " << project_dir.absolutePath().append("/config.json");

    if (!config_file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    /* File : config.json */
    QTextStream out_config_file(&config_file);
    out_config_file << "{" << endl;
    out_config_file << "\"resultsDir\" : \"" << project_dir.absolutePath().append("\results_test") << "\"," << endl;
    out_config_file << "\"datasetPath\": \"" << project_dir.absolutePath() << "\"," << endl;
    out_config_file << "\"datasetName\": \"" << simulation_name->text() << "\"," << endl;
    out_config_file << "\"imgPathsFName\": \"test_imgs.txt\"," << endl;
    out_config_file << "\"maskPathsFName\": \"test_masks.txt\"," << endl;
    out_config_file << "\"sampleSize\": 51," << endl;
    out_config_file << "\"channelList\": [" << endl;
    out_config_file << "\"IMAGE_GRAY_CH\"," << endl;
    out_config_file << "\"IMAGE_GREEN_CH\"," << endl;
    out_config_file << "\"IMAGE_RED_CH\"," << endl;
    out_config_file << "\"MEDIAN_FILTERING\"," << endl;
    out_config_file << "\"LAPLACIAN_FILTERING\"," << endl;
    out_config_file << "\"GAUSSIAN_FILTERING\"," << endl;
    out_config_file << "\"SOBEL_DRV_X\"," << endl;
    out_config_file << "\"SOBEL_DRV_Y\"" << endl;
    out_config_file << "]" << endl;
    out_config_file << "}" << endl;

    out_config_file.flush();
    config_file.close();

    /* Create test_imgs file */
    QFile test_imgs_file(project_dir.absolutePath().append("/test_imgs.txt"));
    qDebug() << "test_imgs.txt : " << project_dir.absolutePath().append("/test_imgs.txt");

    if (!test_imgs_file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out_test_imgs_file(&test_imgs_file);
    foreach(QString img, images) {
        out_test_imgs_file << img << endl;
    }
    out_test_imgs_file.flush();
    test_imgs_file.close();

    /* Create test_masks file */
    QFile test_masks_file(project_dir.absolutePath().append("/test_masks.txt"));
    qDebug() << "test_masks.txt : " << project_dir.absolutePath().append("/test_masks.txt");

    if (!test_masks_file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out_test_masks_file(&test_masks_file);
    foreach(QString mask, masks) {
        out_test_masks_file << mask << endl;
    }
    out_test_masks_file.flush();
    test_masks_file.close();

    /* Call classifier */
    qDebug() << "Call classifier... " << QFile::copy("/home/mylag/Documents/1412151257-100x-0012_segm.png", project_dir.absolutePath().append("/results_test/1412151257-100x-0012_segm.png"));
//    QProcess *process = new QProcess(this);
//    QString program = "../../build/src/test/test_movable";
//    QString params = "";
//    process->start(program, QStringList() << params);

    /* Get paths of results */
    QDir results_dir(project_dir.absolutePath().append("/results_test"));
    QStringList results = results_dir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
    for(int i=0; i < results.size(); i++) {
        results.replace(i, results_dir.absolutePath().append("/").append(results.at(i)));
    }
    qDebug() << "Results of analysis : \n" << results;

    /* Create .sim file */
    QFile simulation_file(project_dir.absolutePath().append("/").append(simulation_name->text()).append(".sim"));
    qDebug() << "*.sim file : " << project_dir.absolutePath().append("/").append(simulation_name->text()).append(".sim");

    parent_simulation_file->clear();
    parent_simulation_file->append(project_dir.absolutePath().append("/").append(simulation_name->text()).append(".sim"));

    if (!simulation_file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out_simulation_file(&simulation_file);
    out_simulation_file << "[absolute_path]" << endl;
    out_simulation_file << "[imgs]" << endl;
    foreach(QString img, images) {
        out_simulation_file << img << endl;
    }

    out_simulation_file << "[gts]" << endl;
    foreach(QString result, results) {
        out_simulation_file << result << endl;
    }

    out_simulation_file.flush();
    simulation_file.close();

    this->setResult(QDialog::Accepted);
    this->close();
}

void FindDialog::browseSimulation()
{
    QString directory =
            QFileDialog::getExistingDirectory(
                this,
                tr("Save as"), /*QDir::currentPath()*/
                QStandardPaths::standardLocations(
                    QStandardPaths::DocumentsLocation).first());

    if (!directory.isEmpty())
        simulation_path->insert(directory);
}

void FindDialog::browseImages()
{
    QString directory =
            QFileDialog::getExistingDirectory(
                this,
                tr("Browse images"), /*QDir::currentPath()*/
                QStandardPaths::standardLocations(
                    QStandardPaths::DocumentsLocation).first());

    if (!directory.isEmpty())
        images_path->insert(directory);
}

void FindDialog::browseMasks()
{
    QString directory =
            QFileDialog::getExistingDirectory(
                this,
                tr("Browse masks"), /*QDir::currentPath()*/
                QStandardPaths::standardLocations(
                    QStandardPaths::DocumentsLocation).first());

    if (!directory.isEmpty())
        masks_path->insert(directory);
}

void FindDialog::browseClassifier()
{
    QString file =
            QFileDialog::getOpenFileName(
                this,
                tr("Browse classifier"), /*QDir::currentPath()*/
                QStandardPaths::standardLocations(
                    QStandardPaths::DocumentsLocation).first());

    if (!file.isEmpty())
        classifier_path->insert(file);
}
