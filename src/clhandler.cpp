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
#include <QTime>
#include <QTimer>
#include <QMutexLocker>
#include <QJsonDocument>

QTextStream cout(stdout, QIODevice::WriteOnly);
QTextStream cin(stdin, QIODevice::ReadOnly);

CLHandler::CLHandler(QCoreApplication *app, QObject *parent) :
    QObject(parent),
    _app(app)
{

    connThread = 0;
    connectServerThread = 0;
    spyServerThread = 0;
    _quit = false;
}

void CLHandler::run()
{
    QCommandLineParser parser;

    parser.addPositionalArgument("ip", QCoreApplication::translate("main", "Server's IP"));
    parser.addPositionalArgument("port", QCoreApplication::translate("main", "Server's port"));
    parser.addPositionalArgument("conn", QCoreApplication::translate("main", "Connection type"));
    parser.addPositionalArgument("params", QCoreApplication::translate("main", "Connection parameters"));

    QCommandLineOption optionHelp(QStringList() << "h" << "help",
                                  tr("Show help."));
    parser.addOption(optionHelp);

    QCommandLineOption optionVersion(QStringList() << "version",
                                     tr("Show version"));
    parser.addOption(optionVersion);

    QCommandLineOption optionVerbose(QStringList() << "verbose",
                                     tr("Verbose mode"));
    parser.addOption(optionVerbose);

    QCommandLineOption optionNoCLI(QStringList() << "no-cli", tr("No CLI (use TCP API instead)"));
    parser.addOption(optionNoCLI);

    QCommandLineOption optionParse(QStringList() << "p" << "parse",
                                   tr("Parse bytestream into packets."));
    parser.addOption(optionParse);

    QCommandLineOption optionJoinFragments(QStringList() << "j" << "join",
                                   tr("Join fragments (only if parse mode is enabled)."));
    parser.addOption(optionJoinFragments);

    QCommandLineOption optionListSerial(QStringList() << "list-serial",
                                        QCoreApplication::translate("main", "List available serial ports."));
    parser.addOption(optionListSerial);

    parser.process(*_app);

    if(parser.isSet(optionVersion))
    {
        cout << qApp->applicationVersion() << "\n";
        cout.flush();
        _exit(0);
    }
    if(parser.isSet(optionHelp)) { _showHelp(parser); _exit(0); }

    if(parser.isSet(optionListSerial))
    {
        QkConnSerial::listAvailable();
        _exit(0);
    }

    const QStringList args = parser.positionalArguments();

    if(args.count() < 3)
    {
        qDebug() << args;
        _slotMessage(QkConnect::MESSAGE_TYPE_ERROR, "Invalid number of arguments", false);
        _exit(1);
    }

    QString serverIP = args.at(0);
    int serverPort = args.at(1).toInt();
    int spyServerPort = serverPort + 1;
    QString connType = args.at(2);
    QStringList connParams = args.mid(3);
    bool parseMode = parser.isSet(optionParse);
    bool joinFragments = parser.isSet(optionJoinFragments);
    _verbose = parser.isSet(optionVerbose);
    _no_cli = parser.isSet(optionNoCLI);

    cout << "-------------------------------------------------\n";
    cout << " QkConnect v" << qApp->applicationVersion() << "\n";
    cout << "-------------------------------------------------\n";

    cout << "Server:     " << QString("%1 %2 (spy: %3)")
            .arg(serverIP)
            .arg(serverPort)
            .arg(spyServerPort) << "\n";

    cout << "Connection: " << connType << "\n";
    cout << "Parameters: ";
    foreach(QString param, connParams)
        cout << param << " ";
    cout << "\n";
    cout << "Parse:      " << parseMode << "\n";
    cout << "Join:       " << joinFragments << "\n";
    cout << "Verbose:    " << _verbose << "\n";
    cout << "No-CLI:     " << _no_cli << "\n";

    connectServerThread = new QThread(this);
    connectServer = new QkConnectServer(serverIP, serverPort);
    connectServer->setParseMode(parseMode);
    int options = 0;
    if(joinFragments)
        options |= QkConnectServer::joinFragments;
    connectServer->setOptions(options);

    connectServer->moveToThread(connectServerThread);

    connect(connectServerThread, SIGNAL(started()), connectServer, SLOT(create()));
    connect(connectServerThread, SIGNAL(finished()), connectServer, SLOT(deleteLater()));
    connect(connectServerThread, SIGNAL(finished()), connectServerThread, SLOT(deleteLater()));

    connect(connectServer, SIGNAL(message(int,QString)), this, SLOT(_slotMessage(int,QString)), Qt::DirectConnection);
    //connect(connectServer, SIGNAL(dataIn(QByteArray)), conn, SLOT(sendData(QByteArray)));
    //connect(conn, SIGNAL(dataIn(QByteArray)), connectServer, SLOT(sendData(QByteArray)));

    connectServerThread->start();
    if(_waitServerReady(connectServer) != QkServer::Connected)
    {
        _quitThreads();
        _exit(1);
    }

    spyServerThread = new QThread(this);
    spyServer = new QkSpyServer(serverIP, spyServerPort);

    spyServer->moveToThread(spyServerThread);

    connect(spyServerThread, SIGNAL(started()), spyServer, SLOT(create()));
    connect(spyServerThread, SIGNAL(finished()), spyServer, SLOT(deleteLater()));
    connect(spyServerThread, SIGNAL(finished()), spyServerThread, SLOT(deleteLater()));
    connect(spyServer, SIGNAL(message(int,QString)), this, SLOT(_slotMessage(int,QString)), Qt::DirectConnection);

    connect(connectServer, SIGNAL(jsonIn(QJsonDocument)), spyServer, SLOT(sendFromClient(QJsonDocument)));
    connect(connectServer, SIGNAL(jsonOut(QJsonDocument)), spyServer, SLOT(sendFromConn(QJsonDocument)));
    connect(connectServer, SIGNAL(jsonIn(QJsonDocument)), this, SLOT(_slotDataToConn(QJsonDocument)), Qt::DirectConnection);
    connect(connectServer, SIGNAL(jsonOut(QJsonDocument)), this, SLOT(_slotDataToClient(QJsonDocument)),Qt::DirectConnection);

    spyServerThread->start();
    if(_waitServerReady(spyServer) != QkServer::Connected)
    {
        _quitThreads();
        _exit(1);
    }

    connThread = new QThread(this);
    QkConn::Descriptor connDesc;
    if(connType == "loopback")
    {
        conn = new QkConnLoopback();
    }
    else if(connType == "serial")
    {
        if(connParams.count() != 3)
        {
            _slotMessage(QkConnect::MESSAGE_TYPE_ERROR, "Invalid number of parameters", false);
            _exit(1);
        }
        connDesc.params["portname"] = connParams.at(0);
        connDesc.params["baudrate"] = connParams.at(1);
        connDesc.params["dtr"] = connParams.at(2);
        conn = new QkConnSerial(connDesc);
    }
    else
    {
        _slotMessage(QkConnect::MESSAGE_TYPE_ERROR, "Invalid connection type", false);
        _exit(1);
    }

    conn->moveToThread(connThread);
    connect(connThread, SIGNAL(started()), conn, SLOT(open()));
    connect(connThread, SIGNAL(finished()), conn, SLOT(deleteLater()));
    connect(connThread, SIGNAL(finished()), connThread, SLOT(deleteLater()));
    connect(conn, SIGNAL(message(int,QString)), this, SLOT(_slotMessage(int,QString)), Qt::DirectConnection);

    connThread->start();
    if(_waitConnReady(conn) != QkConn::Ready)
    {
        _quitThreads();
        _exit(1);
    }

    connect(conn, SIGNAL(jsonIn(QJsonDocument)), connectServer, SLOT(sendJson(QJsonDocument)));
    connect(connectServer, SIGNAL(jsonIn(QJsonDocument)), conn, SLOT(sendJson(QJsonDocument)));

    connect(this, SIGNAL(jsonOut(QJsonDocument)), connectServer, SLOT(sendJson(QJsonDocument)));
    connect(this, SIGNAL(jsonOut(QJsonDocument)), spyServer, SLOT(sendStatus(QJsonDocument)));
    connect(this, SIGNAL(jsonOut(QJsonDocument)), this, SLOT(_slotStatus(QJsonDocument)));

    QEventLoop eventLoop;
    connect(this, SIGNAL(aboutToQuit()), &eventLoop, SLOT(quit()));

    cout << "\nReady.\nNeed help? Type 'help'\n";
    cout.flush();

    while(!_quit)
    {
//          cout << "> ";
//          cout.flush();
        QString inputText = cin.readLine();

        if(!inputText.isEmpty())
        {
            if(inputText == "quit")
                break;
            else if(inputText == "help")
            {
                _clHelp();
            }
            else
            {
                cout << "Unknown command: " << inputText << "\n";
                cout.flush();
            }
        }

    }

    _quitThreads();
    _exit(0);
}

