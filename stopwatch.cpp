#include "stopwatch.h"

stopwatch::stopwatch()
    : m_times()
{

}

void stopwatch::recordTime(std::string message /*=""*/)
{
    m_times.emplace_back(std::chrono::steady_clock::now(), message);
}

void stopwatch::clear()
{
    m_times.clear();
}

void stopwatch::reportTimes(std::function<void (std::string)> callThisOnEach, bool reportSplitTimes /*= true*/) const
{
    if (m_times.empty()) {
        return;
    }

    auto it = m_times.begin();

    auto diffTime = it->time;

    //start message
    {
        std::string report = "Start: " + it->message;
        callThisOnEach(report);
    }
    ++it;

    for ( ; it != m_times.end(); ++it) {
        auto diff = it->time - diffTime;
        auto time = std::chrono::duration<double, std::milli>(diff).count();

        std::string report = "Time " + std::to_string(time) + "ms: " + it->message;
        callThisOnEach(report);

        if (reportSplitTimes) {
            diffTime = it->time;
        }
    }
}
