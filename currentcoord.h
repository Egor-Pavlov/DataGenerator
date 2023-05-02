#ifndef CURRENTCOORD_H
#define CURRENTCOORD_H

#include <QString>
#include <QDateTime>

class CurrentCoord
{
public:
    CurrentCoord(QString Mac, QString Name, int x,int y, QDateTime dateTime, int roomId);

    QString Name;
    QString Mac;
    int RoomId;
    int x;
    int y;

    QDateTime dateTime;
};

#endif // CURRENTCOORD_H
