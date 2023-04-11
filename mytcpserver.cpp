#include "mytcpserver.h"
#include <QDebug>
#include <QCoreApplication>

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);

    connect(mTcpServer, &QTcpServer::newConnection, this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, PortId)){
        qDebug() << "server is not started";
    } else {
        qDebug() << "server is started";
    }
}

void MyTcpServer::slotNewConnection()
{
    mTcpSocket = mTcpServer->nextPendingConnection();

    mTcpSocket->write("Hello, World!!! I am echo server!\r\n");

    connect(mTcpSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);
    connect(mTcpSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
}

void MyTcpServer::slotServerRead()
{
    while(mTcpSocket->bytesAvailable()>0)
    {
        QString str = mTcpSocket->readAll();
        qDebug() <<str;

        if(str == "Hello from client")
            mTcpSocket->write("Hello from server");
        else
        {
            mTcpSocket->write("Who are you");
        }
        //QByteArray array;
        //array.append(*str);

        //Обработали

        //отправили

        //
    }
}

void MyTcpServer::slotClientDisconnected()
{
    qDebug()<<"client diconnected";
    mTcpSocket->close();
}
