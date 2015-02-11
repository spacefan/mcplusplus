#include "tests.h"

#include <iostream>

using namespace std;
using namespace MCPP;

void pass() {
    cout << "testHistogram PASSED" << endl;
    remove(outputFileName);
    exit(EXIT_SUCCESS);
}

void fail() {
    cout << "FAILED" << endl;
    remove(outputFileName);
    exit(EXIT_FAILURE);
}

int main() {

    if(access(outputFileName,F_OK)<0)
        testBilayer();

    H5OutputFile file;

    file.openFile(outputFileName);

    MCfloat buf[52*251];
    uint idx;
    MCfloat relativeError, result, expectedResult;

    file.openDataSet("points");
    file.loadAll(buf);
    idx = 29*2+1;
    result = buf[idx];
    expectedResult = 1.6724756731690697E-6;
    relativeError = fabs((result - expectedResult) / expectedResult);
    if(relativeError > 1e-13) fail();

    file.openDataSet("times");
    file.loadAll(buf);
    idx = 29*3+1;
    result = buf[idx];
    expectedResult = 1.34e-4;
    relativeError = fabs((result - expectedResult) / expectedResult);
    if(relativeError > 1e-13) fail();

    idx = 29*3+2;
    result = buf[idx];
    expectedResult = 14063.28343893917;
    relativeError = fabs((result - expectedResult) / expectedResult);
    if(relativeError > 1e-13) fail();

    file.openDataSet("k");
    file.loadAll(buf);
    idx = 16*2+1;
    result = buf[idx];
    expectedResult = 0.0013635159299326945;
    relativeError = fabs((result - expectedResult) / expectedResult);
    if(relativeError > 1e-13) fail();

    file.openDataSet("points_vs_time");
    file.loadAll(buf);
    idx = 20*52+10;
    result = buf[idx];
    expectedResult = 1.5139128733131508E-7;
    relativeError = fabs((result - expectedResult) / expectedResult);
    if(relativeError > 1e-13) fail();

    pass();
    return 0;
}

