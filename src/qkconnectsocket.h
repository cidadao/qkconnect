#ifndef QKCONNECTSOCKET_H
#define QKCONNECTSOCKET_H

#include <QTcpSocket>

class QkConnectSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit QkConnectSocket(QObject *parent = 0);

signals:
    void dataIn(QByteArray);

public slots:
    void sendData(QByteArray data);

private slots:
    void _slotReadyRead();

};

#endif // QKCONNECTSOCKET_H
