#ifndef CLHANDLER_H
#define CLHANDLER_H

#include <QObject>
#include <QCommandLineParser>

#define _return() {emit done(); return;}

class QCoreApplication;
class QkConnectServer;
class QkConn;
class QkConnThread;

class CLHandler : public QObject
{
    Q_OBJECT
public:
    explicit CLHandler(QCoreApplication *app, QObject *parent = 0);

signals:
    void done();

public slots:
    void run();
    void _slotMessage(int type, QString message);
    void _slotDataToClient(QByteArray data);
    void _slotDataToConn(QByteArray data);

private:
    void _showHelp(const QCommandLineParser &parser);

    QCoreApplication *_app;
    QThread *serverThread;
    QThread *connThread;
    QkConnectServer *server;
    QkConn *conn;
//    QkConnThread *connThread;

};

#endif // CLHANDLER_H
