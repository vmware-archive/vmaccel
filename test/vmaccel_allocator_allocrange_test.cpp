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
#include "vmaccel_types_allocrange.hpp"

#include "log_level.h"


using namespace std;

int main(int argc, char **argv) {
   VMAccelAllocator<AllocRange, AllocRangeCmp> *rangeMgr = NULL;
   VMAccelAllocateStatus parent;
   VMAccelAllocateStatus alloc[2048];
   AllocRange range;
   AllocRange val;

   rangeMgr = new VMAccelAllocator<AllocRange, AllocRangeCmp>(2048);

   Log("%s: Running self-test of allocator for AllocRange...\n", __FUNCTION__);

   memset(&range, 0, sizeof(range));
   memset(&val, 0, sizeof(val));

   // Register the resource.
   range.size = 65535;
   range.begin = 0;
   range.end = range.size;
   parent = *rangeMgr->Register(&range);
   assert(parent.status == VMACCEL_SUCCESS);

   // Allocate half+1 of the resource.
   range.size = 32768;
   range.begin = 0;
   range.end = 123456789;
   alloc[0] = *rangeMgr->Alloc(parent.id, &range, val);
   assert(alloc[0].status == VMACCEL_SUCCESS);
   Log("%s: rangeMgr.Alloc(%zu, ...) -> [%zu ... %zu]\n", __FUNCTION__,
       range.size, val.begin, val.end);

   // Try to unregister the resource with pending consumers.
   assert(rangeMgr->Unregister(parent.id)->status == VMACCEL_FAIL);

   // Try to allocate another half+1 of the resource.
   assert(rangeMgr->Alloc(parent.id, &range, val)->status ==
          VMACCEL_RESOURCE_UNAVAILABLE);

   // Allocate one quarter of the resource.
   range.size = 16384;
   alloc[1] = *rangeMgr->Alloc(parent.id, &range, val);
   assert(alloc[1].status == VMACCEL_SUCCESS);
   Log("%s: rangeMgr.Alloc(%zu, ...) -> [%zu ... %zu]\n", __FUNCTION__,
       range.size, val.begin, val.end);

   // Allocate another quarter of the resource.
   assert(rangeMgr->Alloc(parent.id, &range, val)->status ==
          VMACCEL_RESOURCE_UNAVAILABLE);

   // Free the quarter of the resource.
   assert(rangeMgr->Free(alloc[1].id));

   // Allocate half of the resource.
   range.size = 32767;
   alloc[1] = *rangeMgr->Alloc(parent.id, &range, val);
   assert(alloc[1].status == VMACCEL_SUCCESS);

   Log("%s: rangeMgr.Alloc(%zu, ...) -> [%zu ... %zu]\n", __FUNCTION__,
       range.size, val.begin, val.end);

   // Free all active resources.
   assert(rangeMgr->Free(alloc[1].id));
   assert(rangeMgr->Free(alloc[0].id));

   // Allocate all the resource slots.
   range.size = 1;
   for (int i = 0; i < 2048; i++) {
      alloc[i] = *rangeMgr->Alloc(parent.id, &range, val);
      assert(alloc[i].status == VMACCEL_SUCCESS);
   }

   // Allocate one more resource slot that available.
   assert(rangeMgr->Alloc(parent.id, &range, val)->status == VMACCEL_FAIL);

   // Free all the resource slots.
   for (int i = 0; i < 2048; i++) {
      assert(rangeMgr->Free(alloc[i].id));
   }

   // Unregister the resource.
   assert(rangeMgr->Unregister(parent.id)->status == VMACCEL_SUCCESS);
   delete rangeMgr;

   Log("%s: Self-test complete...\n", __FUNCTION__);

   return 0;
}
