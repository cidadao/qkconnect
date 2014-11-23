#include "qkconnectsocket.h"

#include <QDebug>

QkConnectSocket::QkConnectSocket(QObject *parent) :
    QTcpSocket(parent)
{

    connect(this, SIGNAL(readyRead()), this, SLOT(_slotReadyRead()));
}

void QkConnectSocket::_slotReadyRead()
{
    emit dataIn(readAll());
}
