#include "qkserver.h"
#include "qkclientthread.h"
#include "qkconnect_global.h"

#include <QDebug>
#include <QMutexLocker>

QkServer::QkServer(QString ip, int port, QObject *parent) :
    QTcpServer(parent)
{
    _ip = ip;
    _port = port;
    _status = QkServer::Disconnected;
    _alive = true;
}

QkServer::~QkServer()
{
    foreach(QkClientThread *thread, _threads)
    {
        thread->quit();
        thread->wait();
    }
}

QkServer::Status QkServer::currentStatus()
{
    return _status;
}

void QkServer::create()
{
    QHostAddress hostAddress;
    if(_ip.toLower() == "localhost")
        hostAddress = QHostAddress::LocalHost;
    else
        hostAddress.setAddress(_ip);

    if(listen(hostAddress, _port))
    {
        emit message(QkConnect::MESSAGE_TYPE_INFO, "server.listening " + QString::number(_port));
        _changeStatus(QkServer::Connected);
    }
    else
    {
        emit message(QkConnect::MESSAGE_TYPE_ERROR,
                     "server.listening " + errorString() +
                     QString().sprintf(" (%s:%s)",
                                       hostAddress.toString().toStdString().c_str(),
                                       QString::number(_port).toStdString().c_str()));
        _changeStatus(QkServer::FailedToConnect);
    }

    run();
}

void QkServer::run()
{

}

void QkServer::kill()
{
    _mutex.lock();
    _alive = false;
    _mutex.unlock();

    emit killed();
}

void QkServer::incomingConnection(qintptr socketDesc)
{
    QkClientThread *thread = new QkClientThread(this, socketDesc, this);

    connect(thread, SIGNAL(dataIn(int,QByteArray)), this, SLOT(handleDataIn(int,QByteArray)));

    connect(thread, SIGNAL(clientConnected(int)), this, SLOT(_slotClientConnected(int)));
    connect(thread, SIGNAL(clientDisconnected(int)), this, SLOT(_slotClientDisconnected(int)), Qt::DirectConnection);
    connect(thread, SIGNAL(message(int,QString)), this, SIGNAL(message(int,QString)));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    _threads.insert(socketDesc, thread);
    thread->start();
}

void QkServer::_slotClientConnected(int socketDesc)
{

}

void QkServer::_slotClientDisconnected(int socketDesc)
{
//    qDebug() << __PRETTY_FUNCTION__ << socketDesc;

    QMutexLocker locker(&_mutex);
    _threads[socketDesc]->quit();
    _threads.remove(socketDesc);
}

void QkServer::_changeStatus(Status status)
{
    _status = status;
    emit statusChanged();
}

