#include "datagenerator.h"
#include <iomanip>
#include <random>

//режим REALTIME

//вход - мак адреса, выход - <Мак + координата>, делаем updateCoord у объектов у которых совпал мак
//если мака нет в списке, то есть он пришел впервые, то генерация(запрос) имени и добавление в список

//QVector<std::pair<QString, Coordinate>> DataGenerator::GenerateCoordinate(const QVector<Device>& devices,
//                                                                          const QPoint& point, QDateTime time)
//{
//    QVector<std::pair<QString, Coordinate>> Pairs;

//    //для каждого мака придумываем координату
//    for(int i = 0; i < devices.size(); i++)
//    {
//        int x = -20 + devices[i].GetCurrentCoord().GetX() + rand() % 40;
//        int y = devices[i].GetCurrentCoord().GetY() - 20 + rand() % 40;
//        Pairs.append(std::make_pair(devices[i].GetMacAddres(), Coordinate(x, y, time)));
//    }

//    int v = 0 + rand() % 100;//вероятность появления нового устройства в сети
//    if (v >= 95)
//    {
//        //имитация что появилось новое устройство
//        Pairs.append(std::make_pair(GenerateMacAddress(), Coordinate((-50 + point.x()) + rand()% 100,
//                                                                     (-50 + point.y()) + rand()% 100, time)));
//    }

//    else if (v <= 5)
//    {
//        if(Pairs.size() > 4)
//        {
//            //удаляем одно старое, как будто оно исчезло из помещения
//            int a = 0 + rand() % Pairs.size();
//            Pairs.removeAt(a);
//        }
//    }

//    return Pairs;
//}

QString DataGenerator::GenerateMacAddress()
{
    // Создаем генератор случайных чисел
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);

        // Генерируем 6 байт MAC-адреса
        std::string mac_address;
        //собираем в строку
        for (int i = 0; i < 6; i++)
        {
            int byte = dis(gen);
            std::stringstream ss;
            ss << std::hex << std::setw(2) << std::setfill('0') << byte;
            mac_address += ss.str();
            if (i != 5) mac_address += ":";
        }

        return QString::fromStdString(mac_address);
}

QString DataGenerator::GenerateName()
{
    // Создаем массив возможных имен
    std::vector<std::string> names = {"Александр", "Алексей", "Анатолий", "Андрей", "Антон", "Аркадий", "Вадим",
                                      "Валентин", "Валерий", "Василий", "Виталий", "Владислав", "Григорий",
                                      "Даниил", "Евгений", "Егор", "Игорь", "Кирилл", "Леонид",
                                      "Михаил", "Никита", "Николай", "Роман", "Руслан",
                                      "Станислав", "Степан", "Фёдор", "Юрий"};

    // Создаем генератор случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, names.size() - 1);

    // Генерируем случайное имя из массива
    return QString::fromStdString(names[dis(gen)]);
}
