#include <stdio.h>
#include "h5outputfile.h"

#include <unistd.h>
#include <cmath>

#include <boost/math/constants/constants.hpp>

using namespace boost::math::constants;
using namespace std;

const char *progName;
MCfloat minVal[2], maxVal[2];
size_t nBins[2] = {1,1};
size_t binStride[2] = {1,1};
MCfloat **histoData[2];
MCfloat *data[2][4];
MCfloat binSize[] = {-1, -1};
MCfloat *modules[4] = {NULL, NULL, NULL, NULL};
const u_int64_t *photonCounters;
unsigned int wFlags = FLAG_TRANSMITTED | FLAG_BALLISTIC;
uint entriesPerWalker[2] = {0, 0};

void usage(FILE *f) {
    fprintf(f, "\n"
            "usage: %s [options] inputFile.h5 [data] [data2]\n"
            "\n"
            "where data and data2 can be one of the following strings:\n"
            "\t times\n"
            "\t kz\n"
            "\t points\n"
            "\n"
            "\tdata is mandatory\n"
            "\tdata2 is optional and specifies binning in the 2nd dimension\n\n"
            "[options]:\n"
            "\t -h display help\n"
            "\t -b [size] bin size (in deg for kz)\n"
            "\t -c [size] bin size for 2nd dimension (in deg for kz)\n"
            "\t -m [M] print only one bin every M bins\n"
            "\t -n [N] print only one bin every N bins (2nd dim)\n"
            "\t -t [tbrk] walker types (defaults to tb)\n"
            "\n", progName);
}

void preprocessData(DataGroup dataGroup, int dimension) {
    MCfloat **_data = data[dimension];

    if(dataGroup == DATA_TIMES) {
        if(binSize[dimension] < 0)
            binSize[dimension] = 5e-2;

        MCfloat minTemp=1./0., maxTemp=-1./0.;

        //find min and max values
        for (uint type = 0; type < 4; ++type) {
            if(_data[type] == NULL)
                continue;
            for (u_int64_t i = 0; i < photonCounters[type]; ++i) {
                if(_data[type][i] > maxTemp)
                    maxTemp = _data[type][i];
                if(_data[type][i] < minTemp)
                    minTemp = _data[type][i];
            }
        }

        maxVal[dimension] = maxTemp;
        minVal[dimension] = minTemp;
        nBins[dimension] = ceil((maxTemp - minTemp)/binSize[dimension]);
        histoData[dimension] = _data;
    }


    if(dataGroup == DATA_K) {

        minVal[dimension] = 90;
        maxVal[dimension] = 90;

        if(_data[TRANSMITTED] != NULL || _data[BALLISTIC] != NULL) {
            minVal[dimension] = 0;
        }

        if(_data[REFLECTED] != NULL || _data[BACKREFLECTED] != NULL) {
            maxVal[dimension] = 180;
        }

        nBins[dimension] = (maxVal[dimension]-minVal[dimension])/binSize[dimension];

        histoData[dimension] = _data;
    }

    if(dataGroup == DATA_POINTS) {
        for (uint type = 0; type < 4; type++) {
            if(wFlags & walkerTypeToFlag(type)) {
                modules[type] = (MCfloat*)malloc(sizeof(MCfloat)*photonCounters[type]);
            }
        }

        //compute modules
        for (uint type = 0; type < 4; ++type) {
            if(_data[type] == NULL)
                continue;
            u_int64_t j = 0;
            for (u_int64_t i = 0; i < photonCounters[type]; ++i) {
                modules[type][i] = sqrt(_data[type][j]*_data[type][j] + _data[type][j+1]*_data[type][j+1]);
                j+=2;
            }
        }

        MCfloat tempMax=-1./0.;

        if(binSize[dimension] < 0)
            binSize[dimension] = 5e-2;

        //find max value
        for (uint type = 0; type < 4; ++type) {
            if(_data[type] == NULL)
                continue;
            for (u_int64_t i = 0; i < photonCounters[type]; ++i) {
                if(modules[type][i] > tempMax) {
                    tempMax = modules[type][i];
                    if(tempMax > 10000) {
                        tempMax = 10000;
                        type = 5; //break outer loop
                        break;
                    }
                }
            }
        }

        minVal[dimension] = 0;
        maxVal[dimension] = tempMax;
        nBins[dimension] = ceil(tempMax/binSize[dimension]);
        histoData[dimension] = modules;
    }
}

