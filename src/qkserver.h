#ifndef QKSERVER_H
#define QKSERVER_H

#include <QTcpServer>
class QkClientThread;

class QkServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit QkServer(QString ip, int port, QObject *parent = 0);
    ~QkServer();

protected:
    void incomingConnection(qintptr socketDesc);

signals:
    void message(int,QString);
    void dataIn(QByteArray);
    void dataOut(QByteArray);

public slots:
    void run();

protected slots:
    void _slotClientConnected(int socketDesc);
    void _slotClientDisconnected(int socketDesc);
    virtual void handleDataIn(int socketDesc, QByteArray data) = 0;

protected:
    QString _ip;
    int _port;
    QMap<int, QkClientThread*> _threads;


};

#endif // QKSERVER_H
