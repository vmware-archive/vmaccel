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
 * vmaccel_allocator.cpp
 *
 * Accelerator object allocator for active accelerators, using class templates
 * for objects defined in vmaccel_type_*.hpp. These objects will adhere to
 * accelerator lifetime semantics, and thus may be referenced in one or more
 * accelerator queues at any given time.
 *
 * Objects are subdivided using number theory notation:
 *
 *    a - registered allocation
 *    d - resulting reservation
 *    q - non-zero positive integer when reservation succeeds
 *    r - remainder of the registered allocation after reservation
 *
 * E.g.
 *
 *    a = dq + r
 */
#include <iostream>
#include <stdbool.h>
#include <string>

#include "vmaccel_rpc.h"
#include "vmaccel_manager.h"
#include "vmaccel_manager.hpp"
#include "vmaccel_allocator.hpp"
#include "vmaccel_types_int.hpp"
#include "vmaccel_types_desc.hpp"
#include "vmaccel_types_allocrange.hpp"
#include "vmaccel_utils.h"

#include "log_level.h"


#define DEFER_FREE 1

using namespace std;

template class VMAccelAllocator<Int, IntCmp>;
template class VMAccelAllocator<AllocRange, AllocRangeCmp>;
template class VMAccelAllocator<VMAccelDesc, VMAccelDescCmp>;

template <class T, typename C>
void VMAccelAllocator<T, C>::CoalesceFreed() {
   VMAccelObject<T> a;

   /*
    * Look at the freed queue and coalesce elements.
    *
    * In practice, an in-order alloc pattern will place contiguous
    * allocations in temporal order of completion.
    */
   while (!freed.empty()) {
      a = freed.front();

      if (!vmaccel_manager_wait_for_fence(a.GetFenceId())) {
         Warning("Unable to wait for fence %d\n", a.GetFenceId());
         continue;
      }

      freed.pop();

      if (!FreeObj(free, a)) {
         Warning("Unable to add freed object to free set...\n");
      }
   }
}

template <class T, typename C>
bool VMAccelAllocator<T, C>::FindFreed(VMAccelObject<T> &req,
                                       VMAccelObject<T> &d) {
   VMAccelObject<T> a;
   bool found = false;

   while (!found && !freed.empty()) {
      a = freed.front();

      if (!vmaccel_manager_wait_for_fence(a.GetFenceId())) {
         Warning("Unable to wait for fence %d\n", a.GetFenceId());
         continue;
      }

      freed.pop();

      if (a.GetParentId() != req.GetParentId()) {
         Warning("Unable to add freed object to free set...\n");
         continue;
      }

      if (req <= a) {
         const T in = a.GetObj();
         T div, r;
         if (Reserve(a.GetObj(), req.GetObj(), div, r)) {
            if (!IsEmpty(r)) {
               if (!FreeObj(free, VMAccelObject<T>(req.GetParentId(), &r))) {
                  Warning("Unable to add remainder to free set...\n");
               }
            }
            d = VMAccelObject<T>(req.GetParentId(), &div);
            found = true;
         }
         break;
      } else if (FreeObj(free, a)) {
         auto it = free.lower_bound(req);
         if (it != free.end()) {
            T div, r;
            if (Reserve(it->GetObj(), req.GetObj(), div, r)) {
               free.erase(it);
               d = VMAccelObject<T>(req.GetParentId(), &div);
               if (!IsEmpty(r)) {
                  if (!FreeObj(free, VMAccelObject<T>(req.GetParentId(), &r))) {
                     Warning("Unable to add remainder to free set...\n");
                  }
               }
               found = true;
            }
         }
      } else {
         Warning("Unable to add remainder to free set...\n");
         continue;
      }
   }

   return found;
}

