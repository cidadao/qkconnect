#ifndef QKCONNECTSERVER_H
#define QKCONNECTSERVER_H

#include "qkserver.h"
#include "qkcore.h"
#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

class QkConnectServer : public QkServer
{
    Q_OBJECT
public:
    explicit QkConnectServer(QString ip, int port, QObject *parent = 0);

    void setParseMode(bool parse);


public slots:
    void sendData(QByteArray data);

protected slots:
    void _slotClientConnected(int socketDesc);
    void _slotClientDisconnected(int socketDesc);
    void handleDataIn(int socketDesc, QByteArray data);
    void handleFrameIn(QByteArray frame, bool raw);
    void handleFrameOut(QByteArray frame, bool raw);

protected:
    void run();

    bool _parseMode;
    bool _frameReceived;
    Qk::Protocol *_protocolIn;
    Qk::Protocol *_protocolOut;
    QQueue<QByteArray> _framesInQueue;
    QWaitCondition _waitInputFrame;
    QList<QByteArray> _fragments;

};

#endif // QKCONNECTSERVER_H
