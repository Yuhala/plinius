/*
 * Created on Tue Apr 14 2020
 *
 * Copyright (c) 2020 xxxx xxxx, xxxx
 */
#include "nvdata.h"

using namespace romuluslog;

/**
 * Default constructor
 */
static size_t chunk_counter = 0;
NVData::NVData() = default;

void NVData::alloc()
{
    //skipping transaction to prevent overflowing sgx-rom log
    //TM_WRITE_TRANSACTION([&]() {
    //allocate X
    X = (NvMatrix *)TM_PMALLOC(sizeof(struct NvMatrix));
    X->rows = NUM_IMGS;
    X->cols = IMG_SIZE;
    X->vals = (float **)TM_PMALLOC(NUM_IMGS * sizeof(float *));
    for (int i = 0; i < NUM_IMGS; i++)
    {
        X->vals[i] = (float *)TM_PMALLOC(IMG_SIZE * sizeof(float) + ADD_ENC_DATA_SIZE);
    }
    //allocate Y
    Y = (NvMatrix *)TM_PMALLOC(sizeof(struct NvMatrix));
    Y->rows = NUM_LABELS;
    Y->cols = NUM_CLASSES;
    Y->vals = (float **)TM_PMALLOC(NUM_LABELS * sizeof(float *));
    for (int i = 0; i < NUM_LABELS; i++)
    {
        Y->vals[i] = (float *)TM_PMALLOC(NUM_CLASSES * sizeof(float) + ADD_ENC_DATA_SIZE);
    }

    //allocate nvdata
    nvdata = (NvData *)TM_PMALLOC(sizeof(struct NvData));
    nvdata->shallow = 0;
    //});
    printf("-----Allocated enc pm data----\n");
}
//copies training data and labels from ram to pmem
void NVData::fill_pm_data(data *ptr)
{
    size_t img_size = IMG_SIZE * sizeof(float);

    for (int i = 0; i < NUM_IMGS; i++)
    {
        memcpy(X->vals[i], ptr->X.vals[i], img_size + ADD_ENC_DATA_SIZE);
        printf("writing image: %d in PM\n", i);
    }

    size_t label_size = NUM_CLASSES * sizeof(float); //each label row is the size of 10 labels but is 1 only at the index of the correct class
    //read labels
    for (int i = 0; i < NUM_LABELS; i++)
    {
        memcpy(Y->vals[i], ptr->y.vals[i], label_size + ADD_ENC_DATA_SIZE);
        printf("writing label: %d in PM\n", i);
    }

    //});
    printf("----Filled PM data----\n");
    data_present = 1;
}

//Points an enclave data variable to pmem data
void NVData::shallow_copy_data(data *ptr)
{
    //X matrix
    ptr->X.cols = X->cols;
    ptr->X.rows = X->rows;
    ptr->X.vals = (float **)X->vals;
    //Y matrix
    ptr->y.cols = Y->cols;
    ptr->y.rows = Y->rows;
    ptr->y.vals = (float **)Y->vals;

    //scale_data_rows(*ptr, 1. / 255);
}
//decrypts chunks of encrypted labeled pm data into enclave memory
void NVData::deep_copy_data(data *ptr, size_t batch_size)
{
    size_t img_size = IMG_SIZE * sizeof(float);
    size_t label_size = NUM_CLASSES * sizeof(float);
    char temp_ct[4 * 1024];
    char temp_pt[4 * 1024];
    int index = 0;

    for (int i = 0; i < batch_size; i++)
    {
        //----------------------get random image---------------------//
        index = rand() % X->rows;
        //copy encrypted image into enclave memory
        memcpy(temp_ct, X->vals[index], img_size + ADD_ENC_DATA_SIZE);
        //decrypt data
        decryptData(temp_ct, img_size + ADD_ENC_DATA_SIZE, temp_pt, img_size, GCM);
        //copy to data variable
        memcpy(ptr->X.vals[i], temp_pt, img_size);

        //---------------------get corresponding lables--------------//
        //copy encrypted label into enclave memory
        memcpy(temp_ct, Y->vals[index], label_size + ADD_ENC_DATA_SIZE);
        //decrypt data
        decryptData(temp_ct, label_size + ADD_ENC_DATA_SIZE, temp_pt, label_size, GCM);
        //copy to data variable
        memcpy(ptr->y.vals[i], temp_pt, label_size);
    }

    //scale_data_rows(*ptr, 1. / 255);
}
