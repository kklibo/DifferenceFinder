#include "usersettings.h"

UserSettings::UserSettings()
    : byteGridColumnMode(dataSetView::ByteGridColumnMode::Fill),
      byteGridColumn_LargestMultipleOf_N(8),
      byteGridColumn_UpTo_N(8),
      byteGridScrollingMode(dataSetView::ByteGridScrollingMode::FixedRows),
      windowWidth(0),
      windowHeight(0),
      logAreaHeight(0)
{

}

void UserSettings::loadINIFile() {

    QSettings settings("DifferenceFinder", "DifferenceFinder");


    byteGridColumnMode =                    static_cast<dataSetView::ByteGridColumnMode>
                                            (settings.value("byteGridColumnMode").toInt());

    byteGridColumn_LargestMultipleOf_N =    settings.value("byteGridColumn_LargestMultipleOf_N").toUInt();

    byteGridColumn_UpTo_N =                 settings.value("byteGridColumn_LargestMultipleOf_N").toUInt();

    byteGridScrollingMode =                 static_cast<dataSetView::ByteGridScrollingMode>
                                            (settings.value("byteGridScrollingMode").toInt());

    windowWidth =                           settings.value("windowWidth").toUInt();

    windowHeight =                          settings.value("windowHeight").toUInt();

    logAreaHeight =                         settings.value("logAreaHeight").toUInt();
}

void UserSettings::saveINIFile() {

    QSettings settings("DifferenceFinder", "DifferenceFinder");


    //cast enum class values to int
    settings.setValue("byteGridColumnMode",                 static_cast<int>(byteGridColumnMode)    );
    settings.setValue("byteGridColumn_LargestMultipleOf_N", byteGridColumn_LargestMultipleOf_N      );
    settings.setValue("byteGridColumn_UpTo_N",              byteGridColumn_UpTo_N                   );
    settings.setValue("byteGridScrollingMode",              static_cast<int>(byteGridScrollingMode) );
    settings.setValue("windowWidth",                        windowWidth                             );
    settings.setValue("windowHeight",                       windowHeight                            );
    settings.setValue("logAreaHeight",                      logAreaHeight                           );

    settings.sync();

}
