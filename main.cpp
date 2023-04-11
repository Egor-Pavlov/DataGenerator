#include <QCoreApplication>
#include "datagenerator.h"
#include "mytcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    DataGenerator generator;

    //принимаем запрос на данные с конкретным временем
    //генерим данные с таким временем
    //отсылаем

    MyTcpServer server;



    return a.exec();
}
