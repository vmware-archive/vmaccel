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

#define DEBUG_OBJECT_LIFETIME 0
#define DEBUG_TEMPLATE_TYPES 0
#define DEBUG_COMPUTE_OPERATION 0
#define DEBUG_ASYNC_COMPUTE 0
#define DEBUG_PERSISTENT_SURFACES 0
#define DEBUG_SURFACE_CONSISTENCY 0
#define DEBUG_FORCE_SURFACE_CONSISTENCY 0

#if DEBUG_ASYNC_COMPUTE || DEBUG_OBJECT_LIFETIME
#define LOG_ENTRY(_ARGS) Log _ARGS
#define LOG_EXIT(_ARGS) Log _ARGS
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

int BitMask_FindFirstZero(unsigned int bitMask);
int BitMask_FindFirstOne(unsigned int bitMask);

typedef struct IdentifierDB {
   unsigned int size;
   unsigned int numWords;
   unsigned int free;
   unsigned int *bits;
} IdentifierDB;

#ifdef __cplusplus
extern "C" {
#endif

extern IdentifierDB *IdentifierDB_Alloc(unsigned int size);
extern unsigned int IdentifierDB_Count(IdentifierDB *db);
extern unsigned int IdentifierDB_Size(IdentifierDB *db);
extern void IdentifierDB_Free(IdentifierDB *db);
extern bool IdentifierDB_AcquireId(IdentifierDB *db, unsigned int id);
extern bool IdentifierDB_AcquireIdRange(IdentifierDB *db, unsigned int start,
                                        unsigned int end);
extern bool IdentifierDB_ActiveId(IdentifierDB *db, unsigned int id);
extern bool IdentifierDB_AllocId(IdentifierDB *db, unsigned int *id);
extern void IdentifierDB_ReleaseId(IdentifierDB *db, unsigned int id);
extern bool IdentifierDB_ReleaseIdRange(IdentifierDB *db, unsigned int start,
                                        unsigned int end);
extern void IdentifierDB_Log(IdentifierDB *db, const char *prefix);

extern bool VMAccel_AddressOpaqueAddrToString(const VMAccelAddress *addr,
                                              char *out, int len);
extern bool VMAccel_AddressStringToOpaqueAddr(const char *addr, char *out,
                                              int len);

#ifdef __cplusplus
}
#endif

#endif /* _VMACCEL_UTILS_H_ */
