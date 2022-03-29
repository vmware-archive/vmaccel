/******************************************************************************

Copyright (c) 2016-2022 VMware, Inc.
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

#include "vmaccel_utils.h"
#include "log_level.h"
#include "vmaccel_rpc.h"
#include "vmaccel_types_address.h"
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

/**
 * @brief Takes a difference of two timespec structures per the example
 * shown at:
 *
 * https://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
 */
struct timespec DiffTime(struct timespec *start, struct timespec *end) {
   struct timespec diff;
   int numSeconds;

   diff.tv_sec = end->tv_sec - start->tv_sec;

   if (end->tv_nsec < start->tv_nsec) {
      diff.tv_nsec = 1000000000 - start->tv_nsec + end->tv_nsec;
   } else if (end->tv_nsec - start->tv_nsec > 1000000000) {
      numSeconds = (end->tv_nsec - start->tv_nsec) / 1000000000;
      start->tv_nsec += 1000000000 * numSeconds;
      diff.tv_sec += numSeconds;
      diff.tv_nsec = end->tv_nsec - start->tv_nsec;
   } else {
      diff.tv_nsec = end->tv_nsec - start->tv_nsec;
   }

   return diff;
}

int BitMask_FindFirstZero(unsigned int bitMask) {
   int idx = 0;
   if (bitMask != 0xffffffff) {
      if ((bitMask & 0xffff) == 0xffff) {
         bitMask >>= 16;
         idx += 16;
      }
      if ((bitMask & 0xff) == 0xff) {
         bitMask >>= 8;
         idx += 8;
      }
      if ((bitMask & 0xf) == 0xf) {
         bitMask >>= 4;
         idx += 4;
      }
      if ((bitMask & 0x1) == 0) {
         return idx;
      } else if ((bitMask & 0x2) == 0) {
         idx += 1;
         return idx;
      } else if ((bitMask & 0x4) == 0) {
         idx += 2;
         return idx;
      } else if ((bitMask & 0x8) == 0) {
         idx += 3;
         return idx;
      }
   }
   return -1;
}

int BitMask_FindFirstOne(unsigned int bitMask) {
   int idx = 0;
   if (bitMask != 0x00000000) {
      if ((bitMask & 0xffff) == 0x0000) {
         bitMask >>= 16;
         idx += 16;
      }
      if ((bitMask & 0xff) == 0x00) {
         bitMask >>= 8;
         idx += 8;
      }
      if ((bitMask & 0xf) == 0x0) {
         bitMask >>= 4;
         idx += 4;
      }
      if (bitMask & 0x1) {
         return idx;
      } else if (bitMask & 0x2) {
         idx += 1;
         return idx;
      } else if (bitMask & 0x4) {
         idx += 2;
         return idx;
      } else if (bitMask & 0x8) {
         idx += 3;
         return idx;
      }
   }
   return -1;
}

IdentifierDB *IdentifierDB_Alloc(unsigned int size) {
   IdentifierDB *db = calloc(1, sizeof(IdentifierDB));
   if (db != NULL) {
      db->size = size;
      db->numWords = (size + 31) / 32;
      db->free = size;
      db->bits = calloc(1, db->numWords * sizeof(unsigned int));
   }
   return db;
}

unsigned int IdentifierDB_Count(IdentifierDB *db) {
   return db->size - db->free;
}

unsigned int IdentifierDB_Size(IdentifierDB *db) {
   return db->size;
}

void IdentifierDB_Free(IdentifierDB *db) {
   assert(db != NULL);
   free(db->bits);
   free(db);
}

static bool ActiveId(IdentifierDB *db, unsigned int id) {
   assert(db != NULL);
   if (id >= db->size) {
      return false;
   }
   return (db->bits[id / 32] & (1 << (id % 32))) != 0;
}

bool IdentifierDB_AcquireId(IdentifierDB *db, unsigned int id) {
   assert(db != NULL);
   if (id >= db->size) {
      return false;
   }
   if (ActiveId(db, id)) {
      return false;
   }
   db->bits[id / 32] |= (1 << (id % 32));
   return true;
}

bool IdentifierDB_AcquireIdRange(IdentifierDB *db, unsigned int start,
                                 unsigned int end) {
   assert(db != NULL);
   if (start >= db->size || end >= db->size) {
      return false;
   }
   for (unsigned int i = start / 32; i <= end / 32; i++) {
      unsigned int numBits = MIN(32, end - start + 1);
      unsigned int mask =
         (numBits == 32) ? 0xffffffff : ((1 << numBits) - 1) << (start % 32);
      if (db->bits[i] & mask) {
         return false;
      }
      db->bits[i] |= mask;
      start += 32 - (start % 32);
   }
   return true;
}

bool IdentifierDB_ActiveId(IdentifierDB *db, unsigned int id) {
   return ActiveId(db, id);
}

