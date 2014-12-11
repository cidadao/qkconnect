#ifndef QKCONNECTCLIENTTHREAD_H
#define QKCONNECTCLIENTTHREAD_H

#include <QThread>

class QkConnectSocket;
class Broker;

class QkConnectClientThread : public QThread
{
    Q_OBJECT
public:
    explicit QkConnectClientThread(Broker *broker, int socketDescriptor, QObject *parent = 0);

    QkConnectSocket* socket();

protected:
    void run();

signals:
    void message(int,QString);
    void dataIn(int, QByteArray);
    void dataOut(QByteArray);
    void clientConnected(int socket);
    void clientDisconnected(int socket);

public slots:
    void sendData(QByteArray data);

private slots:
    void _slotDataIn(QByteArray data);
    void _slotDisconnected();

private:
    int _socketDescriptor;
    QkConnectSocket *_socket;
    Broker *_broker;


};

#endif // QKCONNECTCLIENTTHREAD_H
