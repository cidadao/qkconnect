#ifndef QKCLIENTTHREAD_H
#define QKCLIENTTHREAD_H

#include <QThread>

class QkSocket;
class QkServer;

class QkClientThread : public QThread
{
    Q_OBJECT
public:
    explicit QkClientThread(QkServer *server, int socketDesc, QObject *parent = 0);

    QkSocket* socket();

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

protected slots:
    void _slotDisconnected();
    virtual void handleDataIn(QByteArray data);

protected:
    int _socketDescriptor;
    QkSocket *_socket;
    QkServer *_server;

};

#endif // QKCLIENTTHREAD_H
