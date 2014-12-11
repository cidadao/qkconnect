#ifndef QKCONNECTSERVER_H
#define QKCONNECTSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QMap>


class QkConnectClientThread;

class Broker : public QObject
{
    Q_OBJECT
public:
    explicit Broker(QObject *parent = 0);
    ~Broker();
    QMap<int, QkConnectClientThread*> threads;

signals:
    void dataToClient(QByteArray);
    void dataToConn(QByteArray data);

public slots:
    void handleDataIn(int socketDescriptor, QByteArray data);
    void sendData(QByteArray data);
};

class QkConnectServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit QkConnectServer(QString ip, int port, QObject *parent = 0);

protected:
    void incomingConnection(qintptr socketDescriptor);

signals:
    void message(int,QString);
    void dataToClient(QByteArray);
    void dataToConn(QByteArray);

public slots:
    void run();
    void sendData(QByteArray data);

private slots:
    void _slotClientDisconnected(int socketDescriptor);

private:
    Broker *_broker;
    QString _ip;
    int _port;

};

#endif // QKCONNECTSERVER_H
