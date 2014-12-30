#include "qkconn.h"

QkConn::QkConn(QObject *parent) :
    QObject(parent)
{
    _status = Closed;
}

QkConn::Status QkConn::currentStatus()
{
    return _status;
}

void QkConn::_changeStatus(Status status)
{
    _status = status;
    emit statusChanged();
}
