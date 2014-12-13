#ifndef QKSPYSERVER_H
#define QKSPYSERVER_H

#include "qkserver.h"

class QkSpyServer : public QkServer
{
    Q_OBJECT
public:
    explicit QkSpyServer(QString ip, int port, QObject *parent = 0);

signals:

public slots:
    void _slotClientConnected(int socketDesc);
    void _slotClientDisconnected(int socketDesc);
    void sendFromClient(QByteArray data);
    void sendFromConn(QByteArray data);

protected slots:
    void handleDataIn(int socketDesc, QByteArray data);

};

#endif // QKSPYSERVER_H
