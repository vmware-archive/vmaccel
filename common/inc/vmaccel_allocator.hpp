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
 * vmaccel_allocator.hpp
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

#ifndef _VMACCEL_ALLOCATOR_HPP_
#define _VMACCEL_ALLOCATOR_HPP_ 1

#include "vmaccel_rpc.h"
#include "vmaccel_manager.h"
#include "vmaccel_utils.h"
#include <algorithm>
#include <cassert>
#include <queue>
#include <set>
#include <vector>

#include "log_level.h"

template <class T>
class VMAccelObject {

public:
   /*
    * Default constructor.
    */
   VMAccelObject<T>() {
      parentId = VMACCEL_INVALID_ID;
      fenceId = VMACCEL_INVALID_ID;
      Constructor(obj);
   }

   /*
    * Copy constructors.
    */
   VMAccelObject<T>(VMAccelId id, T *o) {
      parentId = id;
      fenceId = VMACCEL_INVALID_ID;
      Constructor(obj);
      DeepCopy(obj, *o);
   }

   VMAccelObject<T>(const VMAccelObject<T> &o) {
      parentId = o.parentId;
      fenceId = o.fenceId;
      Constructor(obj);
      DeepCopy(obj, o.obj);
   }

   /*
    * Move constructors.
    */
   VMAccelObject<T>(VMAccelObject<T> &&o) {
      parentId = o.parentId;
      o.parentId = VMACCEL_INVALID_ID;
      fenceId = o.fenceId;
      o.fenceId = VMACCEL_INVALID_ID;
      Move(obj, o.obj);
   }

   VMAccelObject<T> &operator=(const VMAccelObject<T> &o) {
      parentId = o.parentId;
      fenceId = o.fenceId;
      DeepCopy(obj, o.obj);
      return *this;
   }

   VMAccelObject<T> &operator=(VMAccelObject<T> &&o) {
      parentId = o.parentId;
      o.parentId = VMACCEL_INVALID_ID;
      fenceId = o.fenceId;
      o.fenceId = VMACCEL_INVALID_ID;
      Move(obj, o.obj);
      return *this;
   }

   /*
    * Destructor.
    */
   ~VMAccelObject<T>() { Destructor(obj); }

   bool operator==(const VMAccelObject<T> &rhs) const { return obj == rhs.obj; }
   bool operator!=(const VMAccelObject<T> &rhs) const { return obj != rhs.obj; }
   bool operator<(const VMAccelObject<T> &rhs) const { return obj < rhs.obj; }
   bool operator>(const VMAccelObject<T> &rhs) const { return obj > rhs.obj; }
   bool operator<=(const VMAccelObject<T> &rhs) const { return obj <= rhs.obj; }
   bool operator>=(const VMAccelObject<T> &rhs) const { return obj >= rhs.obj; }
   VMAccelObject<T> operator+(const VMAccelObject<T> &rhs) const {
      return obj + rhs.obj;
   }
   VMAccelObject<T> operator-(const VMAccelObject<T> &rhs) const {
      return obj + rhs.obj;
   }
   VMAccelObject<T> operator+=(const VMAccelObject<T> &rhs) const {
      return obj += rhs.obj;
   }
   VMAccelObject<T> operator-=(const VMAccelObject<T> &rhs) const {
      return obj -= rhs.obj;
   }

   const T &GetObj() const { return obj; }

   void SetObj(T &o) { DeepCopy(obj, o); }

   VMAccelId GetParentId() const { return parentId; }

   VMAccelId GetFenceId() const { return fenceId; }

   void SetFenceId(VMAccelId id) { fenceId = id; }

private:
   VMAccelId parentId;
   VMAccelId fenceId;
   T obj;
};

template <class T, typename C>
class VMAccelAllocator {

public:
   /*
    * Base constructor functionality.
    */
   VMAccelAllocator<T, C>(size_t num) {
      registered.resize(num);
      capacity.resize(num);
      load.resize(num);
      refCount.resize(num);
      allocated.resize(num);
      registeredIds = IdentifierDB_Alloc(num);
      externalIds = IdentifierDB_Alloc(num);
   }

   /*
    * Destructor.
    */
   ~VMAccelAllocator<T, C>() {
      IdentifierDB_Free(externalIds);
      IdentifierDB_Free(registeredIds);
      externalIds = NULL;
      registeredIds = NULL;
   }

   VMAccelAllocateStatus *Register(T *desc);
   VMAccelStatus *Unregister(VMAccelId id);
   VMAccelAllocateStatus *Alloc(VMAccelId parentId, T *desc, T &out);
   VMAccelStatus *Free(VMAccelId id);

private:
   void CoalesceFreed();
   bool FindFreed(VMAccelObject<T> &req, VMAccelObject<T> &out);

