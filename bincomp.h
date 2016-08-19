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

#endif // BINCOMP_H