int main(int argc, char *argv[])
{
    bool histo2D = false;

    progName = argv[0];

    //parse command line options
    char c;
    extern char *optarg;
    extern int optind;
    while ((c = getopt(argc, argv, "hb:c:t:m:n:")) != -1) {
        switch (c) {
        case 'h':
            usage(stdout);
            exit(EXIT_SUCCESS);
            break;

        case 'b':
            binSize[0] = atof(optarg);
            break;

        case 'c':
            binSize[1] = atof(optarg);
            break;

        case 'm':
            binStride[0] = atoi(optarg);
            break;

        case 'n':
            binStride[1] = atoi(optarg);
            break;

        case 't':
        {
            XMLParser parser;
            wFlags = parser.walkerSaveFlags(optarg);
        }
            break;

        default:
            break;
        }
    }

    if(argc-optind < 2) {
        fprintf(stderr,"Error: mandatory arguments not specified\n");
        usage(stderr);
        exit(EXIT_FAILURE);
    }

    const char *fileName = argv[optind++];
    DataGroup dataGroup[2];
    const char *dataGroupString = argv[optind++];
    if(strcmp(dataGroupString,"times") == 0)
        dataGroup[0] = DATA_TIMES;
    else if (strcmp(dataGroupString,"kz") == 0)
        dataGroup[0] = DATA_K;
    else if (strcmp(dataGroupString,"points") == 0)
        dataGroup[0] = DATA_POINTS;
    else {
        fprintf(stderr,"Error: unknown data group: %s\n",dataGroupString);
        usage(stderr);
        exit(EXIT_FAILURE);
    }

    if(argc-optind != 0) {
        dataGroupString = argv[optind++];
        if(strcmp(dataGroupString,"times") == 0)
            dataGroup[1] = DATA_TIMES;
        else if (strcmp(dataGroupString,"kz") == 0)
            dataGroup[1] = DATA_K;
        else if (strcmp(dataGroupString,"points") == 0)
            dataGroup[1] = DATA_POINTS;
        else {
            fprintf(stderr,"Error: unknown data group: %s\n",dataGroupString);
            usage(stderr);
            exit(EXIT_FAILURE);
        }
        histo2D = true;
    }

    if(dataGroup[0] == dataGroup[1]) {
        fprintf(stderr,"Error: data and data2 cannot be the same\n");
        exit(EXIT_FAILURE);
    }

    //end options

    H5OutputFile file;
    if(!file.openFile(fileName)) {
        cerr << "Cannot open file " << fileName << endl;
    }

    photonCounters = file.photonCounters();

    cerr << "Walker types: ";
    for (uint type = 0; type < 4; ++type) {
        if(wFlags & walkerTypeToFlag(type))
            cerr << walkerTypeToString(type) << " ";
    }

    cerr << endl;

    for (int d = 0; d < 2; ++d) {
        switch (dataGroup[d]) {
        case DATA_TIMES:
            entriesPerWalker[d] = 1;
            break;
        case DATA_POINTS:
            entriesPerWalker[d] = 2;
            break;
        case DATA_K: {
            XMLParser parser;
            parser.setXMLContent(file.readXMLDescription());
            parser.parseOutput();
            if(!(parser.exitKVectorsDirsSaveFlags() & DIR_Z))
            {
                fprintf(stderr,"Error: file does not contain kz data\n");
                usage(stderr);
                exit(EXIT_FAILURE);
            }
            for (uint i = 0; i < 3; ++i) {
                if(parser.exitKVectorsDirsSaveFlags() & (1 << i))
                    entriesPerWalker[d]++;
            }
        }
        default:
            break;
        }
    }

    //allocate memory and load data from file
    for (int i = 0; i < 2; ++i) {
        memset(data[i],0,4*sizeof(MCfloat *));
        for (uint type = 0; type < 4; type++) {
            if(wFlags & walkerTypeToFlag(type)) {
                data[i][type] = (MCfloat*)malloc(sizeof(MCfloat)*photonCounters[type]*entriesPerWalker[i]);
                if(!file.loadData((DataGroup)dataGroup[i],(walkerType)type, data[i][type])) {
                    free(data[i][type]);
                    data[i][type] = NULL;
                }
            }
        }
        if(i == 0 && !histo2D)
            break;
    }
    preprocessData(dataGroup[0], 0);
    if(histo2D)
        preprocessData(dataGroup[1], 1);

    for (int i = 0; i < 2; ++i) {
        cerr << "dim " << i << " nbins = " << nBins[i] << " minVal = " << minVal[i] << endl;
    }

    //build histogram
    MCfloat scale1 = (photonCounters[0]+photonCounters[1]+photonCounters[2]+photonCounters[3]);
    size_t nStridedBins[2];
    nStridedBins[0] = nBins[0] / binStride[0] + 1;
    nStridedBins[1] = nBins[1] / binStride[1] + 1;
    size_t totBins = nStridedBins[0]*nStridedBins[1];
    u_int64_t *histo = (u_int64_t *)calloc(totBins,sizeof(u_int64_t));
    MCfloat degPerRad = 180/pi<MCfloat>();
    for (uint type = 0; type < 4; ++type) {
        if(histoData[0][type] == NULL)
            continue;
        for (u_int64_t i = 0; i < photonCounters[type]; ++i) {
            size_t index[2] = {0,0};
            if(dataGroup[0] == DATA_K)
                index[0] = (acos(histoData[0][type][i])*degPerRad - minVal[0])/binSize[0];
            else
                index[0] = (histoData[0][type][i]-minVal[0])/binSize[0];

            if(index[0] >= nBins[0])
                continue;

            if(index[0] % binStride[0] != 0)
                continue;

            if(histo2D) {
                if(dataGroup[1] == DATA_K)
                    index[1] = (acos(histoData[1][type][i])*degPerRad - minVal[1])/binSize[1];
                else
                    index[1] = (histoData[1][type][i]-minVal[1])/binSize[1];
                if(index[1] >= nBins[1])
                    continue;
                if(index[1] % binStride[1] != 0)
                    continue;
            }

            size_t idx =  (index[0]/binStride[0])*nStridedBins[1] + index[1]/binStride[1];
            histo[idx]++;
        }
    }

    //print histogram

    switch (dataGroup[0]) {
    case DATA_K:
        cout << "cosTheta";
        break;

    case DATA_POINTS:
        cout << "um";
        break;

    case DATA_TIMES:
        cout << "ps";
        break;
    default:
        break;
    }
    if(histo2D)
        for (unsigned int i = 0; i < nBins[1]; ++i) {
            if(i % binStride[1] != 0)
                continue;
            if(dataGroup[1] == DATA_K)
                cout << "\tb-" << cos(binSize[1]*(i+0.5));
            else
                cout << "\tb-" << binSize[1]*(i+0.5);
        }
    else
        cout << "\tcounts";

    cout << endl;


    switch (dataGroup[0]) {
    case DATA_K:
        for (unsigned int i = 0; i < nBins[0]; ++i) {
            if(i % binStride[0] != 0)
                continue;
            MCfloat binCenter = cos(binSize[0]*(i+0.5)/degPerRad);
            MCfloat scale2 = scale1*4.0*pi<MCfloat>()*sin((i+0.5)*binSize[0]/degPerRad)*sin(binSize[0]/2./degPerRad);
            cout << binCenter;
            for (unsigned int j = 0; j < nBins[1]; ++j) {
                if(j % binStride[1] != 0)
                    continue;
                cout << "\t" << histo[(i/binStride[0])*nStridedBins[1] + j/binStride[1]]/scale2;
            }
            cout << endl;
        }
        break;

    case DATA_TIMES:
        for (unsigned int i = 0; i < nBins[0]; ++i) {
            if(i % binStride[0] != 0)
                continue;
            MCfloat binCenter = binSize[0]*(i+0.5);
            MCfloat scale2 = scale1;
            cout << binCenter;
            for (unsigned int j = 0; j < nBins[1]; ++j) {
                if(j % binStride[1] != 0)
                    continue;
                cout << "\t" << histo[(i/binStride[0])*nStridedBins[1] + j/binStride[1]]/scale2;
            }
            cout << endl;
        }
        break;

    case DATA_POINTS:
        for (unsigned int i = 0; i < nBins[0]; ++i) {
            if(i % binStride[0] != 0)
                continue;
            MCfloat dr = binSize[0];
            MCfloat binCenter = binSize[0]*(i+0.5);
            MCfloat scale2 = scale1 * (2*pi<MCfloat>()*(i+0.5)*dr*dr);
            cout << binCenter;
            for (unsigned int j = 0; j < nBins[1]; ++j) {
                if(j % binStride[1] != 0)
                    continue;
                cout << "\t" << histo[(i/binStride[0])*nStridedBins[1] + j/binStride[1]]/scale2;
            }
            cout << endl;
        }
        break;

    default:
        break;
    }

    for (int i = 0; i < 2; ++i) {
        for (uint type = 0; type < 4; type++) {
            free(data[i][type]);
        }
    }

    free(histo);

    return 0;
}

