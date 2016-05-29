#include <QApplication>
#include <QCommandLineParser>

#include "movable_ui.h"
#include "model_blood_image.h"

//#include "dialog_new_simulation.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
//    FindDialog dialog;
//    dialog.show();
    QGuiApplication::setApplicationDisplayName(MovableUI::tr("Image Viewer"));

    QCommandLineParser commandLineParser;
    commandLineParser.addHelpOption();
    commandLineParser.addPositionalArgument(MovableUI::tr("[file]"), MovableUI::tr("Image file to open."));
    commandLineParser.process(QCoreApplication::arguments());

    MovableUI movable;
    if (!commandLineParser.positionalArguments().isEmpty()
        && !movable.loadFile(commandLineParser.positionalArguments().front()))
    {
        return -1;
    }

    movable.show();

    return app.exec();
}
