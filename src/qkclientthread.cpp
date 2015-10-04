#include "qkclientthread.h"
#include "qksocket.h"
#include "qkconnect_global.h"
#include "qkserver.h"
#include "qkutils.h"

#include <QDebug>
#include <QHostAddress>

using namespace QkUtils;

QkClientThread::QkClientThread(QkServer *server, int socketDesc, QObject *parent) :
    QThread(parent)
{
    _socketDescriptor = socketDesc;
    _socket = 0;
    _server = server;
}

QkSocket* QkClientThread::socket()
{
    return _socket;
}

void QkClientThread::run()
{
    QkSocket socket;
    _socket = &socket;

    if(!_socket->setSocketDescriptor(_socketDescriptor))
    {
        emit message(QkConnect::MESSAGE_TYPE_ERROR, _socket->errorString());
        return;
    }

    emit clientConnected(_socketDescriptor);

    connect(_server, SIGNAL(dataOut(QByteArray)), _socket, SLOT(sendData(QByteArray)));
    connect(_socket, SIGNAL(dataIn(QByteArray)), this, SLOT(handleDataIn(QByteArray)), Qt::DirectConnection);
    connect(_socket, SIGNAL(disconnected()), this, SLOT(_slotDisconnected()), Qt::DirectConnection);

    exec();
}

void QkClientThread::sendData(QByteArray data)
{
    qDebug() << __PRETTY_FUNCTION__;
    if(_socket->isOpen() && !data.isEmpty())
        _socket->write(data);
}

void QkClientThread::handleDataIn(QByteArray data)
{
    emit dataIn(_socketDescriptor, data);
}

void QkClientThread::_slotDisconnected()
{
//    qDebug() << __PRETTY_FUNCTION__;
    emit clientDisconnected(_socketDescriptor);
}
