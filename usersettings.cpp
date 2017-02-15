#include "usersettings.h"

UserSettings::UserSettings()
    : byteGridColumnMode(dataSetView::ByteGridColumnMode::Fill),
      byteGridColumn_LargestMultipleOf_N(8),
      byteGridColumn_UpTo_N(8),
      byteGridScrollingMode(dataSetView::ByteGridScrollingMode::FixedRows)
{

}
