#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <datagenerator.cpp>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // Создаем сервер
    DataGenerator generator;

        //принимаем запрос на данные с конкретным временем
        //генерим данные с таким временем
        //отсылаем
    return a.exec();
}
