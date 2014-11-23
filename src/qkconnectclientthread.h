#ifndef QKCONNECTCLIENTTHREAD_H
#define QKCONNECTCLIENTTHREAD_H

#include <QThread>

class QkConnectSocket;

class QkConnectClientThread : public QThread
{
    Q_OBJECT
public:
    explicit QkConnectClientThread(int socketDescriptor, QObject *parent = 0);

    QkConnectSocket* socket();

protected:
    void run();

signals:
    void message(int,QString);
    void dataIn(QByteArray);
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


};

#endif // QKCONNECTCLIENTTHREAD_H
