#include "qkconnectserver.h"
#include "qkconnectclientthread.h"
#include "qkconnectsocket.h"
#include "qkconnect_global.h"

#include <QDebug>

QkConnectServer::QkConnectServer(QString ip, int port, QObject *parent) :
    QTcpServer(parent)
{
    _ip = ip;
    _port = port;
}

void QkConnectServer::run()
{
    QHostAddress hostAddress;
    if(_ip.toLower() == "localhost")
        hostAddress = QHostAddress::LocalHost;
    else
        hostAddress.setAddress(_ip);

    if(listen(hostAddress, _port))
    {
        emit message(QKCONNECT_MESSAGE_INFO, "Listening...");
    }
    else
    {
        emit message(QKCONNECT_MESSAGE_ERROR, errorString());
        exit(1);
    }
}

void QkConnectServer::incomingConnection(qintptr socket)
{
    QkConnectClientThread *thread = new QkConnectClientThread(socket, this);


    connect(thread, SIGNAL(dataIn(QByteArray)), this, SIGNAL(dataOut(QByteArray)), Qt::DirectConnection);


    connect(thread, SIGNAL(clientDisconnected(int)), this, SLOT(_slotClientDisconnected(int)));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(message(int,QString)), this, SIGNAL(message(int,QString)), Qt::DirectConnection);

    _threads.insert(socket, thread);
    thread->start();
}


void QkConnectServer::_slotClientDisconnected(int socketDescriptor)
{
    QkConnectSocket *socket = _threads.value(socketDescriptor)->socket();
    _threads.remove(socket->socketDescriptor());
}
