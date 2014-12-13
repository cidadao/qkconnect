#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTcpSocket>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QVariantMap>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _colorClient = QColor("#53DE9D");
    _colorConn = QColor("#83D6F2");
    _depthLevel = 0;

    _socket = new QTcpSocket(this);

    connect(_socket, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));
    connect(_socket, SIGNAL(connected()), this, SLOT(slotConnected()));
    connect(_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotSocketError(QAbstractSocket::SocketError)));

    connect(ui->buttonConnect, SIGNAL(clicked()),
            this, SLOT(slotConnect()));
#ifdef Q_OS_UNIX
    ui->textWindow->setFont(QFont("Monospace", 9));
#endif

    setWindowTitle("QkSpy");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotConnect()
{
    if(ui->buttonConnect->text() == "Connect")
    {
        QString ip = ui->lineIP->text();
        int port = ui->spinPort->value();
        QHostAddress host;
        if(ip.toLower() == "localhost")
            host = QHostAddress::LocalHost;
        else
            host.setAddress(ip);

        _socket->connectToHost(host, port);
    }
    else
    {
        _socket->disconnectFromHost();
    }
}

void MainWindow::slotConnected()
{
    ui->buttonConnect->setText("Disconnect");
    ui->lineIP->setEnabled(false);
    ui->spinPort->setEnabled(false);
}

void MainWindow::slotDisconnected()
{
    ui->buttonConnect->setText("Connect");
    ui->lineIP->setEnabled(true);
    ui->spinPort->setEnabled(true);
}

void MainWindow::slotReadyRead()
{
    QByteArray data = _socket->readAll();
    _parseData(data);
}

void MainWindow::slotSocketError(QAbstractSocket::SocketError error)
{
    QMessageBox::critical(this, "Error", _socket->errorString());
}

void MainWindow::_parseData(QByteArray data)
{
    bool done = false;
    char *p_data = data.data();
    for(int i = 0; i < data.count(); i++)
    {
        if(*p_data == '{')
        {
            if(_depthLevel == 0)
                _jsonStr = "";
            _depthLevel++;
        }
        else if(*p_data == '}')
        {
            _depthLevel--;
            if(_depthLevel == 0)
                done = true;
        }
        _jsonStr += *p_data++;

        if(done)
        {
            QJsonParseError jsonError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(_jsonStr, &jsonError);
            if(jsonError.error != QJsonParseError::NoError)
            {
                qDebug() << jsonError.errorString();
            }
            else
            {
                _parseJson(jsonDoc);
                done = false;
            }
        }

    }
}

void MainWindow::_parseJson(QJsonDocument json)
{
    QJsonObject obj = json.object();
    QVariantMap map = obj.toVariantMap();

    QString src = map.value("src").toString();
    QByteArray data = map.value("data").toByteArray();
    if(src == "client")
    {
        ui->textWindow->setTextColor(_colorClient);
    }
    else
    {
        ui->textWindow->setTextColor(_colorConn);
    }

    if(ui->comboFormat->currentText() == "HEX")
    {
        foreach(char c, data)
        {
            ui->textWindow->insertPlainText(QString().sprintf("%02X ", c));
        }
    }
    else if(ui->comboFormat->currentText() == "ASCII")
    {
        ui->textWindow->insertPlainText(QString(data));
    }

}
