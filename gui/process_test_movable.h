#include <QtWidgets>
#include <QProcess>

class TestMovableProcess : public QObject
{
    Q_OBJECT
private:
    QProcess *process;

public:
    TestMovableProcess(QObject *parent = 0) : QObject(parent), process(new QProcess) {}

    void init(const QString &program) {
        connect(process,SIGNAL(readyRead()),this,SLOT(readStdOut()));
        connect(process,SIGNAL(started()),this,SLOT(onStarted()));
        connect(process,SIGNAL(finished(int)),this,SLOT(onFinished(int)));
        process->start(program);
    }

private slots:
    void readStdOut() {
        qDebug() << "[test_movable] : " << QString(process->readAllStandardOutput()).toUtf8().constData() << endl;
    }
    void onStarted(){
        qDebug() << "[test_movable] : started" << endl;
    }
    void onFinished(int signal) {
        qDebug() << "[test_movable] : finished " << signal << endl;
        qDebug() << "process->atEnd()" << process->atEnd();
    }
};
