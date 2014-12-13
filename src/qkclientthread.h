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

private slots:
    void _slotDataIn(QByteArray data);
    void _slotDisconnected();

private:
    int _socketDescriptor;
    QkSocket *_socket;
    QkServer *_server;


};

#endif // QKCLIENTTHREAD_H
