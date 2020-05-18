

#ifndef _ENCLAVE_H_
#define _ENCLAVE_H_

#include <stdlib.h>
#include <assert.h>


//#include "Types.h" //types used for marshalling and enclave romulus object construction
#include "romulus/common/RomSGX.h"
#include "dnet-in/src/dnet_sgx_utils.h"
#include "crypto/crypto.h"

#if defined(__cplusplus)
extern "C" {
#endif

void ecall_nvram_worker(int val,size_t tid);
void ecall_init(void *per_out,uint8_t *base_addr_out);
void empty_ecall();
void do_work(int val,size_t tid);
void do_close();


#if defined(__cplusplus)
}
#endif

#endif /* !_ENCLAVE_H_ */
