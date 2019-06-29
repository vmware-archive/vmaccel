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

#include <iostream>
#include <stdbool.h>
#include <string>

#include "vmaccel_allocator.hpp"
#include "vmaccel_types_int.hpp"

#include "log_level.h"

using namespace std;

int main(int argc, char **argv) {
   VMAccelAllocator<Int, IntCmp> *intMgr = NULL;
   VMAccelAllocateStatus parent;
   VMAccelAllocateStatus alloc[2048];
   Int desc;
   Int val;

   intMgr = new VMAccelAllocator<Int, IntCmp>(2048);

   VMACCEL_LOG("%s: Running self-test of allocator for Int...\n", __FUNCTION__);

   // Register the resource.
   desc.x = 65535;
   parent = *intMgr->Register(&desc);
   assert(parent.status == VMACCEL_SUCCESS);

   // Allocate half+1 of the resource.
   desc.x = 32768;
   alloc[0] = *intMgr->Alloc(parent.id, &desc, val);
   assert(alloc[0].status == VMACCEL_SUCCESS);
   VMACCEL_LOG("%s: intMgr.Alloc(%d, ...) -> %d\n", __FUNCTION__, desc.x,
               val.x);

   // Try to unregister the resource with pending consumers.
   assert(intMgr->Unregister(parent.id)->status == VMACCEL_FAIL);

   // Try to allocate another half+1 of the resource.
   assert(intMgr->Alloc(parent.id, &desc, val)->status ==
          VMACCEL_RESOURCE_UNAVAILABLE);

   // Allocate one quarter of the resource.
   desc.x = 16384;
   alloc[1] = *intMgr->Alloc(parent.id, &desc, val);
   assert(alloc[1].status == VMACCEL_SUCCESS);
   VMACCEL_LOG("%s: intMgr.Alloc(%d, ...) -> %d\n", __FUNCTION__, desc.x,
               val.x);

   // Allocate another quarter of the resource.
   assert(intMgr->Alloc(parent.id, &desc, val)->status ==
          VMACCEL_RESOURCE_UNAVAILABLE);

   // Free the quarter of the resource.
   assert(intMgr->Free(alloc[1].id));

   // Allocate half of the resource.
   desc.x = 32767;
   alloc[1] = *intMgr->Alloc(parent.id, &desc, val);
   assert(alloc[1].status == VMACCEL_SUCCESS);

   // Free all active resources.
   assert(intMgr->Free(alloc[1].id));
   assert(intMgr->Free(alloc[0].id));

   // Allocate all the resource slots.
   desc.x = 1;
   for (int i = 0; i < 2048; i++) {
      alloc[i] = *intMgr->Alloc(parent.id, &desc, val);
      assert(alloc[i].status == VMACCEL_SUCCESS);
   }

   // Allocate one more resource slot that available.
   assert(intMgr->Alloc(parent.id, &desc, val)->status == VMACCEL_FAIL);

   // Free all the resource slots.
   for (int i = 0; i < 2048; i++) {
      assert(intMgr->Free(alloc[i].id));
   }

   // Unregister the resource.
   assert(intMgr->Unregister(parent.id)->status == VMACCEL_SUCCESS);
   delete intMgr;

   VMACCEL_LOG("%s: Self-test complete...\n", __FUNCTION__);

   return 0;
}
