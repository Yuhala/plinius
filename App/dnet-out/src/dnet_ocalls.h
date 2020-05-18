/*
 * Created on Fri Feb 14 2020
 *
 * Copyright (c) 2020 xxx xxxx, xxxx
 */

#ifndef DNET_OCALLS_H
#define DNET_OCALLS_H

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "darknet.h"

//edgerator header file
#include "Enclave_u.h"

#if defined(__cplusplus)

extern "C"
{
#endif

    void ocall_free_sec(section *sec);
    void ocall_free_list(list *list);
    void ocall_print_string(const char *str);
    void ocall_open_file(const char *filename, flag oflag);
    void ocall_close_file();
    void ocall_fread(void *ptr, size_t size, size_t nmemb);
    void ocall_fwrite(void *ptr, size_t size, size_t nmemb);

#if defined(__cplusplus)
}
#endif

#endif /* DNET_OCALLS_H */
