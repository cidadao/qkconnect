#ifndef CLHANDLER_H
#define CLHANDLER_H

#include <QObject>
#include <QCommandLineParser>

#define _return() {emit done(); return;}

class QCoreApplication;
class QkConnectServer;

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
    void _slotDataOut(QByteArray data);

private:
    void _showHelp(const QCommandLineParser &parser);

    QCoreApplication *_app;
    QThread *thread;
    QkConnectServer *server;

};

#endif // CLHANDLER_H
