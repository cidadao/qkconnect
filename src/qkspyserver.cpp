#include "qkspyserver.h"
#include "qksocket.h"
#include "qkclientthread.h"
#include "qkconnect_global.h"

#include <QDebug>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>


QkSpyServer::QkSpyServer(QString ip, int port, QObject *parent) :
    QkServer(ip, port, parent)
{

}

void QkSpyServer::handleDataIn(int socketDescriptor, QByteArray data)
{
    qDebug() << "handle data from spy";
}


void QkSpyServer::sendFromClient(QByteArray data)
{
    QJsonObject jsonObj;
    jsonObj.insert("src", QJsonValue("client"));
    jsonObj.insert("data", QJsonValue(QString(data)));
    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObj);

    emit dataOut(jsonDoc.toJson());
}

void QkSpyServer::sendFromConn(QByteArray data)
{
    QJsonObject jsonObj;
    jsonObj.insert("src", QJsonValue("conn"));
    jsonObj.insert("data", QJsonValue(QString(data)));
    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObj);

    emit dataOut(jsonDoc.toJson());
}


void QkSpyServer::_slotClientConnected(int socketDesc)
{
    QkServer::_slotClientConnected(socketDesc);

    QkSocket *socket = _threads.value(socketDesc)->socket();
    QString heyMsg = QString().sprintf("Hey: %s (spy:%d)",
                                       socket->peerAddress().toString().toStdString().c_str(),
                                       socketDesc);

    emit message(QKCONNECT_MESSAGE_INFO, heyMsg);
}

void QkSpyServer::_slotClientDisconnected(int socketDesc)
{
    QkSocket *socket = _threads.value(socketDesc)->socket();
    QString byeMsg = QString().sprintf("Bye: %s (spy:%d)",
                                       socket->peerAddress().toString().toStdString().c_str(),
                                       socketDesc);

    emit message(QKCONNECT_MESSAGE_INFO, byeMsg);

    QkServer::_slotClientDisconnected(socketDesc);
}
