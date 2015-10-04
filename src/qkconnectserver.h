#ifndef QKCONNECTSERVER_H
#define QKCONNECTSERVER_H

#include "qkserver.h"
#include "qkcore.h"
#include "qkutils.h"
#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

class QkConnectServer : public QkServer
{
    Q_OBJECT
public:
    enum Options
    {
        joinFragments = (1<<0)
    };

    explicit QkConnectServer(QString ip, int port, QObject *parent = 0);

    void setParseMode(bool parse);
    void setOptions(int options);

signals:
    void jsonIn(QJsonDocument);
    void jsonOut(QJsonDocument);

public slots:
    void sendJson(QJsonDocument doc);
    void sendData(QByteArray data);

protected slots:
    void _slotClientConnected(int socketDesc);
    void _slotClientDisconnected(int socketDesc);
    void handleDataIn(int socketDesc, QByteArray data);
    void handleFrameIn(QByteArray frame, bool raw);
    void handleFrameOut(QByteArray frame, bool raw);
    void handleJsonIn(QJsonDocument doc);

protected:
    void run();

    QkUtils::JsonParser _jsonParser;
    bool _parseMode;
    bool _ackReceived;
    Qk::Protocol *_protocolIn;
    Qk::Protocol *_protocolOut;
    QQueue<QByteArray> _framesInQueue;
    QWaitCondition _waitInputFrame;
    QList<QByteArray> _fragments;
    int _options;

};

#endif // QKCONNECTSERVER_H
