/*
 * Created on Wed Feb 19 2020
 *
 * Copyright (c) 2020 xxx xxxx, xxxx
 */

#ifndef DATA_MNIST_H
#define DATA_MNIST_H

//use C linkage for these
extern "C"
{
#include "data.h"
#include "utils.h"
#include "image.h"
#include "cuda.h"
#include "matrix.h"
#include <stdlib.h>
}

#include <cstring>
#include <cstdint>
#include <cstdio>

// basic file operations
#include <iostream>
#include <fstream>

#define ERROR()                                                                    \
    {                                                                              \
        std::cout << "I/O error in:" << __func__ << ": " << __LINE__ << std::endl; \
    }
//num of images in mnist dataset
#define NUM_IMGS_MNIST 60000
//number of classes in mnist dataset
#define NUM_CLASSES 10

uint32_t swap_bytes(uint32_t *);
data load_mnist_images(std::string path, size_t chunk_size);
matrix load_mnist_labels(std::string path, size_t chunk_size);
data load_enc_mnist_images(std::string path, size_t chunk_size);
matrix load_enc_mnist_labels(std::string path, size_t chunk_size);

#endif /* DATA_MNIST_H */
