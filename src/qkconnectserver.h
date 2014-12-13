#ifndef QKCONNECTSERVER_H
#define QKCONNECTSERVER_H

#include "qkserver.h"
#include <QObject>

class QkConnectServer : public QkServer
{
    Q_OBJECT
public:
    explicit QkConnectServer(QString ip, int port, QObject *parent = 0);
public slots:
    void sendData(QByteArray data);

protected slots:
    void _slotClientConnected(int socketDesc);
    void _slotClientDisconnected(int socketDesc);
    void handleDataIn(int socketDesc, QByteArray data);

};

#endif // QKCONNECTSERVER_H
