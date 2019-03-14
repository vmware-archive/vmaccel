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

#include <assert.h>
#include <stdlib.h>
#include "vmaccel_utils.h"

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
      db->numWords = (size + 31) / 32;
      db->free = size;
      db->bits = calloc(1, db->numWords * sizeof(unsigned int));
   }

   return db;
}

void IdentifierDB_Free(IdentifierDB *db) {
   assert(db != NULL);
   free(db->bits);
   free(db);
}

static bool ActiveId(IdentifierDB *db, unsigned int id) {
   assert(db != NULL);
   return (db->bits[id / 32] & (1 << (id % 32))) != 0;
}

bool IdentifierDB_AcquireId(IdentifierDB *db, unsigned int id) {
   assert(db != NULL);
   if (ActiveId(db, id)) {
      return false;
   }
   db->bits[id / 32] |= (1 << (id % 32));
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

   if (db->bits[id / 32] & (1 << (id % 32))) {
      db->bits[id / 32] &= ~(1 << (id % 32));
      db->free++;
   } else {
      // Double free...
      assert(0);
   }
}
