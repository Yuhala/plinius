/*
 * Created on Mon Feb 24 2020
 *
 * Copyright (c) 2020 xxx xxxx, xxxx
 */

#include "dnet_mirror.h"

using namespace romuluslog;

/**
 * Default constructor
 */
NVModel::NVModel() = default;

/**
 * NVModel memory allocator
 * Allocates layers and other net params to non-volatile model
 */

void NVModel::allocator(network *net)
{
    Layer *temp = nullptr;
    int num = (net->n) - 1;
    /**
     * We create the linked list of layers starting from the last layer and 
     * we connect them till we reach the head; could still be done in the reverse direction :-)
     * Allocate full model in a transaction
     */
    TM_WRITE_TRANSACTION([&]() {
        num_layers = net->n;
        for (int i = num; i >= 0; i--)
        {
            layer l = net->layers[i];
            if (l.dontsave)
                continue;
            switch (l.type)
            {
            case CONVOLUTIONAL:
            case DECONVOLUTIONAL:
            {

                Layer *nvlayer = (Layer *)TM_PMALLOC(sizeof(struct Layer));
                nvlayer->id = i;
                nvlayer->nvweights = (uint8_t *)TM_PMALLOC(l.nweights * sizeof(float) + ADD_ENC_DATA_SIZE);
                nvlayer->nvbiases = (uint8_t *)TM_PMALLOC(l.n * sizeof(float) + ADD_ENC_DATA_SIZE);
                if (l.batch_normalize)
                {
                    nvlayer->nvscales = (uint8_t *)TM_PMALLOC(l.n * sizeof(float) + ADD_ENC_DATA_SIZE);
                    nvlayer->rollxmean = (uint8_t *)TM_PMALLOC(l.n * sizeof(float) + ADD_ENC_DATA_SIZE);
                    nvlayer->rollxvar = (uint8_t *)TM_PMALLOC(l.n * sizeof(float) + ADD_ENC_DATA_SIZE);
                }
                if (i == 0)
                {
                    head = nvlayer; //root layer
                }
                nvlayer->next = temp; //NB: temp = nullptr for the last layer :-)
                temp = nvlayer;
                break;
            }

            case CONNECTED:
            {
                Layer *nvlayer = (Layer *)TM_PMALLOC(sizeof(struct Layer));
                nvlayer->id = i;
                nvlayer->nvweights = (uint8_t *)TM_PMALLOC(l.outputs * l.inputs * sizeof(float) + ADD_ENC_DATA_SIZE);
                nvlayer->nvbiases = (uint8_t *)TM_PMALLOC(l.outputs * sizeof(float) + ADD_ENC_DATA_SIZE);
                if (l.batch_normalize)
                {
                    nvlayer->nvscales = (uint8_t *)TM_PMALLOC(l.outputs * sizeof(float) + ADD_ENC_DATA_SIZE);
                    nvlayer->rollxmean = (uint8_t *)TM_PMALLOC(l.outputs * sizeof(float) + ADD_ENC_DATA_SIZE);
                    nvlayer->rollxvar = (uint8_t *)TM_PMALLOC(l.outputs * sizeof(float) + ADD_ENC_DATA_SIZE);
                }
                if (i == 0)
                {
                    head = nvlayer; //root layer
                }
                nvlayer->next = temp; //NB: temp = nullptr for the last layer :-)
                temp = nvlayer;
                break;
            }

            case BATCHNORM:
            {
                Layer *nvlayer = (Layer *)TM_PMALLOC(sizeof(struct Layer));
                nvlayer->id = i;
                nvlayer->nvscales = (uint8_t *)TM_PMALLOC(l.c * sizeof(float) + ADD_ENC_DATA_SIZE);
                nvlayer->rollxmean = (uint8_t *)TM_PMALLOC(l.c * sizeof(float) + ADD_ENC_DATA_SIZE);
                nvlayer->rollxvar = (uint8_t *)TM_PMALLOC(l.c * sizeof(float) + ADD_ENC_DATA_SIZE);
                if (i == 0)
                {
                    head = nvlayer; //root layer
                }
                nvlayer->next = temp; //NB: temp = nullptr for the last layer :-)
                temp = nvlayer;

                break;
            }

            default:
                break;
            }
        }
    });
}

/**
 * 
 * Mirrors/recovers network params from persistent memory into the enclave
 * i.e nv_model --> net
 */

