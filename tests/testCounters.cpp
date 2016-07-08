#include "tests.h"

#include <iostream>

using namespace std;
using namespace MCPP;

const char outputFileName[] = "testCounters.h5";

void pass() {
    cout << "testCounters PASSED" << endl;
    remove(outputFileName);
    exit(EXIT_SUCCESS);
}

void fail() {
    cout << "FAILED" << endl;
    remove(outputFileName);
    exit(EXIT_FAILURE);
}

int main() {
    remove(outputFileName);
    testBilayer(outputFileName);

    H5OutputFile file;

    file.openFile(outputFileName);

    if(file.transmitted() != 56426) fail();
    if(file.ballistic() != 0) fail();
    if(file.reflected() != 903482) fail();
    if(file.backReflected() != 40092) fail();

    pass();
    return 0;
}

