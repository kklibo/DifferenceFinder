#include "hexfield.h"

hexField::hexField(QWidget* parent) :
    QTextEdit(parent)
{
    setAcceptDrops(true);
}

void hexField::dropEvent(QDropEvent *e)
{
    QMimeData const* mimeData = e->mimeData();
    QList<QUrl> urlList = mimeData->urls();

    if (urlList.isEmpty()) {
        return; //no filename generated
    }

    QString filename = urlList.first().toString();
    QString prefix = "file://";
    if (filename.startsWith(prefix))    //reject dropped items that aren't files
    {
        filename.remove(0,prefix.size());
        emit filenameDropped(filename);
    }

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

void hexField::changeEvent(QEvent *e)
{
    QTextEdit::changeEvent(e);
    emit fontChanged();
}

void hexField::resizeEvent(QResizeEvent *e)
{
    QTextEdit::resizeEvent(e);
    emit resized();
}
