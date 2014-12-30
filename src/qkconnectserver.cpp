#include "qkconnectserver.h"
#include "qkclientthread.h"
#include "qksocket.h"
#include "qkconnect_global.h"

#include <QDebug>
#include <QQueue>
#include <QEventLoop>
#include <QTimer>

QkConnectServer::QkConnectServer(QString ip, int port, QObject *parent) :
    QkServer(ip, port, parent)
{
    _parseMode = false;
    _protocolIn = new Qk::Protocol(this);
    _protocolOut = new Qk::Protocol(this);

    connect(_protocolIn, SIGNAL(parsedFrame(QByteArray,bool)),
            this, SLOT(handleFrameIn(QByteArray,bool)));
    connect(_protocolOut, SIGNAL(parsedFrame(QByteArray,bool)),
            this, SLOT(handleFrameOut(QByteArray,bool)));
}

void QkConnectServer::setParseMode(bool parse)
{
    _parseMode = parse;
}

void QkConnectServer::run()
{
    if(_parseMode)
    {
        QEventLoop eventLoop;
        QTimer timer;
        int framesInCount;
        bool frameReceived;
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
                _mutex.lock();
                QByteArray frame = _framesInQueue.dequeue();
                frameReceived = _frameReceived = false;
                _mutex.unlock();

                emit dataIn(frame);

                timer.start(3000);
                while(timer.isActive() && !frameReceived)
                {
                    eventLoop.exec();
                    _mutex.lock();
                    frameReceived = _frameReceived;
                    _mutex.unlock();
                }
                if(!frameReceived)
                    qDebug() << "timeout! can't get a frame from connection";
                else
                    qDebug() << "frame received from connection :)";
            }
        }
    }
}


void QkConnectServer::handleDataIn(int socketDesc, QByteArray data)
{
    if(_parseMode)
    {
        _protocolIn->parseData(data, true);
    }
    else
    {
        emit dataIn(data);
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

void QkConnectServer::handleFrameIn(QByteArray frame, bool raw)
{
    _mutex.lock();
    _framesInQueue.enqueue(frame);
    _mutex.unlock();
    _waitInputFrame.wakeAll();
}

void QkConnectServer::handleFrameOut(QByteArray frame, bool raw)
{
    _mutex.lock();
    _frameReceived = true;
    _mutex.unlock();

    //TODO join fragments

    emit dataOut(frame);
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

