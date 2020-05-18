/**
 * Modified version of the original file from romulus
 * xxx xxxx
 */


/* Definitions for different Persistent Transactional Memories */


#ifdef ROMULUS_LOG_PTM
#include "romuluslog/RomulusLogSGX.hpp"
#define TM_WRITE_TRANSACTION   romuluslog::RomulusLog::write_transaction
#define TM_READ_TRANSACTION    romuluslog::RomulusLog::read_transaction
#define TM_BEGIN_TRANSACTION() romuluslog::gRomLog.begin_transaction()
#define TM_END_TRANSACTION()   romuluslog::gRomLog.end_transaction()
#define TM_ALLOC               romuluslog::RomulusLog::alloc
#define TM_FREE                romuluslog::RomulusLog::free
#define TM_PMALLOC             romuluslog::RomulusLog::pmalloc
#define TM_PFREE               romuluslog::RomulusLog::pfree
#define TM_TYPE                romuluslog::persist
#define TM_NAME                romuluslog::RomulusLog::className
#define TM_CONSISTENCY_CHECK   romuluslog::RomulusLog::consistency_check
#define TM_INIT                romuluslog::RomulusLog::init
#endif