template <class T, typename C>
VMAccelAllocateStatus *VMAccelAllocator<T, C>::Register(T *a) {
   static VMAccelAllocateStatus result;

   memset(&result, 0, sizeof(result));

   if (!IdentifierDB_AllocId(registeredIds, &result.id)) {
      Warning("Unable to allocate a registered accelerator ID\n");
      result.status = VMACCEL_FAIL;
      return &result;
   }

   capacity[result.id] += *a;
   registered[result.id] = *a;
   refCount[result.id] = 0;

   if (!FreeObj(free, VMAccelObject<T>(result.id, a))) {
      Warning("Unable to add object to free set...\n");
      IdentifierDB_ReleaseId(registeredIds, result.id);
      result.status = VMACCEL_FAIL;
   } else {
      result.status = VMACCEL_SUCCESS;
   }

   return &result;
}

template <class T, typename C>
VMAccelStatus *VMAccelAllocator<T, C>::Unregister(VMAccelId id) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   if (refCount[id] != 0) {
      result.status = VMACCEL_FAIL;
      return &result;
   }

   /*
    * Assumes no pending objects on the Accelerator.
    */
   CoalesceFreed();

   assert(refCount[id] == 0);

   IdentifierDB_ReleaseId(registeredIds, id);

   for (auto it = free.begin(); it != free.end();) {
      if (it->GetParentId() == id) {
         it = free.erase(it);
      } else {
         ++it;
      }
   }

   result.status = VMACCEL_SUCCESS;

   return &result;
}

template <class T, typename C>
VMAccelAllocateStatus *VMAccelAllocator<T, C>::Alloc(VMAccelId parentId, T *a,
                                                     T &d) {
   static VMAccelAllocateStatus result;
   VMAccelObject<T> req(parentId, a);
   VMAccelObject<T> obj;
   VMAccelId registeredId;
   VMAccelId externalId;
   bool found = false;

   if (!IdentifierDB_AllocId(externalIds, &externalId)) {
      Warning("Unable to allocate an external accelerator ID\n");
      result.status = VMACCEL_FAIL;
      return &result;
   }

   auto it = free.lower_bound(req);
   if (it != free.end()) {
      T div, r;
      if (Reserve(it->GetObj(), req.GetObj(), div, r)) {
         free.erase(it);
         obj = VMAccelObject<T>(req.GetParentId(), &div);
         if (!IsEmpty(r)) {
            if (!FreeObj(free, VMAccelObject<T>(req.GetParentId(), &r))) {
               Warning("Unable to add remainder to free set...\n");
            }
         }
         found = true;
      }
   } else if (FindFreed(req, obj)) {
      found = true;
   }

   /*
    * No matching allocation.
    */
   if (!found) {
      Warning("No matching allocation found...\n");
      IdentifierDB_ReleaseId(externalIds, externalId);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return &result;
   }

   d = obj.GetObj();
   registeredId = obj.GetParentId();
   allocated[externalId] = obj;

   capacity[registeredId] -= d;
   load[registeredId] += d;
   refCount[registeredId]++;

   result.id = externalId;
   result.status = VMACCEL_SUCCESS;

   return &result;
}

template <class T, typename C>
VMAccelStatus *VMAccelAllocator<T, C>::Free(VMAccelId id) {
   static VMAccelStatus result;
   VMAccelId registeredId;

   assert(IdentifierDB_ActiveId(externalIds, id));

   registeredId = allocated[id].GetParentId();

   freed.push(allocated[id]);

#if !DEFER_FREE
   // If we don't want to defer the free, wait until the object's Accelerator
   // lifetime is complete and immediately coalesce.
   CoalesceFreed();
#endif

   capacity[registeredId] += allocated[id].GetObj();
   load[registeredId] -= allocated[id].GetObj();
   refCount[registeredId]--;

   // Don't retain the previous information in this vector entry.
   allocated[id] = VMAccelObject<T>();

   IdentifierDB_ReleaseId(externalIds, id);

   result.status = VMACCEL_SUCCESS;

   return &result;
}
