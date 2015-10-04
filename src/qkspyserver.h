#ifndef QKSPYSERVER_H
#define QKSPYSERVER_H

#include "qkserver.h"
#include <QJsonDocument>

class QkSpyServer : public QkServer
{
    Q_OBJECT
public:
    explicit QkSpyServer(QString ip, int port, QObject *parent = 0);

signals:

public slots:
    void _slotClientConnected(int socketDesc);
    void _slotClientDisconnected(int socketDesc);
    void sendFromClient(QJsonDocument doc);
    void sendFromConn(QJsonDocument doc);
    void sendStatus(QJsonDocument doc);

protected slots:
    void handleDataIn(int socketDesc, QByteArray data);

};

#endif // QKSPYSERVER_H
