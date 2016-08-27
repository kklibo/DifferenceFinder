#ifndef HEXFIELD_H
#define HEXFIELD_H

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTextEdit>
#include <QMimeData>
#include <QString>

/*
 *  extends QTextEdit to support file drag and drop
*/


QT_BEGIN_NAMESPACE
class QDropEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
QT_END_NAMESPACE

class hexField : public QTextEdit
{
    Q_OBJECT

public:
    explicit hexField(QWidget* parent = nullptr);

    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *e) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *e) Q_DECL_OVERRIDE;

signals:
    void filenameDropped(QString filename);

};

#endif // HEXFIELD_H
