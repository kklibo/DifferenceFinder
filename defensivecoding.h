#ifndef DEFENSIVECODING_H
#define DEFENSIVECODING_H

#include "log.h"

/*
    LOG.Info(QString("%1, %2, %3").arg(__FILE__).arg(__LINE__).arg(__func__));
*/

#define ASSERT(val)                 do{ if(val          ){break;} LOG.Defensive(QString("ASSERT Failed: %1:%2")                 .arg(__FILE__).arg(__LINE__)); }while(false)
#define ASSERT_EQ(lhv, rhv)         do{ if(lhv==rhv     ){break;} LOG.Defensive(QString("ASSERT_EQ Failed: %1:%2")              .arg(__FILE__).arg(__LINE__)); }while(false)
#define ASSERT_NOT_NEGATIVE(val)    do{ if(0 <= val     ){break;} LOG.Defensive(QString("ASSERT_NOT_NEGATIVE Failed: %1:%2")    .arg(__FILE__).arg(__LINE__)); }while(false)
#define ASSERT_LE_INT_MAX(val)      do{ if(val<=INT_MAX ){break;} LOG.Defensive(QString("ASSERT_LE_INT_MAX Failed: %1:%2")      .arg(__FILE__).arg(__LINE__)); }while(false)
#define ASSERT_LE_UINT_MAX(val)     do{ if(val<=UINT_MAX){break;} LOG.Defensive(QString("ASSERT_LE_UINT_MAX Failed: %1:%2")     .arg(__FILE__).arg(__LINE__)); }while(false)

#define FAIL()                      do{                           LOG.Defensive(QString("FAIL (should be unreachable): %1:%2")  .arg(__FILE__).arg(__LINE__)); }while(false)


template <typename T>
struct ScopeExit {
    ScopeExit(T callThis) : m_func(callThis) {}
    ~ScopeExit() { m_func(); }
    T m_func;
};

template <typename T>
ScopeExit<T> MakeScopeExit(T callThis) {
    return ScopeExit<T>(callThis);
}



#endif // DEFENSIVECODING_H
