#ifndef QKSERVER_H
#define QKSERVER_H

#include <QTcpServer>
#include <QMutex>
class QkClientThread;

class QkServer : public QTcpServer
{
    Q_OBJECT
public:
    enum Status
    {
        Disconnected,
        FailedToConnect,
        Connected
    };

    explicit QkServer(QString ip, int port, QObject *parent = 0);
    ~QkServer();

    Status currentStatus();

protected:
    void incomingConnection(qintptr socketDesc);
    virtual void run();

signals:
    void statusChanged();
    void message(int,QString);
    void dataIn(QByteArray);
    void dataOut(QByteArray);
    void killed();

public slots:
    void create();
    void kill();

protected slots:
    void _slotClientConnected(int socketDesc);
    void _slotClientDisconnected(int socketDesc);
    virtual void handleDataIn(int socketDesc, QByteArray data) = 0;

protected:
    void _changeStatus(Status status);

    QString _ip;
    int _port;
    QMap<int, QkClientThread*> _threads;
    Status _status;
    QMutex _mutex;
    bool _alive;

};

#endif // QKSERVER_H
