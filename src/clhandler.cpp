#include "clhandler.h"
#include "qkconnect_global.h"
#include "qkconnectserver.h"

#include <QDebug>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QHostAddress>
#include <QEventLoop>
#include <QThread>

CLHandler::CLHandler(QCoreApplication *app, QObject *parent) :
    QObject(parent),
    _app(app)
{


}

void CLHandler::run()
{
    QCommandLineParser parser;
    //parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("ip", QCoreApplication::translate("main", "Server's IP"));
    parser.addPositionalArgument("port", QCoreApplication::translate("main", "Server's port"));
    parser.addPositionalArgument("tech", QCoreApplication::translate("main", "Communication technology"));
    parser.addPositionalArgument("params", QCoreApplication::translate("main", "Communication parameters"));

    QCommandLineOption optionHelp(QStringList() << "h" << "help",
                                  QCoreApplication::translate("main", "Show help."));
    parser.addOption(optionHelp);

    parser.process(*_app);

    if(parser.isSet(optionHelp)) { _showHelp(parser); }

    const QStringList args = parser.positionalArguments();

    if(args.count() < 4)
    {
        qDebug() << "ERROR: Invalid number of arguments";
        exit(1);
    }

    QString serverIP = args.at(0);
    QString serverPort = args.at(1);
    QString commTech = args.at(2);
    QStringList commParams = args.mid(3);

    qDebug("%s %s | qkthings.com", _app->applicationName().toStdString().c_str()
                                 , _app->applicationVersion().toStdString().c_str());

    qDebug() << "Server:     " << serverIP << serverPort;
    qDebug() << "Connection: " << commTech << commParams;


    thread = new QThread();
    server = new QkConnectServer(serverIP, serverPort.toInt());

    server->moveToThread(thread);

    connect(thread, SIGNAL(started()), server, SLOT(run()));
    connect(thread, SIGNAL(finished()), server, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    connect(server, SIGNAL(dataOut(QByteArray)), this, SLOT(_slotDataOut(QByteArray)), Qt::DirectConnection);
    connect(server, SIGNAL(message(int,QString)), this, SLOT(_slotMessage(int,QString)), Qt::DirectConnection);

    thread->start();


    qDebug() << "Type 'quit' to quit";

    bool alive = true;

    while(alive)
    {
        //cout << "> "; cout.flush();
        QString inputText = cin.readLine();

        if(!inputText.isEmpty())
            cout << inputText << "\n";
        cout.flush();

        if(inputText == "quit")
            alive = false;
    }

    _return();
}

void CLHandler::_slotDataOut(QByteArray data)
{
    cout << data << "\n";
    cout.flush();
}

void CLHandler::_slotMessage(int type, QString message)
{
    switch(type)
    {
    case QKCONNECT_MESSAGE_INFO: cout << "[i] "; break;
    case QKCONNECT_MESSAGE_ERROR: cout << "[e] "; break;
    }

    cout << message << "\n";
    cout.flush();
}

void CLHandler::_showHelp(const QCommandLineParser &parser)
{
    qDebug("%s", parser.helpText().toStdString().c_str());
    qDebug() << "[tech]           [params]";
    qDebug() << "serial           <portname> <baudrate>";
    exit(0);
}

