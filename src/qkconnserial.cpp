#include "qkconnserial.h"
#include "qkconnect_global.h"

#include <QDebug>
#include <QSerialPort>
#include <QEventLoop>

QkConnSerial::QkConnSerial(const Descriptor &desc, QObject *parent) :
    QkConn(parent)
{
    _desc = desc;
    _sp = new QSerialPort(this);
}


bool QkConnSerial::open()
{
    connect(_sp, SIGNAL(readyRead()), this, SLOT(_slotReadyRead()));

    QString portName = _desc.params["portName"].toString();
    int baudRate = _desc.params["baudRate"].toInt();


    _sp->setPortName(portName);
    _sp->setBaudRate(baudRate);

    if(_sp->open(QIODevice::ReadWrite))
    {
        _sp->setBaudRate(baudRate);
        _sp->setParity(QSerialPort::NoParity);
        _sp->setFlowControl(QSerialPort::NoFlowControl);
        _sp->setDataBits(QSerialPort::Data8);

        _sp->clear();

        emit message(QKCONNECT_MESSAGE_INFO,
                     tr("Connection ready!"));
    }
    else
    {
        emit message(QKCONNECT_MESSAGE_ERROR,
                     tr("Connection failed: ") + QString().sprintf("%s. %s", portName.toStdString().c_str(),
                                                                                     _sp->errorString().toStdString().c_str()));

        return false;
    }

    return true;
}

void QkConnSerial::close()
{
    if(_sp->isOpen())
        _sp->close();
}

void QkConnSerial::sendData(QByteArray data)
{
    _sp->write(data);
}


void QkConnSerial::_slotReadyRead()
{
    emit dataIn(_sp->readAll());
}