void NVModel::mirror_in(network *net, float *avg_loss)
{
    /**
     * We traverse the persistent linked list of layers 
     * and mirror the corresponding weights into the enclave neural net
     * We do not need a transaction here
     */
    *(net->seen) = epoch;
    *avg_loss = aloss;
    Layer *temp = head;
    int num = 0;

    for (int i = 0; i < net->n; i++)
    {
        if (temp == nullptr)
        {
            break;
        }
        layer l = net->layers[i];
        if (l.dontload)
            continue;
        switch (l.type)
        {
        case CONVOLUTIONAL:
        case DECONVOLUTIONAL:
        {

            if (l.numload)
                l.n = l.numload;
            num = l.c / l.groups * l.n * l.size * l.size;
            enc_memcpy(l.biases, temp->nvbiases, l.n * sizeof(float), DECRYPT);

            if (l.batch_normalize && (!l.dontloadscales))
            {
                enc_memcpy(l.scales, temp->nvscales, l.n * sizeof(float), DECRYPT);
                enc_memcpy(l.rolling_mean, temp->rollxmean, l.n * sizeof(float), DECRYPT);
                enc_memcpy(l.rolling_variance, temp->rollxvar, l.n * sizeof(float), DECRYPT);
            }
            enc_memcpy(l.weights, temp->nvweights, num * sizeof(float), DECRYPT);
            if (l.flipped)
            {
                transpose_matrix(l.weights, l.c * l.size * l.size, l.n);
            }
            temp = temp->next;
            break;
        }

        case CONNECTED:
        {

            enc_memcpy(l.biases, temp->nvbiases, l.outputs * sizeof(float), DECRYPT);
            enc_memcpy(l.weights, temp->nvweights, l.outputs * l.inputs * sizeof(float), DECRYPT);
            if (l.batch_normalize)
            {
                enc_memcpy(l.scales, temp->nvscales, l.outputs * sizeof(float), DECRYPT);
                enc_memcpy(l.rolling_mean, temp->rollxmean, l.outputs * sizeof(float), DECRYPT);
                enc_memcpy(l.rolling_variance, temp->rollxvar, l.outputs * sizeof(float), DECRYPT);
            }

            temp = temp->next;
            break;
        }

        case BATCHNORM:
        {

            enc_memcpy(l.scales, temp->nvscales, l.c * sizeof(float), DECRYPT);
            enc_memcpy(l.rolling_mean, temp->rollxmean, l.c * sizeof(float), DECRYPT);
            enc_memcpy(l.rolling_variance, temp->rollxvar, l.c * sizeof(float), DECRYPT);

            temp = temp->next;
            break;
        }

        default:
            break;
        }
    }
}

/**
 * Mirrors/flushes network params from the enclave to persistent memory
 * i.e net --> nv_model
 */
void NVModel::mirror_out(network *net, float *avg_loss)
{
    /**
     * We traverse the persistent linked list of layers 
     * and mirror the corresponding weights from the enclave neural net to persistent memory
     * We do the whole "mirror" operation in one transaction to ensure we have a consistent
     * neural network i.e no layers with undefined params or params from different training epochs
     * The above scenario will lead to undefined behaviour as the next training will not resume from a consistent point
     */

    Layer *temp = head;

    int num = 0;
    TM_WRITE_TRANSACTION([&]() {
        epoch = *(net->seen);
        aloss = *avg_loss;
        for (int i = 0; i < net->n; i++)
        {
            if (temp == nullptr)
                break;
            layer l = net->layers[i];
            if (l.dontsave)
                continue;
            switch (l.type)
            {
            case CONVOLUTIONAL:
            case DECONVOLUTIONAL:
            {

                num = l.nweights;
                enc_memcpy(temp->nvbiases, l.biases, l.n * sizeof(float), ENCRYPT);
                enc_memcpy(temp->nvweights, l.weights, num * sizeof(float), ENCRYPT);

                if (l.batch_normalize)
                {
                    enc_memcpy(temp->nvscales, l.scales, l.n * sizeof(float), ENCRYPT);
                    enc_memcpy(temp->rollxmean, l.rolling_mean, l.n * sizeof(float), ENCRYPT);
                    enc_memcpy(temp->rollxvar, l.rolling_variance, l.n * sizeof(float), ENCRYPT);
                }

                temp = temp->next;
                break;
            }

            case CONNECTED:
            {

                enc_memcpy(temp->nvbiases, l.biases, l.outputs * sizeof(float), ENCRYPT);
                enc_memcpy(temp->nvweights, l.weights, l.outputs * l.inputs * sizeof(float), ENCRYPT);
                if (l.batch_normalize)
                {
                    enc_memcpy(temp->nvscales, l.scales, l.outputs * sizeof(float), ENCRYPT);
                    enc_memcpy(temp->rollxmean, l.rolling_mean, l.outputs * sizeof(float), ENCRYPT);
                    enc_memcpy(temp->rollxvar, l.rolling_variance, l.outputs * sizeof(float), ENCRYPT);
                }

                temp = temp->next;
                break;
            }

            case BATCHNORM:
            {

                enc_memcpy(temp->nvscales, l.scales, l.c * sizeof(float), ENCRYPT);
                enc_memcpy(temp->rollxmean, l.rolling_mean, l.c * sizeof(float), ENCRYPT);
                enc_memcpy(temp->rollxvar, l.rolling_variance, l.c * sizeof(float), ENCRYPT);

                temp = temp->next;
                break;
            }

            default:
                break;
            }
        }
    });
}
