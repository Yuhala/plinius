#ifndef CHECKS_H
#define CHECKS_H

#include "Enclave_t.h"
//for pointer checks
#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */
#include "sgx_lfence.h" /* for sgx_lfence */
#include "sgx_error.h"

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz))){\
		printf("SGX_ERROR_INVALID_PARAMETER\n");\
        abort();}\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz))){	\
		printf("SGX_ERROR_INVALID_PARAMETER\n");\
        abort();}\
} while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_within_enclave((ptr), (siz))){	\
		printf("SGX_ERROR_INVALID_PARAMETER\n");\
        abort();}\
} while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (	\
	((a) += (b)) < (b)	\
)

#endif /* CHECKS_H */
