#include "qkconnthread.h"

#include "qkconn.h"
#include "qkconnserial.h"

#include <QDebug>

QkConnThread::QkConnThread(const QString &type, QkConn::Descriptor desc, QObject *parent) :
    QThread(parent)
{
    _type = type;
    _desc = desc;
}

void QkConnThread::run()
{
    QkConn *conn = 0;
    if(_type == "serial")
    {
        conn = new QkConnSerial(_desc, this);
    }

    _conn = conn;

    connect(_conn, SIGNAL(message(int,QString)), this, SIGNAL(message(int,QString)), Qt::DirectConnection);

    _conn->open();

    exec();
}

void QkConnThread::sendData(QByteArray data)
{
    qDebug() << "thread send data" << data;
    _conn->sendData(data);
}
