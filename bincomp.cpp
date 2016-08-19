#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QVector>

#include "bincomp.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
bool fileToVector(QString fileName, QVector<unsigned char>& dataSet)
{
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDataStream in(&file);

    int s = file.size();
    char *rawFile = new char[s];
    in.readRawData(rawFile, s);

    dataSet.clear();
    dataSet.resize(s);

    qCopy(rawFile, rawFile+s, dataSet.begin());
    file.close();

    delete[] rawFile;
    return true;
}*/

/*
bool compare(QVector<unsigned char> dataset1, QVector<unsigned char> dataset2, QVector<byterange>& diffs)
{
    if (dataset1.size() != dataset2.size()){
        return false;
    }

    QVector<unsigned char>::iterator it_dataset1 = dataset1.begin();
    QVector<unsigned char>::iterator it_dataset2 = dataset2.begin();
    bool openrange = false;

    while (it_dataset1 != dataset1.end() && it_dataset2 != dataset2.end()) {
        if (*it_dataset1 != *it_dataset2){

            int byteindex = it_dataset1 - dataset1.begin();

            if (openrange){
                byterange& last = diffs.back();
                last.count++;
            } else {
                diffs.push_back(byterange(byteindex));
                openrange = true;
            }

            std::cout << byteindex << ": " << int(*it_dataset1) << ", " << int(*it_dataset2) << std::endl;
        } else {
            openrange = false;
        }

        it_dataset1++;
        it_dataset2++;
    }

    return true;
}*/


void vectorToQTextEdit(QTextEdit* textEdit, QVector<unsigned char>& dataSet, QVector<byterange>& diffs)
{
    int ndiff = 0;

    QColor orig = textEdit->textColor();

    for (int i = 0; i < dataSet.size(); i++) {
        QString str;
        if (!(dataSet[i] & 0xF0)){
            str += "0";
        }
        str +=  QString::number( dataSet[i], 16 ).toUpper() + " ";

        if (ndiff < diffs.size()) {

            if (i >= diffs[ndiff].start) {
                textEdit->setTextColor(QColor::fromRgb(255,0,0));
            }

            if (i >= diffs[ndiff].start + diffs[ndiff].count) {
                textEdit->setTextColor(orig);
                ndiff++;
            }
        }

        textEdit->insertPlainText(str);
    }
}

/*
bool vectorSubsetToQTextEdit(QTextEdit* textEdit, QVector<unsigned char>& dataSet, byterange subset, QVector<byterange>& diffs)
{
    int ndiff = 0;

    QColor orig = textEdit->textColor();

    if (subset.start < 0 || subset.count + subset.start > dataSet.size()) {
        return false;
    }

    while (diffs[ndiff].start + diffs[ndiff].count <= subset.start && ndiff < diffs.size())
    {
        ndiff++;    //advance through diffs to the first one that applies to this range
    }

    for (int i = subset.start; i < subset.count + subset.start; i++) {
        QString str;
        str.reserve(65536);
        if (!(dataSet[i] & 0xF0)){
            str += "0";
        }
        str +=  QString::number( dataSet[i], 16 ).toUpper() + " ";

        if (ndiff < diffs.size()) {

            if (i >= diffs[ndiff].start) {
                textEdit->setTextColor(QColor::fromRgb(255,0,0));
            }

            if (i >= diffs[ndiff].start + diffs[ndiff].count) {
                textEdit->setTextColor(orig);
                ndiff++;
            }
        }

        textEdit->insertPlainText(str);
    }

    textEdit->setTextColor(orig);
    return true;
}
*/
