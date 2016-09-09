#ifndef LOG_H
#define LOG_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QDateTime>

class Log : public QObject
{

    Q_OBJECT

public:
    void Info(QString str);
    void Warning(QString str);
    void Error(QString str);
    void Debug(QString str);

    void sendMessage(QString str = "", QColor color = QColor(0,0,0), bool timestamp = true);


signals:
    void message(QString str, QColor color);

};

extern Log LOG; //global log object (declared in log.cpp)

#endif // LOG_H
