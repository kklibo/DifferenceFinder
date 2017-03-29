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
    /*
    LOG, the global log object, may receive signals from multiple threads.
    I think multithreaded use of this instance is thread safe and won't block:
        QT docs say "signal emission is thread-safe"
        LOG has (QT Event System) thread affinity in the main (UI) thread
            as long as the signal/slot connection uses auto (default) or queued connections,
            events will be run on the main thread regardless of thread origin
        Log member functions are reentrant
    */
    void Info(QString str);
    void Warning(QString str);
    void Error(QString str);
    void Debug(QString str);
    void Defensive(QString str);

    void sendMessage(QString str = "", QColor color = QColor(0,0,0), bool timestamp = true);

    static void strMessageLvl1(const std::string& str);
    static void strMessageLvl2(const std::string& str);
    static void strMessageLvl3(const std::string& str);

signals:
    void message(QString str, QColor color);

};

extern Log LOG; //global log object (declared in log.cpp)

#endif // LOG_H
