#include "qkserver.h"
#include "qkclientthread.h"
#include "qkconnect_global.h"

#include <QDebug>

QkServer::QkServer(QString ip, int port, QObject *parent) :
    QTcpServer(parent)
{
    _ip = ip;
    _port = port;
}

QkServer::~QkServer()
{
    foreach(QkClientThread *thread, _threads)
    {
        thread->quit();
        thread->wait();
    }
}

void QkServer::run()
{
    QHostAddress hostAddress;
    if(_ip.toLower() == "localhost")
        hostAddress = QHostAddress::LocalHost;
    else
        hostAddress.setAddress(_ip);

    if(listen(hostAddress, _port))
    {
        emit message(QKCONNECT_MESSAGE_INFO, "Listening on port "+ QString::number(_port));
    }
    else
    {
        emit message(QKCONNECT_MESSAGE_ERROR, errorString());
        exit(1);
    }
}

void QkServer::incomingConnection(qintptr socketDesc)
{
    QkClientThread *thread = new QkClientThread(this, socketDesc, this);

    connect(thread, SIGNAL(dataIn(int,QByteArray)), this, SLOT(handleDataIn(int,QByteArray)));

    connect(thread, SIGNAL(clientConnected(int)), this, SLOT(_slotClientConnected(int)));
    connect(thread, SIGNAL(clientDisconnected(int)), this, SLOT(_slotClientDisconnected(int)));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(message(int,QString)), this, SIGNAL(message(int,QString)));

    _threads.insert(socketDesc, thread);
    thread->start();
}

void QkServer::_slotClientConnected(int socketDesc)
{

}

void QkServer::_slotClientDisconnected(int socketDesc)
{
    if(_threads.remove(socketDesc) == 0)
        emit message(QKCONNECT_MESSAGE_ERROR,
                     QString().sprintf("Failed to remove socket descriptor %d", socketDesc));
}

