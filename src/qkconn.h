#ifndef QKCONN_H
#define QKCONN_H

#include <QObject>
#include <QMap>
#include <QVariant>

class QkConn : public QObject
{
    Q_OBJECT
public:
    class Descriptor
    {
    public:
        QMap<QString,QVariant> params;
        bool operator==(Descriptor &other);
    };

    explicit QkConn(QObject *parent = 0);

signals:
    void message(int, QString);
    void dataIn(QByteArray);

public slots:
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual void sendData(QByteArray data) = 0;

protected:
    Descriptor _desc;

};

#endif // QKCONN_H
