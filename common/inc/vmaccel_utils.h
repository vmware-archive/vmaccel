/******************************************************************************

Copyright (c) 2016-2019 VMware, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#ifndef _VMACCEL_UTILS_H_
#define _VMACCEL_UTILS_H_ 1

#include <stdbool.h>
#include <time.h>

#define DEBUG_OBJECT_LIFETIME 0
#define DEBUG_TEMPLATE_TYPES 0
#define DEBUG_COMPUTE_OPERATION 0
#define DEBUG_ASYNC_COMPUTE 0
#define DEBUG_PERSISTENT_SURFACES 0
#define DEBUG_SURFACE_CONSISTENCY 0
#define DEBUG_FORCE_SURFACE_CONSISTENCY 0
#define DEBUG_STREAMS 0
#define DEBUG_STATISTICS 0

#if DEBUG_ASYNC_COMPUTE || DEBUG_OBJECT_LIFETIME
#define LOG_ENTRY(_ARGS) printf _ARGS
#define LOG_EXIT(_ARGS) printf _ARGS
#else
#define LOG_ENTRY(_ARGS)
#define LOG_EXIT(_ARGS)
#endif

#ifndef MIN
#define MIN(_A, _B) (_A < _B) ? _A : _B
#endif

#ifndef MAX
#define MAX(_A, _B) (_A > _B) ? _A : _B
#endif

struct timespec DiffTime(struct timespec *start, struct timespec *end);

#if DEBUG_STATISTICS
#define DECLARE_STAT(_NAME)                                                    \
   struct timespec min##_NAME##Time;                                           \
   struct timespec max##_NAME##Time;                                           \
   float total##_NAME##TimeMS;                                                 \
   unsigned int num##_NAME##Instances = 0

#define STAT_TO_MS(_STAT)                                                      \
   _STAT.tv_sec * 1000.0f + (_STAT.tv_nsec != 0)                               \
      ? (double)_STAT.tv_nsec / 1000000.0f                                     \
      : 0.0f

#define START_STAT(_NAME)                                                      \
   struct timespec _NAME##StartTime;                                           \
   struct timespec _NAME##EndTime;                                             \
   struct timespec _NAME##DiffTime;                                            \
   clock_gettime(CLOCK_REALTIME, &_NAME##StartTime)

#define END_STAT(_NAME)                                                        \
   clock_gettime(CLOCK_REALTIME, &_NAME##EndTime);                             \
   _NAME##DiffTime = DiffTime(&_NAME##StartTime, &_NAME##EndTime);             \
   if (num##_NAME##Instances == 0) {                                           \
      min##_NAME##Time = _NAME##DiffTime;                                      \
      max##_NAME##Time = _NAME##DiffTime;                                      \
   } else {                                                                    \
      if ((_NAME##DiffTime.tv_sec < min##_NAME##Time.tv_sec) ||                \
          (_NAME##DiffTime.tv_sec == min##_NAME##Time.tv_sec &&                \
           _NAME##DiffTime.tv_nsec < min##_NAME##Time.tv_nsec)) {              \
         min##_NAME##Time = _NAME##DiffTime;                                   \
      }                                                                        \
      if ((_NAME##DiffTime.tv_sec > max##_NAME##Time.tv_sec) ||                \
          (_NAME##DiffTime.tv_sec == max##_NAME##Time.tv_sec &&                \
           _NAME##DiffTime.tv_nsec > max##_NAME##Time.tv_nsec)) {              \
         max##_NAME##Time = _NAME##DiffTime;                                   \
      }                                                                        \
   }                                                                           \
   total##_NAME##TimeMS += STAT_TO_MS(_NAME##DiffTime);                        \
   num##_NAME##Instances++

#define PRINT_STAT(_NAME)                                                      \
   printf("%s: Count=%d, Min=%f ms, Max=%f ms, Avg=%f ms, Total=%f ms\n",      \
          #_NAME, num##_NAME##Instances, STAT_TO_MS(min##_NAME##Time),         \
          STAT_TO_MS(max##_NAME##Time),                                        \
          total##_NAME##TimeMS / num##_NAME##Instances, total##_NAME##TimeMS)

#else
#define DECLARE_STAT(_NAME)
#define START_STAT(_NAME)
#define END_STAT(_NAME)
#define PRINT_STAT(_NAME)
#endif

int BitMask_FindFirstZero(unsigned int bitMask);
int BitMask_FindFirstOne(unsigned int bitMask);

typedef struct IdentifierDB {
   unsigned int size;
   unsigned int numWords;
   unsigned int free;
   unsigned int *bits;
} IdentifierDB;

IdentifierDB *IdentifierDB_Alloc(unsigned int size);
unsigned int IdentifierDB_Count(IdentifierDB *db);
unsigned int IdentifierDB_Size(IdentifierDB *db);
void IdentifierDB_Free(IdentifierDB *db);
bool IdentifierDB_AcquireId(IdentifierDB *db, unsigned int id);
bool IdentifierDB_AcquireIdRange(IdentifierDB *db, unsigned int start,
                                 unsigned int end);
bool IdentifierDB_ActiveId(IdentifierDB *db, unsigned int id);
bool IdentifierDB_AllocId(IdentifierDB *db, unsigned int *id);
void IdentifierDB_ReleaseId(IdentifierDB *db, unsigned int id);
bool IdentifierDB_ReleaseIdRange(IdentifierDB *db, unsigned int start,
                                 unsigned int end);
void IdentifierDB_Log(IdentifierDB *db, const char *prefix);

bool VMAccel_AddressOpaqueAddrToString(const VMAccelAddress *addr, char *out,
                                       int len);
bool VMAccel_AddressStringToOpaqueAddr(const char *addr, char *out, int len);

bool VMAccel_IsLocal();

void vmaccel_xdr_free(xdrproc_t proc, caddr_t ptr);

#endif /* _VMACCEL_UTILS_H_ */
