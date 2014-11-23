#ifndef QKCONNECTSERVER_H
#define QKCONNECTSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QMap>

class QkConnectClientThread;

class QkConnectServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit QkConnectServer(QString ip, int port, QObject *parent = 0);

protected:
    void incomingConnection(qintptr socket);

signals:
    void message(int,QString);
    void dataOut(QByteArray);

public slots:
    void run();

private slots:
    void _slotClientDisconnected(int socketDescriptor);

private:
    QMap<int, QkConnectClientThread*> _threads;
    QString _ip;
    int _port;

};

#endif // QKCONNECTSERVER_H