bool IdentifierDB_AllocId(IdentifierDB *db, unsigned int *id) {
   unsigned int i;
   assert(db != NULL);
   if (db->free == 0) {
      return false;
   }
   for (i = 0; i < db->numWords; i++) {
      int ret = BitMask_FindFirstZero(db->bits[i]);
      if (ret != -1) {
         db->free--;
         db->bits[i] |= (1 << ret);
         *id = (i * 32) + ret;
         return true;
      }
   }
   // Corrupted database...
   assert(0);
   return false;
}

void IdentifierDB_ReleaseId(IdentifierDB *db, unsigned int id) {
   assert(db != NULL);
   if (id >= db->size) {
      return;
   }
   if (db->bits[id / 32] & (1 << (id % 32))) {
      db->bits[id / 32] &= ~(1 << (id % 32));
      db->free++;
   } else {
      // Double free...
      assert(0);
   }
}

bool IdentifierDB_ReleaseIdRange(IdentifierDB *db, unsigned int start,
                                 unsigned int end) {
   assert(db != NULL);
   if (start >= db->size || end >= db->size) {
      return false;
   }
   for (unsigned int i = start / 32; i <= end / 32; i++) {
      unsigned int numBits = MIN(32, end - start + 1);
      unsigned int mask =
         (numBits == 32) ? 0xffffffff : ((1 << numBits) - 1) << (start % 32);
      db->bits[i] &= ~mask;
      start += 32 - (start % 32);
   }
   return true;
}

void IdentifierDB_Log(IdentifierDB *db, const char *prefix) {
   assert(db != NULL);
   for (unsigned int i = 0; i < db->numWords; i++) {
      VMACCEL_LOG("%s: bits[%d]=0x%08x\n", prefix, i, db->bits[i]);
   }
}

bool VMAccel_AddressOpaqueAddrToString(const VMAccelAddress *addr, char *out,
                                       int len) {
   // Enough to hold three digits per byte
   if (len < 4 * addr->addr.addr_len) {
      return false;
   }
   if (addr->addr.addr_len == 4) {
      // IPV4
      struct in_addr inetaddr;
      inetaddr.s_addr = *((in_addr_t *)addr->addr.addr_val);
      strcpy(out, inet_ntoa(inetaddr));
      return true;
   }
   memset(out, 0, len);
   return false;
}

bool VMAccel_AddressStringToOpaqueAddr(const char *addr, char *out, int len) {
   if (len == 4) {
      // IPV4
      assert(sizeof(in_addr_t) == 4);
      *((in_addr_t *)out) = inet_addr(addr);
      return true;
   }
   return false;
}

/*
 * Mutexes used by various vmaccel modules.
 */
pthread_mutex_t svc_compute_mutex;
pthread_mutex_t svc_data_mutex;
pthread_mutex_t svc_state_mutex;

VMAccelAllocateStatus *
vmaccel_utils_poweron_svc() {
   static VMAccelAllocateStatus clnt_res;
   pthread_mutexattr_t attr;
   if (pthread_mutexattr_init(&attr) != 0) {
      VMACCEL_WARNING("%s: Unable to initialize service mutex attributes\n", __FUNCTION__);
      return (NULL);
   }
   if (pthread_mutex_init(&svc_compute_mutex, &attr) != 0) {
      VMACCEL_WARNING("%s: Unable to initialize service compute mutex\n", __FUNCTION__);
      pthread_mutexattr_destroy(&attr);
      return (NULL);
   }
   if (pthread_mutex_init(&svc_data_mutex, &attr) != 0) {
      VMACCEL_WARNING("%s: Unable to initialize service data mutex\n", __FUNCTION__);
      pthread_mutexattr_destroy(&attr);
      return (NULL);
   }
   if (pthread_mutex_init(&svc_state_mutex, &attr) != 0) {
      VMACCEL_WARNING("%s: Unable to initialize service state mutex\n", __FUNCTION__);
      pthread_mutexattr_destroy(&attr);
      return (NULL);
   }
   pthread_mutexattr_destroy(&attr);
   clnt_res.status = VMACCEL_SUCCESS;
   return (&clnt_res);
}

VMAccelStatus *
vmaccel_utils_poweroff_svc() {
   static VMAccelStatus clnt_res;
   if (pthread_mutex_destroy(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to destroy service state mutex\n", __FUNCTION__);
      return (NULL);
   }
   if (pthread_mutex_destroy(&svc_data_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to destroy service data mutex\n", __FUNCTION__);
      return (NULL);
   }
   if (pthread_mutex_destroy(&svc_compute_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to destroy service compute mutex\n", __FUNCTION__);
      return (NULL);
   }
   clnt_res.status = VMACCEL_SUCCESS;
   return (&clnt_res);
}

void vmaccel_xdr_free(xdrproc_t proc, caddr_t ptr) {
   xdr_free(proc, ptr);
}
