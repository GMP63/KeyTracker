/**
 * @file test.h
 * @brief Unit tests constants.
 * @author Guillermo M. Paris
 * @date 2019-12-10
 */

#include <cstddef>
#include "MapManager.h"
#include "MessageThreadedQueue.h"

extern const long CLOCKS_PER_MILLISEC;
extern const size_t maxNumbers;

void keyInsertion(MapManager* pMgr, MessageThreadedQueue* pMtq, bool direct, size_t burstSize);
