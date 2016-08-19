#ifndef BINCOMP_H
#define BINCOMP_H

#include "ui_mainwindow.h"


class byterange {
public:
    int start;
    int count;

    byterange(){
        start = 0;
        count = 0;
    }
    byterange(int start){
        this->start = start;
        count = 1;
    }
    byterange(int start, int count){
        this->start = start;
        this->count = count;
    }

    int end() const {
        return start + count;
    }
};

bool fileToVector(QString fileName, QVector<unsigned char>& dataSet);
bool compare(QVector<unsigned char> dataset1, QVector<unsigned char> dataset2, QVector<byterange>& diffs);
void vectorToQTextEdit(QTextEdit* textEdit, QVector<unsigned char>& dataSet, QVector<byterange>& diffs);
bool vectorSubsetToQTextEdit(QTextEdit* textEdit, QVector<unsigned char>& dataSet, byterange subset, QVector<byterange>& diffs);


#endif // BINCOMP_H
