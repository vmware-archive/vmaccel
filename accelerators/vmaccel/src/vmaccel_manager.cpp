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

/*
 * vmaccel_manager.cpp
 *
 * Accelerator manager for active accelerators.
 */
#include <iostream>
#include <stdbool.h>
#include <string>

extern "C" {
#include "vmaccel_rpc.h"
#include "vmaccel_manager.h"
#include "vmaccel_utils.h"
}

#include "vmaccel_manager.hpp"
#include "vmaccel_types_int.hpp"
#include "vmaccel_types_desc.hpp"
#include "vmaccel_types_allocrange.hpp"

#include "log_level.h"


using namespace std;

VMAccelAllocator<VMAccelDesc, VMAccelDescCmp> *accelMgr = NULL;
VMAccelAllocator<AllocRange, AllocRangeCmp> *memMgr = NULL;

unsigned int vmaccel_manager_poweron() {
   VMAccelAllocator<Int, IntCmp> *intMgr = NULL;
   VMAccelAllocateStatus parent;
   VMAccelAllocateStatus alloc[2048];
   Int desc;
   Int val;

   intMgr = new VMAccelAllocator<Int, IntCmp>(2048);

   VMACCEL_LOG("%s: Running self-test of allocator...\n", __FUNCTION__);
   desc.x = 65535;
   parent = *intMgr->Register(&desc);
   assert(parent.status == VMACCEL_SUCCESS);
   desc.x = 32768;
   alloc[0] = *intMgr->Alloc(parent.id, &desc, val);
   assert(alloc[0].status == VMACCEL_SUCCESS);
   VMACCEL_LOG("%s: intMgr.Alloc(%d, ...) -> %d\n", __FUNCTION__, desc.x,
               val.x);
   assert(intMgr->Unregister(parent.id)->status == VMACCEL_FAIL);
   assert(intMgr->Alloc(parent.id, &desc, val)->status ==
          VMACCEL_RESOURCE_UNAVAILABLE);
   desc.x = 16384;
   alloc[1] = *intMgr->Alloc(parent.id, &desc, val);
   assert(alloc[1].status == VMACCEL_SUCCESS);
   VMACCEL_LOG("%s: intMgr.Alloc(%d, ...) -> %d\n", __FUNCTION__, desc.x,
               val.x);
   assert(intMgr->Alloc(parent.id, &desc, val)->status ==
          VMACCEL_RESOURCE_UNAVAILABLE);
   assert(intMgr->Free(alloc[1].id));
   desc.x = 32767;
   alloc[1] = *intMgr->Alloc(parent.id, &desc, val);
   assert(alloc[1].status == VMACCEL_SUCCESS);
   assert(intMgr->Free(alloc[1].id));
   assert(intMgr->Free(alloc[0].id));

   desc.x = 1;
   for (int i = 0; i < 2048; i++) {
      alloc[i] = *intMgr->Alloc(parent.id, &desc, val);
      assert(alloc[i].status == VMACCEL_SUCCESS);
   }

   assert(intMgr->Alloc(parent.id, &desc, val)->status == VMACCEL_FAIL);

   for (int i = 0; i < 2048; i++) {
      assert(intMgr->Free(alloc[i].id));
   }

   assert(intMgr->Unregister(parent.id)->status == VMACCEL_SUCCESS);
   delete intMgr;
   VMACCEL_LOG("%s: Self-test complete...\n", __FUNCTION__);

   accelMgr = new VMAccelAllocator<VMAccelDesc, VMAccelDescCmp>(
      VMACCEL_MAX_ACCELERATORS);

   memMgr =
      new VMAccelAllocator<AllocRange, AllocRangeCmp>(VMACCEL_MAX_ACCELERATORS);

   return (accelMgr != NULL) && (memMgr != NULL) ? VMACCEL_SUCCESS
                                                 : VMACCEL_FAIL;
}

unsigned int vmaccel_manager_poweroff() {
   if (memMgr != NULL) {
      delete memMgr;
      memMgr = NULL;
   }

   if (accelMgr != NULL) {
      delete accelMgr;
      accelMgr = NULL;
      return VMACCEL_SUCCESS;
   }

   return VMACCEL_FAIL;
}

VMAccelAllocateStatus *vmaccel_manager_register(VMAccelDesc *desc) {
   static VMAccelAllocateStatus result;

   memset(&result, 0, sizeof(VMAccelAllocateStatus));

   if (accelMgr == NULL) {
      VMACCEL_WARNING("No manager object found\n");
      result.status = VMACCEL_FAIL;
      return &result;
   }

   Log_VMAccelDesc("vmaccel_manager_register: desc:", desc);

   return accelMgr->Register(desc);
}

VMAccelStatus *vmaccel_manager_unregister(VMAccelId id) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(VMAccelStatus));

   if (accelMgr == NULL) {
      VMACCEL_WARNING("No manager object found\n");
      result.status = VMACCEL_FAIL;
      return &result;
   }

   VMACCEL_LOG("vmaccel_manager_unregister: id=%u\n", id);

   return accelMgr->Unregister(id);
}

VMAccelAllocateStatus *vmaccel_manager_alloc(VMAccelDesc *desc) {
   static VMAccelAllocateStatus result;
   VMAccelAllocateStatus *res;

   memset(&result, 0, sizeof(VMAccelAllocateStatus));

   if (accelMgr == NULL) {
      VMACCEL_WARNING("No manager object found\n");
      result.status = VMACCEL_FAIL;
      return &result;
   }

   Log_VMAccelDesc("vmaccel_manager_alloc: desc:", desc);

   res = accelMgr->Alloc(desc->parentId, desc, result.desc);

   result.status = res->status;
   result.id = res->id;

   return &result;
}

VMAccelStatus *vmaccel_manager_free(VMAccelId id) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(VMAccelStatus));

   if (accelMgr == NULL) {
      VMACCEL_WARNING("No manager object found\n");
      result.status = VMACCEL_FAIL;
      return &result;
   }

   VMACCEL_LOG("vmaccel_manager_free: id=%u\n", id);

   return accelMgr->Free(id);
}

bool vmaccel_manager_wait_for_fence(VMAccelId id) {
   return true;
}
