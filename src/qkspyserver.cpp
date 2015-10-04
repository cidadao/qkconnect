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


void QkSpyServer::sendFromClient(QJsonDocument doc)
{
    QJsonObject jsonObj;
    jsonObj.insert("src", QJsonValue("client"));
    jsonObj.insert("data", QJsonValue(doc.object()));
    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObj);

    emit dataOut(jsonDoc.toJson());
}

void QkSpyServer::sendFromConn(QJsonDocument doc)
{
    QJsonObject jsonObj;
    jsonObj.insert("src", QJsonValue("conn"));
    jsonObj.insert("data", QJsonValue(doc.object()));
    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObj);

    emit dataOut(jsonDoc.toJson());
}

void QkSpyServer::sendStatus(QJsonDocument doc)
{
    QJsonObject jsonObj;
    jsonObj.insert("src", QJsonValue("server"));
    jsonObj.insert("data", QJsonValue(doc.object()));
    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObj);

    emit dataOut(jsonDoc.toJson());
}


void QkSpyServer::_slotClientConnected(int socketDesc)
{
    QkServer::_slotClientConnected(socketDesc);

    QkSocket *socket = _threads.value(socketDesc)->socket();
    QString heyMsg = QString().sprintf("spy.connected %s %d",
                                       socket->peerAddress().toString().toStdString().c_str(),
                                       socketDesc);

    emit message(QkConnect::MESSAGE_TYPE_INFO, heyMsg);
}

void QkSpyServer::_slotClientDisconnected(int socketDesc)
{
    QkSocket *socket = _threads.value(socketDesc)->socket();
    QString byeMsg = QString().sprintf("spy.disconnected %s %d",
                                       socket->peerAddress().toString().toStdString().c_str(),
                                       socketDesc);

    emit message(QkConnect::MESSAGE_TYPE_INFO, byeMsg);

    QkServer::_slotClientDisconnected(socketDesc);
}
