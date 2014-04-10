#ifndef H5OUTPUTFILE_H
#define H5OUTPUTFILE_H

#include "h5filehelper.h"

/**
 * @brief The H5OutputFile class allows to manipulate the HDF5 files generated by
 *        MCPlusPlus
 */

class H5OutputFile : public H5FileHelper
{
public:
    H5OutputFile();

    virtual bool newFile(const char *fileName);
    void appendTransmittedExitPoints(const MCfloat *buffer, const hsize_t size);
    void loadTransmittedExitPoints(MCfloat *destBuffer);
    void loadTransmittedExitPoints(const hsize_t *start, const hsize_t *count, MCfloat *destBuffer);
    unsigned long int transmitted();

private:
    bool createExitPointsDatasets();
};

#endif // H5OUTPUTFILE_H
