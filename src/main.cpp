#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTimer>
#include <QDateTime>

#include "qkconnect_global.h"
#include "clhandler.h"

QTextStream cout(stdout);
QTextStream cin(stdin);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("QkConnect");
    QCoreApplication::setApplicationVersion(QDateTime::currentDateTime().date().toString("yyyyMMdd"));

    CLHandler clhandler(&a);

    QObject::connect(&clhandler, SIGNAL(done()), &a, SLOT(quit()));
    QTimer::singleShot(0, &clhandler, SLOT(run()));

    int exitCode = a.exec();
    qDebug() << "Exit" << exitCode;
    return exitCode;
}
