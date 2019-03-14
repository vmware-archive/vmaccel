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
#include "vmaccel_types_desc.hpp"

#include "log_level.h"


using namespace std;

int main(int argc, char **argv) {
   VMAccelAllocator<VMAccelDesc, VMAccelDescCmp> *descMgr = NULL;
   VMAccelAllocateStatus parent;
   VMAccelAllocateStatus alloc[2048];
   VMAccelDesc desc;
   VMAccelDesc val;

   descMgr = new VMAccelAllocator<VMAccelDesc, VMAccelDescCmp>(2048);

   Log("%s: Running self-test of allocator for VMAccelDesc...\n", __FUNCTION__);

   memset(&desc, 0, sizeof(desc));
   memset(&val, 0, sizeof(val));

   // Register the resource.
   desc.capacity.megaFlops = 65535;
   desc.capacity.megaOps = 1;
   parent = *descMgr->Register(&desc);
   assert(parent.status == VMACCEL_SUCCESS);

   // Allocate half+1 of the resource.
   desc.capacity.megaFlops = 32768;
   desc.capacity.megaOps = 0;
   alloc[0] = *descMgr->Alloc(parent.id, &desc, val);
   assert(alloc[0].status == VMACCEL_SUCCESS);
   Log("%s: descMgr.Alloc(%d, ...) -> %d\n", __FUNCTION__,
       desc.capacity.megaFlops, val.capacity.megaFlops);

   // Try to unregister the resource with pending consumers.
   assert(descMgr->Unregister(parent.id)->status == VMACCEL_FAIL);

   // Try to allocate another half+1 of the resource.
   assert(descMgr->Alloc(parent.id, &desc, val)->status ==
          VMACCEL_RESOURCE_UNAVAILABLE);

   // Allocate one quarter of the resource.
   desc.capacity.megaFlops = 16384;
   alloc[1] = *descMgr->Alloc(parent.id, &desc, val);
   assert(alloc[1].status == VMACCEL_SUCCESS);
   Log("%s: descMgr.Alloc(%d, ...) -> %d\n", __FUNCTION__,
       desc.capacity.megaFlops, val.capacity.megaFlops);

   // Allocate another quarter of the resource.
   assert(descMgr->Alloc(parent.id, &desc, val)->status ==
          VMACCEL_RESOURCE_UNAVAILABLE);

   // Free the quarter of the resource.
   assert(descMgr->Free(alloc[1].id));

   // Allocate half of the resource.
   desc.capacity.megaFlops = 32767;
   alloc[1] = *descMgr->Alloc(parent.id, &desc, val);
   assert(alloc[1].status == VMACCEL_SUCCESS);

   // Free all active resources.
   assert(descMgr->Free(alloc[1].id));
   assert(descMgr->Free(alloc[0].id));

   // Allocate all the resource slots.
   desc.capacity.megaFlops = 1;
   for (int i = 0; i < 2048; i++) {
      alloc[i] = *descMgr->Alloc(parent.id, &desc, val);
      assert(alloc[i].status == VMACCEL_SUCCESS);
   }

   // Allocate one more resource slot that available.
   assert(descMgr->Alloc(parent.id, &desc, val)->status == VMACCEL_FAIL);

   // Free all the resource slots.
   for (int i = 0; i < 2048; i++) {
      assert(descMgr->Free(alloc[i].id));
   }

   // Allocate a second dimension requirement, assumes less-than comparison is
   // satisify if at least one dimension is less than.
   desc.capacity.megaFlops = 1;
   desc.capacity.megaOps = 2;
   alloc[0] = *descMgr->Alloc(parent.id, &desc, val);
   assert(alloc[0].status == VMACCEL_RESOURCE_UNAVAILABLE);

   // Unregister the resource.
   assert(descMgr->Unregister(parent.id)->status == VMACCEL_SUCCESS);
   delete descMgr;

   Log("%s: Self-test complete...\n", __FUNCTION__);

   return 1;
}
