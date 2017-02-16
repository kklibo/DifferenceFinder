#include "log.h"

Log LOG;    //global log object

void Log::Info(QString str)
{
    sendMessage(str, QColor(96,96,128));
}

void Log::Warning(QString str)
{
    sendMessage(str, QColor(255,128,96));
}

void Log::Error(QString str)
{
    sendMessage(str, QColor(192,128,128));
}

void Log::Debug(QString str)
{
    sendMessage(str, QColor(128,32,128));
}

void Log::Defensive(QString str)
{
    sendMessage(str, QColor(64,192,64));
}

void Log::sendMessage(QString str, QColor color, bool timestamp)
{
    QString outStr;
    if (timestamp) {
        outStr = QString("%1:  %2").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd HH:mm:ss")).arg(str);
    } else {
        outStr = str;
    }

    message(outStr, color);
}
