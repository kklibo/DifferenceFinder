#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include <QString>
#include <QSettings>
#include "dataSetView.h"

/*
    Stores user-controllable settings
 */


class UserSettings
{
public:
    UserSettings();

    void loadINIFile();
    void saveINIFile();

    //column count mode
    dataSetView::ByteGridColumnMode byteGridColumnMode;
    //values for specific modes
    unsigned int byteGridColumn_LargestMultipleOf_N;
    unsigned int byteGridColumn_UpTo_N;

    //scrolling mode
    dataSetView::ByteGridScrollingMode byteGridScrollingMode;

    //saved window size
    unsigned int windowWidth;
    unsigned int windowHeight;

};

#endif // USERSETTINGS_H
