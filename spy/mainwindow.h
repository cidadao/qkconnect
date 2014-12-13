#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractSocket>
class QTcpSocket;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void slotConnect();
    void slotConnected();
    void slotDisconnected();
    void slotReadyRead();
    void slotSocketError(QAbstractSocket::SocketError error);

private:
    Ui::MainWindow *ui;
    QTcpSocket *_socket;

    QColor _colorClient;
    QColor _colorConn;

    int _depthLevel;
    QByteArray _jsonStr;

    void _parseData(QByteArray data);
    void _parseJson(QJsonDocument json);


};

#endif // MAINWINDOW_H