void CLHandler::requestToQuit()
{
    _quit = true;
    emit aboutToQuit();
}

void CLHandler::_quitThreads()
{
    if(connThread != 0 &&
       connThread->isRunning())
    {
        connThread->quit();
        connThread->wait();
    }

    if(connectServerThread != 0 &&
       connectServerThread->isRunning())
    {
        connectServer->kill();
        connectServerThread->quit();
        connectServerThread->wait();
    }

    if(spyServerThread != 0 &&
       spyServerThread->isRunning())
    {
        spyServerThread->quit();
        spyServerThread->wait();
    }
}

QkConn::Status CLHandler::_waitConnReady(QkConn *conn)
{
    QEventLoop eventLoop;
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
    connect(conn, SIGNAL(statusChanged()), &eventLoop, SLOT(quit()));
    timer.start(5000);
    eventLoop.exec();
    disconnect(conn, SIGNAL(statusChanged()), &eventLoop, SLOT(quit()));
    return conn->currentStatus();
}

QkServer::Status CLHandler::_waitServerReady(QkServer *server)
{
    QEventLoop eventLoop;
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
    connect(server, SIGNAL(statusChanged()), &eventLoop, SLOT(quit()));
    timer.start(5000);
    eventLoop.exec();
    disconnect(server, SIGNAL(statusChanged()), &eventLoop, SLOT(quit()));
    return server->currentStatus();
}

