#ifndef QKSOCKET_H
#define QKSOCKET_H

#include <QTcpSocket>

class QkSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit QkSocket(QObject *parent = 0);

signals:
    void dataIn(QByteArray);

public slots:
    void sendData(QByteArray data);

private slots:
    void _slotReadyRead();

};

#endif // QKSOCKET_H
