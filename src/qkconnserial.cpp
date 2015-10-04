#include "qkconnserial.h"
#include "qkconnect_global.h"
#include "clhandler.h"

#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QEventLoop>
#include <QJsonObject>
#include <QJsonValue>


QkConnSerial::QkConnSerial(const Descriptor &desc, QObject *parent) :
    QkConn(parent)
{
    _desc = desc;
    _dtr = false;
    _sp = new QSerialPort(this);
}

void QkConnSerial::listAvailable()
{
    cout << "     Name             PID    VID    Description\n";

    int i = 0;
    foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts())
    {

        if(!info.portName().contains("ACM") &&
           !info.portName().contains("USB") &&
           !info.portName().contains("COM"))
            continue;

        cout << QString("[%1] %2 %3 %4 %5")
                .arg(QString::number(i++).rightJustified(2, '0'))
                .arg(info.portName().leftJustified(16))
                .arg(QString::number(info.productIdentifier()).leftJustified(6))
                .arg(QString::number(info.vendorIdentifier()).leftJustified(6))
                .arg(info.description()) << "\n";
    }
    cout.flush();
}


bool QkConnSerial::open()
{
    connect(_sp, SIGNAL(readyRead()), this, SLOT(_slotReadyRead()));

    QString portName = _desc.params["portname"].toString();
    int baudRate = _desc.params["baudrate"].toInt();
    bool dtr = _desc.params["dtr"].toBool();

    _dtr = dtr;
    _sp->setPortName(portName);
    _sp->setBaudRate(baudRate);

    if(_sp->open(QIODevice::ReadWrite))
    {
        _sp->setBaudRate(baudRate);
        _sp->setParity(QSerialPort::NoParity);
        _sp->setFlowControl(QSerialPort::NoFlowControl);
        _sp->setDataBits(QSerialPort::Data8);

        if(_dtr == true)
            _sp->setDataTerminalReady(false);
        else
            _sp->setDataTerminalReady(true);

        _sp->clear();

        emit message(QkConnect::MESSAGE_TYPE_INFO,
                     "conn.ready");
        _changeStatus(QkConn::Ready);
        return true;
    }
    else
    {
        emit message(QkConnect::MESSAGE_TYPE_ERROR,
                     "conn.failed" + QString().sprintf("%s. %s", portName.toStdString().c_str(),
                                                                                     _sp->errorString().toStdString().c_str()));
        _changeStatus(QkConn::FailedToOpen);
        return false;
    }
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

void QkConnSerial::sendJson(QJsonDocument doc)
{
    QJsonObject obj = doc.object();
    QByteArray data = QByteArray::fromBase64(obj.value("data").toString().toUtf8());

    emit message(QkConnect::MESSAGE_TYPE_DEBUG,
                 "serial tx: " + data.toHex().toUpper());
    _sp->write(data);
}


void QkConnSerial::_slotReadyRead()
{
    QByteArray data = _sp->readAll();
    emit message(QkConnect::MESSAGE_TYPE_DEBUG,
                 "serial rx: " + data.toHex().toUpper());
//    emit dataIn(data);

    QString json_str = "{";
    json_str += "\"type\": \"data\",";
    json_str += "\"format\": \"serial\",";
    json_str += "\"data\": \"" + data.toBase64() + "\"";
    json_str += "}";
    QJsonDocument doc = QJsonDocument::fromJson(json_str.toUtf8());
    emit jsonIn(doc);
}