void CLHandler::_slotDataToConn(QJsonDocument doc)
{
    if(_verbose)
    {
//        cout << "--> " <<  doc.toJson() << "\n";
//        cout.flush();
    }
}

void CLHandler::_slotDataToClient(QJsonDocument doc)
{
    if(_verbose)
    {
//        cout << "<-- " <<  doc.toJson() << "\n";
//        cout.flush();
    }
}

void CLHandler::_slotStatus(QJsonDocument doc)
{
    if(_verbose)
    {
        cout << "status:" << doc.toJson() << "\n";
        cout.flush();
    }
}

void CLHandler::_slotMessage(int type, QString message, bool timestamp)
{
//    qDebug() << __PRETTY_FUNCTION__ << message;
    QMutexLocker locker(&_mutex);

    QString timeStr = "(" + QTime::currentTime().toString("hh:mm:ss") + ") ";
    QString typeStr;
    switch(type)
    {
    case QkConnect::MESSAGE_TYPE_INFO: typeStr ="[i] "; break;
    case QkConnect::MESSAGE_TYPE_ERROR: typeStr = "[e] "; break;
    case QkConnect::MESSAGE_TYPE_DEBUG: typeStr = "[d] ";
        if(!_verbose)
            return;
        break;
    }

    QString line = QString("%1 %2 %3")
            .arg(timeStr).arg(typeStr).arg(message);

    if(!_no_cli)
    {
        cout << line << "\n";
        cout.flush();
    }
    else
    {
        QString json_str = "{";
        json_str += "\"type\": \"status\",";
        json_str += "\"message\": \"" + line + "\"";
        json_str += "}";
        QJsonDocument doc = QJsonDocument::fromJson(json_str.toUtf8());

        emit jsonOut(doc);
    }
}

void CLHandler::_showHelp(const QCommandLineParser &parser)
{
    qDebug("%s", parser.helpText().toStdString().c_str());
    qDebug() << "[conn]           [params]";
    qDebug() << "loopback";
    qDebug() << "serial           <portname> <baudrate> <dtr>";
    qDebug() << "";
}

void CLHandler::_clHelp()
{
    cout << "quit      Quit" << "\n";
    cout.flush();
}

