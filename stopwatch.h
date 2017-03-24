#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <string>
#include <list>
#include <chrono>
#include <functional>

class stopwatch
{
public:
    stopwatch();

    void recordTime(std::string message = "");

    void clear();

    void reportTimes(std::function<void(std::string)> callThisOnEach, bool reportSplitTimes = true) const;


private:

    struct timeRecord {
        timeRecord( std::chrono::time_point<std::chrono::steady_clock> time_,
                    std::string message_)
        {
            time = time_;
            message = message_;
        }

        std::chrono::time_point<std::chrono::steady_clock> time;
        std::string message;
    };

    std::list<timeRecord> m_times;

};

#endif // STOPWATCH_H
