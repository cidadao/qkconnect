#ifndef CLHANDLER_H
#define CLHANDLER_H

#include <QObject>
#include <QCommandLineParser>
#include <QMutex>
#include <QJsonDocument>

#include "qkconn.h"
#include "qkserver.h"

#define _return() {emit done(); return;}
#define _exit(code) {qApp->exit(code); return;}

class QCoreApplication;
class QkConnectServer;
class QkSpyServer;
class QkConn;
class QkConnThread;
class QMutex;

extern QTextStream cout;

class CLHandler : public QObject
{
    Q_OBJECT
public:
    explicit CLHandler(QCoreApplication *app, QObject *parent = 0);

signals:
    void jsonOut(QJsonDocument);
    void done();
    void aboutToQuit();

public slots:
    void run();
    void requestToQuit();
    void _slotMessage(int type, QString message, bool timestamp = true);
    void _slotDataToClient(QJsonDocument doc);
    void _slotDataToConn(QJsonDocument doc);
    void _slotStatus(QJsonDocument doc);

private:
    void _clHelp();
    void _showHelp(const QCommandLineParser &parser);
    void _quitThreads();
    QkConn::Status _waitConnReady(QkConn *conn);
    QkServer::Status _waitServerReady(QkServer *server);

    QCoreApplication *_app;
    QThread *connectServerThread;
    QThread *spyServerThread;
    QThread *connThread;
    QkConnectServer *connectServer;
    QkSpyServer *spyServer;
    QkConn *conn;
    QMutex _mutex;
    bool _verbose;
    bool _no_cli;
    bool _quit;

};

#endif // CLHANDLER_H
