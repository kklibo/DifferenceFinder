#ifndef DEFENSIVECODING_H
#define DEFENSIVECODING_H

#include "log.h"

/*
PRECONDITION
POSTCONDITION
ASSERT
ASSERT_EQ


    LOG.Info(QString("%1, %2, %3").arg(__FILE__).arg(__LINE__).arg(__func__));
*/

#define ASSERT(val)                 do{ if(val          ){break;} LOG.Defensive(QString("ASSERT Failed: %1:%2")                 .arg(__FILE__).arg(__LINE__)); }while(false)
#define ASSERT_EQ(lhv, rhv)         do{ if(lhv==rhv     ){break;} LOG.Defensive(QString("ASSERT_EQ Failed: %1:%2")              .arg(__FILE__).arg(__LINE__)); }while(false)
#define ASSERT_NOT_NEGATIVE(val)    do{ if(0 <= val     ){break;} LOG.Defensive(QString("ASSERT_NOT_NEGATIVE Failed: %1:%2")    .arg(__FILE__).arg(__LINE__)); }while(false)
#define ASSERT_LE_INT_MAX(val)      do{ if(val<=INT_MAX ){break;} LOG.Defensive(QString("ASSERT_LE_INT_MAX Failed: %1:%2")      .arg(__FILE__).arg(__LINE__)); }while(false)

#endif // DEFENSIVECODING_H
