#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

#include <QtSql>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <datagenerator.cpp>

void addCoordinates(const CurrentCoord &currentCoord)
{
    // Открытие соединения с базой данных
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.open())
    {
        qCritical() << "Не удалось открыть базу данных";
        return;
    }

    QSqlQuery query;

    // Выбор id устройства по его mac-адресу
    query.prepare("INSERT INTO Devices (mac_address, name) "
                  "SELECT :mac_address, :name "
                  "WHERE NOT EXISTS (SELECT id FROM Devices WHERE mac_address = :mac_address)");

    query.bindValue(":mac_address", currentCoord.Mac);
    query.bindValue(":name", currentCoord.Name);


    if(!query.exec())
    {
        qDebug() << "Error executing query: " << query.lastError().text();
    }

    // Выбор id устройства по его mac-адресу
    query.prepare("SELECT id FROM Devices WHERE mac_address = :mac_address");
    query.bindValue(":mac_address", currentCoord.Mac);
    if (!query.exec())
    {
        qCritical() << "Ошибка выполнения запроса: " << query.lastError().text();
        return;
    }

    int deviceId = -1;
    if (query.next())
    {
        deviceId = query.value(0).toInt();
    }
    else
    {
        qWarning() << "Устройство с mac-адресом" << currentCoord.Mac << "не найдено в базе данных";
        return;
    }

    // Вставка новой строки в таблицу Coordinates
    query.prepare("INSERT INTO Coordinates (device_id, room_id, x, y, timestamp) "
                  "VALUES (:device_id, :room_id, :x, :y, :timestamp)");
    query.bindValue(":device_id", deviceId);
    query.bindValue(":room_id", currentCoord.RoomId);
    query.bindValue(":x", currentCoord.x);
    query.bindValue(":y", currentCoord.y);
    query.bindValue(":timestamp", currentCoord.dateTime.toString("yyyy-MM-dd hh:mm:ss"));

    if (!query.exec())
    {
        qCritical() << "Ошибка выполнения запроса: " << query.lastError().text();
        return;
    }

    qDebug() << "Добавлена запись в таблицу Coordinates: " << currentCoord.Name << currentCoord.Mac
             << currentCoord.RoomId << currentCoord.x << currentCoord.y << currentCoord.dateTime;
}

void readActualData(DataGenerator &generator)
{
    //получаем количество комнат для генерации
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM Rooms;");
    if (query.next())
    {
        generator.RoomsCount = query.value(0).toInt();
        qDebug() << "Number of rows in Rooms table:" << generator.RoomsCount;
    }
    else
    {
        qDebug() << "Query failed:" << query.lastError().text();
        return ;
    }

    //читаем какие устройства есть в базе чтобы для них генерировать
    query.prepare("SELECT id, mac_address, name FROM Devices");
    if (!query.exec())
    {
        qCritical() << "Ошибка выполнения запроса: " << query.lastError().text();
        return ;
    }

    while (query.next())
    {
        //читаем последние координаты каждого устройства
        QSqlQuery query1;
        query1.prepare("SELECT * FROM Coordinates WHERE device_id = :device_id ORDER BY timestamp DESC LIMIT 1;");
        query1.bindValue(":device_id", query.value(0).toInt());
        if (!query1.exec())
        {
            qCritical() << "Ошибка выполнения запроса: " << query1.lastError().text();
            return ;
        }
        while (query1.next())
        {
            CurrentCoord *c = new CurrentCoord(query.value(1).toString(), query.value(2).toString(), query1.value(3).toInt(), query1.value(4).toInt(),
                                              query1.value(5).toDateTime(), query1.value(2).toInt());
            generator.coords.append(*c);
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    DataGenerator generator;
    // Создаем подключение к базе данных PostgreSQL
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setPort(1111);
    db.setDatabaseName("Coords");
    db.setUserName("generator");
    db.setPassword("1234");

    if(db.open())
    {
        qDebug() << "Succesfully connected!";
        //получаем данные об устройствах и их последнем положении
        readActualData(generator);
        for(int i = 0; i < 100; i++)
        {

//            QThread::sleep(1000); // блокирует выполнение текущего потока на 1 секунду

            //генерим данные по всем устройствам на основе положения (мб генерим новые устройства)
            generator.GenerateCoordinate();
            foreach(const CurrentCoord& c, generator.coords)
            {
                addCoordinates(c);
            }
        }
    }

    return a.exec();
}

//закинули картинку
//    if(db.open())
//    {
//        qDebug() << "Succesfully connected!";
//        QSqlQuery query;
//        QFile imageFile("C:/Users/Егор/Downloads/image.jpg");
//        if (!imageFile.open(QIODevice::ReadOnly)) {
//            qWarning() << "Failed to open image file";
//            return 0;
//        }
//        QByteArray imageData = imageFile.readAll();

//        query.prepare("INSERT INTO Rooms (name, description, owner, image) VALUES (:name, :description, :owner, :image)");
//        query.bindValue(":name", "TestRoom");
//        query.bindValue(":description", "FirstTestRoom");
//        query.bindValue(":owner", "room_owner");
//        query.bindValue(":image", imageData);

//        if (!query.exec()) {
//            qWarning() << "Failed to insert image data: " << query.lastError().text();
//        }
//    }

