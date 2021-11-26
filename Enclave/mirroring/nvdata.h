
/*
 * Created on Tue Apr 14 2020
 *
 * Copyright (c) 2020 xxxx xxxx, xxxx
 */

#ifndef NVDATA_H
#define NVDATA_H
#define ROMULUS_LOG_PTM //for romulus persistent types

//#include "romulus/common/RomSGX.h"
#include "romulus/common/tm.h"
#include "dnet-in/src/darknet.h"
#include "crypto/crypto.h"

//mnist image xtics
#define NUM_IMGS 60000
#define NUM_LABELS NUM_IMGS
#define IMG_ROWS 28
#define IMG_COLS 28
#define NUM_CLASSES 10
#define IMG_SIZE 784
//(IMG_ROWS * IMG_COLS)

class NVData
{

public:
    struct NvBox
    {
        TM_TYPE<float> x, y, w, h;
    };
    struct NvMatrix
    {
        TM_TYPE<int> rows;
        TM_TYPE<int> cols;
        TM_TYPE<float **> vals;
    };
    struct NvData
    {
        TM_TYPE<int> w, h;
        TM_TYPE<NvMatrix *> x;
        TM_TYPE<NvMatrix *> y;
        TM_TYPE<int> shallow;
        TM_TYPE<int *> num_boxes;
        TM_TYPE<NvBox **> boxes;
    };

    TM_TYPE<NvData *> nvdata{0}; //main data object
    TM_TYPE<NvMatrix *> X;
    TM_TYPE<NvMatrix *> Y;
    TM_TYPE<int> data_present = 0;

    NVData();
    //NV Data management
    void alloc();
    void fill_pm_data(data *ptr);
    void shallow_copy_data(data *ptr);
    void deep_copy_data(data *ptr, size_t chunk);
};

#endif /* NVDATA_H */
