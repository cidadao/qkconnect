#include "qkconnloopback.h"
#include "qkconnect_global.h"

#include <QDebug>

QkConnLoopback::QkConnLoopback(QObject *parent) :
    QkConn(parent)
{
}

bool QkConnLoopback::open()
{
    emit message(QKCONNECT_MESSAGE_INFO,
                 tr("Connection ready!"));
    return true;
}

void QkConnLoopback::close()
{

}

void QkConnLoopback::sendData(QByteArray data)
{
    emit dataIn(data);
}
