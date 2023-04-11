#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H
#include <QString>
//#include "coordinate.h"
//#include <device.h>


class DataGenerator
{
public:
    //DataGenerator();

    QString GenerateMacAddress();
    QString GenerateName();
    //QVector<std::pair<QString, Coordinate>> GenerateCoordinate(const QVector<Device>& devices, const QPoint& point, QDateTime time = QDateTime::currentDateTime());
};

#endif // DATAGENERATOR_H
