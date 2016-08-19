#include "hexfield.h"

hexField::hexField(QWidget* parent) :
    QTextEdit(parent)
{
    setAcceptDrops(true);
}

void hexField::dropEvent(QDropEvent *e)
{
/*    QString tmp;
    tmp = e->mimeData()->text();
    tmp.append( QStringLiteral("%1").arg(e->dropAction()));

    this->setText(tmp);*/

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

