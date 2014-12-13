#include "qkconnectserver.h"
#include "qkclientthread.h"
#include "qksocket.h"
#include "qkconnect_global.h"

#include <QDebug>

QkConnectServer::QkConnectServer(QString ip, int port, QObject *parent) :
    QkServer(ip, port, parent)
{

}

void QkConnectServer::handleDataIn(int socketDesc, QByteArray data)
{
    emit dataIn(data);
}

void QkConnectServer::sendData(QByteArray data)
{
    emit dataOut(data);
}

void QkConnectServer::_slotClientConnected(int socketDesc)
{
    QkServer::_slotClientConnected(socketDesc);

    QkSocket *socket = _threads.value(socketDesc)->socket();
    QString heyMsg = QString().sprintf("Hey: %s (client:%d)",
                                       socket->peerAddress().toString().toStdString().c_str(),
                                       socketDesc);

    emit message(QKCONNECT_MESSAGE_INFO, heyMsg);
}

void QkConnectServer::_slotClientDisconnected(int socketDesc)
{
    QkSocket *socket = _threads.value(socketDesc)->socket();
    QString byeMsg = QString().sprintf("Bye: %s (client:%d)",
                                       socket->peerAddress().toString().toStdString().c_str(),
                                       socketDesc);

    emit message(QKCONNECT_MESSAGE_INFO, byeMsg);

    QkServer::_slotClientDisconnected(socketDesc);
}
