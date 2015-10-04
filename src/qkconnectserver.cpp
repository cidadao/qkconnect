#include "qkconnectserver.h"
#include "qkclientthread.h"
#include "qksocket.h"
#include "qkconnect_global.h"

#include <QDebug>
#include <QQueue>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

QkConnectServer::QkConnectServer(QString ip, int port, QObject *parent) :
    QkServer(ip, port, parent)
{
    _parseMode = false;
    _protocolIn = new Qk::Protocol(this);
    _protocolOut = new Qk::Protocol(this);

    connect(&_jsonParser, SIGNAL(parsed(QJsonDocument)),
            this, SLOT(handleJsonIn(QJsonDocument)));

    connect(_protocolIn, SIGNAL(parsedFrame(QByteArray,bool)),
            this, SLOT(handleFrameIn(QByteArray,bool)));
    connect(_protocolOut, SIGNAL(parsedFrame(QByteArray,bool)),
            this, SLOT(handleFrameOut(QByteArray,bool)));
}

void QkConnectServer::setParseMode(bool parse)
{
    _parseMode = parse;
}

void QkConnectServer::setOptions(int options)
{
    _options = options;
}

void QkConnectServer::run()
{
    if(_parseMode)
    {
        QEventLoop eventLoop;
        QTimer timer;
        int framesInCount;
        bool ackReceived;
        bool alive;

        timer.setSingleShot(true);
        connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
        connect(_protocolOut, SIGNAL(parsedFrame(QByteArray,bool)),
                &eventLoop, SLOT(quit()));

        while(1)
        {
            eventLoop.processEvents();

            _mutex.lock();
            framesInCount = _framesInQueue.count();
            alive = _alive;
            _mutex.unlock();

            if(!alive)
                break;

            if(framesInCount > 0)
            {
                emit message(QkConnect::MESSAGE_TYPE_DEBUG,
                             "dequeue frame_in (" + QString::number(framesInCount) + " available)");

                _mutex.lock();
                QByteArray frame = _framesInQueue.dequeue();
                ackReceived = _ackReceived = false;
                _mutex.unlock();

//                emit dataIn(frame);
                QString json_str = "{";
                json_str += "\"type\": \"data\",";
                json_str += "\"format\": \"serial\",";
                json_str += "\"data\": \"" + frame.toBase64() + "\"";
                json_str += "}";
                QJsonDocument doc = QJsonDocument::fromJson(json_str.toUtf8());
                emit jsonIn(doc);
                emit dataIn(doc.toJson());

                timer.start(5000);
                while(timer.isActive() && !ackReceived)
                {
                    eventLoop.exec();
                    _mutex.lock();
                    ackReceived = _ackReceived;
                    _mutex.unlock();
                }

                if(!ackReceived)
                    qDebug() << "TIMEOUT! failed to get ACK from connection";
            }
        }
    }
}

void QkConnectServer::handleDataIn(int socketDesc, QByteArray data)
{
    _jsonParser.parseData(data);
}

void QkConnectServer::handleJsonIn(QJsonDocument doc)
{
    if(_parseMode)
    {
        QJsonObject obj = doc.object();
        if(obj.value("type").toString() == "data" &&
           obj.value("format").toString() == "serial")
        {
            QString data_str = obj.value("data").toString();
            QByteArray data = QByteArray::fromBase64(data_str.toUtf8());
           _protocolIn->parseData(data, true);
        }
        else
        {
            emit jsonIn(doc);
            emit dataIn(doc.toJson());
        }
    }
    else
    {
        emit jsonIn(doc);
        emit dataIn(doc.toJson());
    }
}

void QkConnectServer::sendData(QByteArray data)
{
    if(_parseMode)
    {
        _protocolOut->parseData(data, true);
    }
    else
    {
        emit dataOut(data);
    }
}

void QkConnectServer::sendJson(QJsonDocument doc)
{
    if(_parseMode)
    {
        QJsonObject obj = doc.object();
        if(obj.value("type").toString() == "data" &&
           obj.value("format").toString() == "serial")
        {
            QString data_str = obj.value("data").toString();
            QByteArray data = QByteArray::fromBase64(data_str.toUtf8());
           _protocolOut->parseData(data, true);
        }
        else
        {
            emit jsonOut(doc);
            emit dataOut(doc.toJson());
        }
    }
    else
    {
        emit jsonOut(doc);
        emit dataOut(doc.toJson());
    }
}

void QkConnectServer::handleFrameIn(QByteArray frame, bool raw)
{
    _mutex.lock();
    _framesInQueue.enqueue(frame);
    _mutex.unlock();
}

void QkConnectServer::handleFrameOut(QByteArray frame, bool raw)
{
    int flags = Qk::Frame::flags(frame, raw);
    int code = Qk::Frame::code(frame, raw);

    emit message(QkConnect::MESSAGE_TYPE_DEBUG,
                 "frame_out: " + frame.toHex().toUpper());

    if(code == Qk::PACKET_CODE_ACK)
    {
        _mutex.lock();
        _ackReceived = true;
        _mutex.unlock();
    }

    if((_options & joinFragments) && (flags & Qk::PACKET_FLAG_FRAG))
    {
        _fragments.append(frame);
        if(flags & Qk::PACKET_FLAG_LASTFRAG)
        {
            QByteArray fullFrame = Qk::Frame::join(_fragments, raw);
            emit dataOut(fullFrame);
            _fragments.clear();
        }
    }
    else
    {
//        emit dataOut(frame);
        QString json_str = "{";
        json_str += "\"type\": \"data\",";
        json_str += "\"format\": \"serial\",";
        json_str += "\"data\": \"" + frame.toBase64() + "\"";
        json_str += "}";
        QJsonDocument doc = QJsonDocument::fromJson(json_str.toUtf8());
        emit jsonOut(doc);
        emit dataOut(doc.toJson());
    }

}

void QkConnectServer::_slotClientConnected(int socketDesc)
{
    QkServer::_slotClientConnected(socketDesc);

    QkSocket *socket = _threads.value(socketDesc)->socket();


    QString heyMsg = "client.connected";
    heyMsg += QString().sprintf(" %s %d",
               socket->peerAddress().toString().toStdString().c_str(),
               socketDesc);

    emit message(QkConnect::MESSAGE_TYPE_INFO, heyMsg);
}

void QkConnectServer::_slotClientDisconnected(int socketDesc)
{
    QkSocket *socket = _threads.value(socketDesc)->socket();

    QString byeMsg = "client.disconnected";
    byeMsg += QString().sprintf(" %s %d",
                socket->peerAddress().toString().toStdString().c_str(),
                socketDesc);

    emit message(QkConnect::MESSAGE_TYPE_INFO, byeMsg);

    QkServer::_slotClientDisconnected(socketDesc);
}

