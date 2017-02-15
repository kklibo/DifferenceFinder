#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include "dataSetView.h"

/*
    Stores user-controllable settings
 */


class UserSettings
{
public:
    UserSettings();

    //column count mode
    dataSetView::ByteGridColumnMode byteGridColumnMode;
    //values for specific modes
    unsigned int byteGridColumn_LargestMultipleOf_N;
    unsigned int byteGridColumn_UpTo_N;

    //scrolling mode
    dataSetView::ByteGridScrollingMode byteGridScrollingMode;

};

#endif // USERSETTINGS_H