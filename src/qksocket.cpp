#include "qksocket.h"

#include <QDebug>

QkSocket::QkSocket(QObject *parent) :
    QTcpSocket(parent)
{

    connect(this, SIGNAL(readyRead()), this, SLOT(_slotReadyRead()));
}

void QkSocket::sendData(QByteArray data)
{
    write(data);
}

void QkSocket::_slotReadyRead()
{
    emit dataIn(readAll());
}
