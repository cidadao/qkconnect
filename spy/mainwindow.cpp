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

    _jsonParser = new JsonParser(this);
    connect(_jsonParser, SIGNAL(parsed(QJsonDocument)),
            this, SLOT(_parseJson(QJsonDocument)));

    _colorClient = QColor("#53DE9D");
    _colorConn = QColor("#83D6F2");

    _socket = new QTcpSocket(this);

    connect(_socket, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));
    connect(_socket, SIGNAL(connected()), this, SLOT(slotConnected()));
    connect(_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotSocketError(QAbstractSocket::SocketError)));

    connect(ui->buttonConnect, SIGNAL(clicked()),
            this, SLOT(slotConnect()));
    connect(ui->buttonClear, SIGNAL(clicked()),
            this, SLOT(slotClear()));

#ifdef Q_OS_UNIX
    ui->textWindow->setFont(QFont("Monospace", 9));
#endif
#ifdef Q_OS_WIN
    ui->textWindow->setFont(QFont("Consolas", 9));
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

void MainWindow::slotClear()
{
    ui->textWindow->clear();
}

void MainWindow::_parseData(QByteArray data)
{
    _jsonParser->parseData(data);
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
