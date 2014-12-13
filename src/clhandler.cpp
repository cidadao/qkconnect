#include "clhandler.h"
#include "qkconnect_global.h"
#include "qkconnectserver.h"
#include "qkspyserver.h"
#include "qkconn.h"
#include "qkconnloopback.h"
#include "qkconnserial.h"
#include "qkconnthread.h"

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
    parser.addPositionalArgument("conn", QCoreApplication::translate("main", "Connection type"));
    parser.addPositionalArgument("params", QCoreApplication::translate("main", "Connection parameters"));

    QCommandLineOption optionHelp(QStringList() << "h" << "help",
                                  QCoreApplication::translate("main", "Show help."));
    parser.addOption(optionHelp);

    parser.process(*_app);

    if(parser.isSet(optionHelp)) { _showHelp(parser); }

    const QStringList args = parser.positionalArguments();

    if(args.count() < 3)
    {
        qDebug() << "ERROR: Invalid number of arguments";
        exit(1);
    }

    QString serverIP = args.at(0);
    int serverPort = args.at(1).toInt();
    int spyServerPort = serverPort + 1;
    QString connType = args.at(2);
    QStringList connParams = args.mid(3);

    qDebug("--------------------------------------");
    qDebug(" %s v%s  |  qkthings.com", _app->applicationName().toStdString().c_str()
                                     , _app->applicationVersion().toStdString().c_str());
    qDebug("--------------------------------------");

    qDebug("Server:     %s %d (spy:%d)",
            serverIP.toStdString().c_str(),
            serverPort, spyServerPort);
    qDebug("Connection: %s", connType.toStdString().c_str());
    qDebug() << "Parameters:" << connParams;

    connThread = new QThread(this);
    QkConn::Descriptor connDesc;
    if(connType == "loopback")
    {
        conn = new QkConnLoopback();
    }
    else if(connType == "serial")
    {
        if(connParams.count() != 2)
        {
            qDebug() << "ERROR: Invalid number of parameters";
            exit(1);
        }
        connDesc.params["portName"] = connParams.at(0);
        connDesc.params["baudRate"] = connParams.at(1);
        conn = new QkConnSerial(connDesc);
    }
    else
    {
        qDebug() << "ERROR: invalid connection type";
        exit(1);
    }

    conn->moveToThread(connThread);
    connect(connThread, SIGNAL(started()), conn, SLOT(open()));
    connect(connThread, SIGNAL(finished()), conn, SLOT(deleteLater()));
    connect(connThread, SIGNAL(finished()), connThread, SLOT(deleteLater()));
    connect(conn, SIGNAL(message(int,QString)), this, SLOT(_slotMessage(int,QString)), Qt::DirectConnection);

    connThread->start();

    connectServerThread = new QThread(this);
    connectServer = new QkConnectServer(serverIP, serverPort);

    connectServer->moveToThread(connectServerThread);

    connect(connectServerThread, SIGNAL(started()), connectServer, SLOT(run()));
    connect(connectServerThread, SIGNAL(finished()), connectServer, SLOT(deleteLater()));
    connect(connectServerThread, SIGNAL(finished()), connectServerThread, SLOT(deleteLater()));

    connect(connectServer, SIGNAL(dataIn(QByteArray)), conn, SLOT(sendData(QByteArray)));
    connect(conn, SIGNAL(dataIn(QByteArray)), connectServer, SLOT(sendData(QByteArray)));


    connectServerThread->start();

    spyServerThread = new QThread(this);
    spyServer = new QkSpyServer(serverIP, spyServerPort);

    spyServer->moveToThread(spyServerThread);

    connect(spyServerThread, SIGNAL(started()), spyServer, SLOT(run()));
    connect(spyServerThread, SIGNAL(finished()), spyServer, SLOT(deleteLater()));
    connect(spyServerThread, SIGNAL(finished()), spyServerThread, SLOT(deleteLater()));
    connect(spyServer, SIGNAL(message(int,QString)), this, SLOT(_slotMessage(int,QString)), Qt::DirectConnection);

    connect(connectServer, SIGNAL(dataIn(QByteArray)), spyServer, SLOT(sendFromClient(QByteArray)));
    connect(connectServer, SIGNAL(dataOut(QByteArray)), spyServer, SLOT(sendFromConn(QByteArray)));
    connect(connectServer, SIGNAL(dataIn(QByteArray)), this, SLOT(_slotDataToConn(QByteArray)), Qt::DirectConnection);
    connect(connectServer, SIGNAL(dataOut(QByteArray)), this, SLOT(_slotDataToClient(QByteArray)),Qt::DirectConnection);
    connect(connectServer, SIGNAL(message(int,QString)), this, SLOT(_slotMessage(int,QString)), Qt::DirectConnection);

    spyServerThread->start();

    qDebug() << "Type 'quit' to quit";

    while(true)
    {
        QString inputText = cin.readLine();

        if(!inputText.isEmpty())
        {
            if(inputText == "quit")
                break;
            else
            {
                cout << inputText << "\n";
                cout.flush();
            }
        }
    }

    connThread->quit();
    connThread->wait();

    connectServerThread->quit();
    connectServerThread->wait();

    spyServerThread->quit();
    spyServerThread->wait();

    _return();
}

void CLHandler::_slotDataToConn(QByteArray data)
{
    cout << "--> " <<  data << "\n";
    cout.flush();
}

void CLHandler::_slotDataToClient(QByteArray data)
{
    cout << "<-- " <<  data << "\n";
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
    qDebug() << "[conn]           [params]";
    qDebug() << "loopback";
    qDebug() << "serial           <portname> <baudrate>";
    qDebug() << "";
    exit(0);
}

