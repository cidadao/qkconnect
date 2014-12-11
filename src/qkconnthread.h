#ifndef QKCONNTHREAD_H
#define QKCONNTHREAD_H

#include <QThread>
#include "qkconn.h"

class QkConnThread : public QThread
{
    Q_OBJECT
public:
    explicit QkConnThread(const QString &type, QkConn::Descriptor desc, QObject *parent = 0);

protected:
    void run();

signals:
    void message(int,QString);


public slots:
    void sendData(QByteArray data);

private:
    QString _type;
    QkConn::Descriptor _desc;
    QkConn *_conn;

};

#endif // QKCONNTHREAD_H
