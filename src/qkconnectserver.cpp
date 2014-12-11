#include "qkconnectserver.h"
#include "qkconnectclientthread.h"
#include "qkconnectsocket.h"
#include "qkconnect_global.h"

#include <QDebug>
#include <QEventLoop>

Broker::Broker(QObject *parent) :
    QObject(parent)
{

}

Broker::~Broker()
{
    foreach(QkConnectClientThread *thread, threads)
    {
        thread->quit();
        thread->wait();
    }
}

QkConnectServer::QkConnectServer(QString ip, int port, QObject *parent) :
    QTcpServer(parent)
{
    _ip = ip;
    _port = port;

    _broker = new Broker(this);

    connect(_broker, SIGNAL(dataToConn(QByteArray)), this, SIGNAL(dataToConn(QByteArray)));
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

void QkConnectServer::incomingConnection(qintptr socketDescriptor)
{
    QkConnectClientThread *thread = new QkConnectClientThread(_broker, socketDescriptor, this);

    connect(thread, SIGNAL(dataIn(int,QByteArray)), _broker, SLOT(handleDataIn(int,QByteArray)), Qt::DirectConnection);

    connect(thread, SIGNAL(clientDisconnected(int)), this, SLOT(_slotClientDisconnected(int)), Qt::DirectConnection);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(message(int,QString)), this, SIGNAL(message(int,QString)), Qt::DirectConnection);

    _broker->threads.insert(socketDescriptor, thread);
    thread->start();
}


void QkConnectServer::_slotClientDisconnected(int socketDescriptor)
{
    if(_broker->threads.remove(socketDescriptor) == 0)
        emit message(QKCONNECT_MESSAGE_ERROR,
                     QString().sprintf("Failed to remove socket descriptor %d", socketDescriptor));
}

void QkConnectServer::sendData(QByteArray data)
{
    _broker->sendData(data);
}


void Broker::handleDataIn(int socketDescriptor, QByteArray data)
{
    emit dataToConn(data);
}

void Broker::sendData(QByteArray data)
{
    emit dataToClient(data);
}
