#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

#include <QtSql>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <datagenerator.cpp>

void addToGroupAccesDevices(int deviceId)
{
    QSqlQuery query;
    //добавляем чтобы админы видели устройство

    query.prepare("INSERT INTO groupaccesdevices (group_id, device_id) "
                  "VALUES (:group_id, :device_id) "
                  "ON CONFLICT DO NOTHING;");
    query.bindValue(":group_id", 1);
    query.bindValue(":device_id", deviceId);



    if (!query.exec())
    {
        qDebug() << "Error inserting GroupAccessDevices record:" << query.lastError().text();
    }

    //с какой-то вероятностью устройство будут видеть и просто операторы
    if(0 + rand() % 100 < 70)
    {
         query.prepare("INSERT INTO groupaccesdevices (group_id, device_id) "
                      "VALUES (:group_id, :device_id) "
                      "ON CONFLICT DO NOTHING;");
        query.bindValue(":group_id", 2);
        query.bindValue(":device_id", deviceId);

        if (!query.exec())
        {
            qDebug() << "Error inserting GroupAccessDevices record:" << query.lastError().text();
        }
    }

}

void addToDevicesinAreas(const CurrentCoord &currentCoord, int deviceId)
{
    QSqlQuery query;
    query.prepare("INSERT INTO DevicesInAreas (DeviceId, AreaId, X, Y, DateTime, Xp, Yp) "
                  "VALUES (:device_id, :area_id, :x, :y, :timestamp, :xp, :yp)");
    query.bindValue(":device_id", deviceId);
    query.bindValue(":area_id", currentCoord.RoomId);
    query.bindValue(":x", currentCoord.x);
    query.bindValue(":y", currentCoord.y);
    query.bindValue(":timestamp", currentCoord.dateTime.toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":xp", currentCoord.x);
    query.bindValue(":yp", currentCoord.y);

    if (!query.exec())
    {
        qCritical() << "Ошибка выполнения запроса: " << query.lastError().text();
        return;
    }
}

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

    //смотрим есть ли девайс в базе, если нет вставляем
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

    //получаем id устройства которое хотим обработать
    int deviceId = -1;
    if (query.next())
    {

        deviceId = query.value(0).toInt();
        //если нет такой записи в таблице доступа групп к устройствам - добавляем доступ админам и с вероятностью операторам
        addToGroupAccesDevices(deviceId);

    }
    else
    {
        qWarning() << "Устройство с mac-адресом" << currentCoord.Mac << "не найдено в базе данных";
        return;
    }

    //добавляем данные в таблицу координат
    addToDevicesinAreas(currentCoord, deviceId);

    qDebug() << "Добавлена запись в таблицу Coordinates: " << currentCoord.Name << currentCoord.Mac
             << currentCoord.RoomId << currentCoord.x << currentCoord.y << currentCoord.dateTime;

}

void readActualData(DataGenerator &generator)
{
    //получаем количество комнат для генерации
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM areas;");
    if (query.next())
    {
        generator.RoomsCount = query.value(0).toInt();
        qDebug() << "Number of rows in areas table:" << generator.RoomsCount;
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
        query1.prepare("SELECT * FROM DevicesinAreas WHERE deviceid = :device_id ORDER BY datetime DESC LIMIT 1;");
        query1.bindValue(":deviceid", query.value(0).toInt());
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
        for(int i = 0; i < 50; i++)
        {

            QThread::sleep(1);

            //генерим данные по всем устройствам на основе положения (мб генерим новые устройства)
            generator.GenerateCoordinate();
            foreach(const CurrentCoord& c, generator.coords)
            {
                addCoordinates(c);
            }
        }
    }

    qDebug() << "finish";
    return a.exec();
}

////закинули картинку
//if(db.open())
//{
//    qDebug() << "Succesfully connected!";
//    QSqlQuery query;
//    QFile imageFile("C:/Users/Егор/Downloads/image.jpg");
//    if (!imageFile.open(QIODevice::ReadOnly))
//    {
//        qWarning() << "Failed to open image file";
//        return 0;
//    }
//    QByteArray image = imageFile.readAll();

//    // Определим значения для добавления
//    QDateTime dateStart = QDateTime::currentDateTime();
//    QDateTime dateEnd = QDateTime::currentDateTime().addYears(1);
//    float scale = 1.0;
//    int xc = 1218;
//    int yc = 897;
//    QString areaDescription = "Описание зоны";
//    QString buildingDescription = "Описание здания";
//    QString address = "Красный проспект";
//    float latitude = 55.75;
//    float longitude = 37.61;

//    // Добавление записи в таблицу Areas


//    // Добавление записи в таблицу Buildings
//    query.prepare("INSERT INTO Buildings (description, Address, Latitude, Longitude) "
//                  "VALUES (:description, :address, :latitude, :longitude)");
//    query.bindValue(":description", buildingDescription);
//    query.bindValue(":address", address);
//    query.bindValue(":latitude", latitude);
//    query.bindValue(":longitude", longitude);
//    if (!query.exec())
//    {
//        qCritical() << "Ошибка выполнения запроса: " << query.lastError().text();
//        return 0;
//    }
//    // Получение идентификатора добавленной записи в таблицу Buildings
//    int newBuildingId = query.lastInsertId().toInt();

//    query.prepare("INSERT INTO Areas (description, BuildingId, name) "
//                  "VALUES (:description, :buildingId, :name)");
//    query.bindValue(":description", areaDescription);
//    query.bindValue(":buildingId", newBuildingId);
//    query.bindValue(":name", "Тестовый план №1");
//    if (!query.exec())
//    {
//        qCritical() << "Ошибка выполнения запроса: " << query.lastError().text();
//        return 0;
//    }

//    // Получение идентификатора добавленной записи в таблицу Areas
//    int newAreaId = query.lastInsertId().toInt();

//    // Добавление записи в таблицу Plans
//    query.prepare("INSERT INTO Plans (AreaId, Image, Date_start, Date_end, scale, Xc, Yc) "
//                  "VALUES (:areaId, :image, :dateStart, :dateEnd, :scale, :xc, :yc)");
//    query.bindValue(":areaId", newAreaId);
//    query.bindValue(":image", image);
//    query.bindValue(":dateStart", dateStart.toString("yyyy-MM-dd hh:mm:ss"));
//    query.bindValue(":dateEnd", dateEnd.toString("yyyy-MM-dd hh:mm:ss"));
//    query.bindValue(":scale", scale);
//    query.bindValue(":xc", xc);
//    query.bindValue(":yc", yc);
//    if (!query.exec())
//    {
//        qCritical() << "Ошибка выполнения запроса: " << query.lastError().text();
//        return 0;
//    }
//}



