#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QWidget>

#include "bincomp.h"

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
    void dataSet1RangeChanged(byterange newRange);
    void dataSet2RangeChanged(byterange newRange);

    void dataSet1SizeChanged(int newSize);
    void dataSet2SizeChanged(int newSize);

private:
    Ui::DebugWindow *ui;
};

#endif // DEBUGWINDOW_H
