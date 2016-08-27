#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QWidget>
#include "byteRange.h"

/*
 *  a floating debug info window for the QT interface (might be temporary?)
*/


namespace Ui {
class DebugWindow;
}

class DebugWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DebugWindow(QWidget *parent = 0);
    ~DebugWindow();

public slots:
    void dataSet1RangeChanged(byteRange newRange);
    void dataSet2RangeChanged(byteRange newRange);

    void dataSet1SizeChanged(unsigned int newSize);
    void dataSet2SizeChanged(unsigned int newSize);

private:
    Ui::DebugWindow *ui;
};

#endif // DEBUGWINDOW_H
