#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTimer>
#include <QDateTime>

#include "qkconnect_global.h"
#include "clhandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("QkConnect");
    QCoreApplication::setApplicationVersion(QDate::fromString(__DATE__,"MMM dd yyyy").toString("yyyyMMdd"));

    CLHandler clhandler(&a);
    QTimer::singleShot(0, &clhandler, SLOT(run()));

    int exitCode = a.exec();
    return exitCode;
}
