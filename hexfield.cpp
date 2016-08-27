#include "hexfield.h"

hexField::hexField(QWidget* parent) :
    QTextEdit(parent)
{
    setAcceptDrops(true);
}

void hexField::dropEvent(QDropEvent *e)
{
    QString filename = e->mimeData()->text().trimmed();
    QString prefix = "file://";
    if (filename.startsWith(prefix))
    {
        filename.remove(0,prefix.size());
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

