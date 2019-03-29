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

#ifndef _VMACCEL_ALLOCATOR_HPP_
#define _VMACCEL_ALLOCATOR_HPP_ 1

#include "vmaccel_rpc.h"
#include "vmaccel_utils.h"
#include <algorithm>
#include <cassert>
#include <queue>
#include <set>
#include <vector>

template <class T>
class VMAccelObject {

public:
   VMAccelObject<T>() {
      parentId = VMACCEL_INVALID_ID;
      fenceId = VMACCEL_INVALID_ID;
      memset(&obj, 0, sizeof(obj));
   }

   VMAccelObject<T>(VMAccelId id, T *o) {
      parentId = id;
      fenceId = VMACCEL_INVALID_ID;
      memset(&obj, 0, sizeof(obj));
      DeepCopy(obj, *o);
   }

   VMAccelObject<T>(const VMAccelObject<T> &o) {
      parentId = o.parentId;
      fenceId = o.fenceId;
      memset(&obj, 0, sizeof(obj));
      DeepCopy(obj, o.obj);
   }

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

#endif /* defined _VMACCEL_ALLOCATOR_HPP_ */
