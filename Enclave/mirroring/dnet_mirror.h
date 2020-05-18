/*
 * Created on Mon Feb 24 2020
 *
 * Copyright (c) 2020 xxx xxxx, xxxx
 */

#ifndef DNET_MIRROR_H
#define DNET_MIRROR_H

#define ROMULUS_LOG_PTM 
//for romulus persistent types

//#include "romulus/common/RomSGX.h"
#include "romulus/common/tm.h"
#include "dnet-in/src/darknet.h"
#include "crypto/crypto.h"

//class prototype

class NVModel
{

public:
    
    struct Layer
    {
        TM_TYPE<int> id;       //Layer Id
        TM_TYPE<Layer *> next; //next layer
        /**
         * The following neural net params may or may not exist
         * depending on the type of layer in question.
         */
        TM_TYPE<uint8_t *> nvweights;
        TM_TYPE<uint8_t *> nvbiases;
        TM_TYPE<uint8_t *> nvscales;
        TM_TYPE<uint8_t *> rollxmean;
        TM_TYPE<uint8_t *> rollxvar;
    };

    TM_TYPE<int> num_layers{0};

    //Epoch: training iteration afterwhich the parameters were saved == net->seen
    TM_TYPE<size_t> epoch{0};
    TM_TYPE<float> aloss{0.0};      //avg loss at specific epoch
    TM_TYPE<Layer *> head{nullptr}; //first layer/root

    //public:
    /**
     * Constructors
     */

    NVModel();
    /**
     * Persistent memory allocator
     */
    void allocator(network *net);
    /**
     * Mirrors/recovers network params from persistent memory into the enclave
     * i.e nv_model --> net
     */

    void mirror_in(network *net, float *avg_loss);

    /**
     * Mirrors/flushes network params from the enclave to persistent memory
     * i.e net --> nv_model
     */
    void mirror_out(network *net, float *avg_loss);

    
};

#endif /* DNET_MIRROR_H */