   /*
    * List of resources in registered id space.
    *
    * For scalability, use a multi-dimensional database (e.g. SQL).
    */
   std::vector<T> registered;
   std::vector<T> capacity;
   std::vector<T> load;

   /*
    * Reference counts for the resources in registered id space.
    */
   std::vector<unsigned int> refCount;

   /*
    * Set of sorted free elements. We use a set to sort capacity and reduce
    * lookup runtime.
    */
   std::multiset<VMAccelObject<T>, C> free;

   /*
    * Queue of freed elements. These elements are released by the client
    * but may still be pending on the Accelerator. Moving an element from
    * released to free will only occur when the Accelerator is done with
    * the element.
    */
   std::queue<VMAccelObject<T>> freed;

   /*
    * Internal database of allocated elements in external Id space.
    */
   std::vector<VMAccelObject<T>> allocated;

   /*
    * Identifier databases for translation from external to registered
    * resources. We allocate external Ids to associate them with accelerator
    * based extents, to give temporal lifetime to each allocation. This avoids
    * fencing on the whole allocation, when only a part of the allocation is
    * busy.
    */
   IdentifierDB *registeredIds;
   IdentifierDB *externalIds;
};

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
      a = VMAccelObject<T>(freed.front());

      if (!vmaccel_manager_wait_for_fence(a.GetFenceId())) {
         VMACCEL_WARNING("Unable to wait for fence %d\n", a.GetFenceId());
         continue;
      }

      freed.pop();

      if (!FreeObj(free, a)) {
         VMACCEL_WARNING("Unable to add freed object to free set...\n");
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
         VMACCEL_WARNING("Unable to wait for fence %d\n", a.GetFenceId());
         continue;
      }

      freed.pop();

      if (a.GetParentId() != req.GetParentId()) {
         VMACCEL_WARNING("Unable to add freed object to free set...\n");
         continue;
      }

      if (req <= a) {
         const T in = a.GetObj();
         T div, r;
         if (Reserve(a.GetObj(), req.GetObj(), div, r)) {
            if (!IsEmpty(r)) {
               if (!FreeObj(free, VMAccelObject<T>(req.GetParentId(), &r))) {
                  VMACCEL_WARNING("Unable to add remainder to free set...\n");
               }
            }
            Destructor(r);
            d = VMAccelObject<T>(req.GetParentId(), &div);
            Destructor(div);
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
               Destructor(div);
               if (!IsEmpty(r)) {
                  if (!FreeObj(free, VMAccelObject<T>(req.GetParentId(), &r))) {
                     VMACCEL_WARNING(
                        "Unable to add remainder to free set...\n");
                  }
               }
               Destructor(r);
               found = true;
            }
         }
      } else {
         VMACCEL_WARNING("Unable to add remainder to free set...\n");
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
      VMACCEL_WARNING("Unable to allocate a registered accelerator ID\n");
      result.status = VMACCEL_FAIL;
      return &result;
   }

   capacity[result.id] += *a;
   registered[result.id] = *a;
   refCount[result.id] = 0;

   if (!FreeObj(free, VMAccelObject<T>(result.id, a))) {
      VMACCEL_WARNING("Unable to add object to free set...\n");
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
      VMACCEL_WARNING("Unable to allocate an external accelerator ID\n");
      result.status = VMACCEL_FAIL;
      return &result;
   }

   auto it = free.lower_bound(req);
   if (it != free.end()) {
      T div, r;
      if (Reserve(it->GetObj(), req.GetObj(), div, r)) {
         free.erase(it);
         obj = VMAccelObject<T>(req.GetParentId(), &div);
         Destructor(div);
         if (!IsEmpty(r)) {
            if (!FreeObj(free, VMAccelObject<T>(req.GetParentId(), &r))) {
               VMACCEL_WARNING("Unable to add remainder to free set...\n");
            }
         }
         Destructor(r);
         found = true;
      }
   } else if (FindFreed(req, obj)) {
      found = true;
   }

   /*
    * No matching allocation.
    */
   if (!found) {
      VMACCEL_WARNING("No matching allocation found...\n");
      IdentifierDB_ReleaseId(externalIds, externalId);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return &result;
   }

   registeredId = obj.GetParentId();
   allocated[externalId] = obj;

   d = allocated[externalId].GetObj();
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

#endif /* defined _VMACCEL_ALLOCATOR_HPP_ */
