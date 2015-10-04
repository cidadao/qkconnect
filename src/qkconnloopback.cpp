#include "qkconnloopback.h"
#include "qkconnect_global.h"

#include <QDebug>

QkConnLoopback::QkConnLoopback(QObject *parent) :
    QkConn(parent)
{
}

bool QkConnLoopback::open()
{
    emit message(QkConnect::MESSAGE_TYPE_INFO,
                 "conn.ready");
    _changeStatus(QkConn::Ready);
    return true;
}

void QkConnLoopback::close()
{

}

void QkConnLoopback::sendData(QByteArray data)
{
//    emit dataIn(data);
}

void QkConnLoopback::sendJson(QJsonDocument doc)
{
    emit jsonIn(doc);
}
