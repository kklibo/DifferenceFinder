#include "hexfield.h"

hexField::hexField(QWidget* parent) :
    QTextEdit(parent)
{
    setAcceptDrops(true);
}

void hexField::dropEvent(QDropEvent *e)
{
    QString filename = e->mimeData()->text().trimmed();
    if (filename.startsWith("file://"))
    {
        filename.remove(0,7);
    }

    emit filenameDropped(filename);

    e->acceptProposedAction();
}

void hexField::dragEnterEvent(QDragEnterEvent *e)
{
    e->acceptProposedAction();
}


void hexField::dragMoveEvent(QDragMoveEvent *e)
{
    e->acceptProposedAction();
}

void hexField::dragLeaveEvent(QDragLeaveEvent *e)
{
    e->accept();
}

