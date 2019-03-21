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

#ifndef _VMACCEL_TYPES_ALLOCRANGE_HPP_
#define _VMACCEL_TYPES_ALLOCRANGE_HPP_ 1

#include <cstring>

#include "log_level.h"


struct AllocRange {
   // Dynamic flow control for templated compare function.
   bool cmpRange;

   // Size is used to find the best fit for a requested allocation.
   size_t size;

   // begin and end are inclusive to match std::ranges, requires
   // size checks...
   size_t begin, end;
};

inline bool operator<(const AllocRange &lhs, const AllocRange &rhs) {
   if (lhs.cmpRange || rhs.cmpRange) {
      if ((lhs.begin < rhs.begin) || (lhs.end < rhs.end)) {
         return true;
      }
   } else {
      return lhs.size < rhs.size;
   }

   return false;
}

inline bool operator<=(const AllocRange &lhs, const AllocRange &rhs) {
   if (lhs.cmpRange || rhs.cmpRange) {
      if ((lhs.begin <= rhs.begin) && (lhs.end <= rhs.end)) {
         return true;
      }
   } else {
      return lhs.size <= rhs.size;
   }
}

inline AllocRange operator+=(AllocRange &lhs, const AllocRange &rhs) {
   lhs.size += rhs.size;
   return lhs;
};

inline AllocRange operator-=(AllocRange &lhs, const AllocRange &rhs) {
   lhs.size -= rhs.size;
   return lhs;
};

struct AllocRangeCmp {
   bool operator()(const VMAccelObject<AllocRange> &lhs,
                   const VMAccelObject<AllocRange> &rhs) const {
      return (lhs.GetParentId() != rhs.GetParentId() ||
              lhs.GetObj() < rhs.GetObj());
   }
};

static bool IsEmpty(const AllocRange val) {
   if (val.size == 0) {
      return true;
   }
   return false;
}

static bool Reserve(const AllocRange &a, const AllocRange &req, AllocRange &d,
                    AllocRange &r) {
   if (a < req) {
      return false;
   }

   d = r = a;

   // Allocate the range from the beginning
   d.size = req.size;
   d.begin = a.begin;
   d.end = a.begin + req.size - 1;
   r.size = a.size - d.size;
   r.end = a.end;
   r.begin = d.end + 1;

   return r < a;
}

static bool
FreeObj(std::multiset<VMAccelObject<AllocRange>, AllocRangeCmp> &pool,
        const VMAccelObject<AllocRange> obj) {
   /*
    * Update and replace the value.
    */
   AllocRange r = obj.GetObj();
   r.cmpRange = true;
   auto it =
      //   typename std::multiset<VMAccelObject<AllocRange>,
      //   AllocRangeCmp>::iterator it =
      pool.lower_bound(VMAccelObject<AllocRange>(obj.GetParentId(), &r));
   if (it != pool.end()) {
      AllocRange range = it->GetObj();
      if (range.begin == r.end + 1) {
         pool.erase(it);
         range.begin = r.begin;
         range.size += r.end - r.begin + 1;
         auto res =
            pool.insert(VMAccelObject<AllocRange>(obj.GetParentId(), &range));
         return res != pool.end();
      }
      if (range.end + 1 == r.begin) {
         pool.erase(it);
         range.end = r.begin;
         range.size += r.end - r.begin + 1;
         auto res =
            pool.insert(VMAccelObject<AllocRange>(obj.GetParentId(), &range));
         return res != pool.end();
      }
   }
   it = pool.upper_bound(VMAccelObject<AllocRange>(obj.GetParentId(), &r));
   if (it != pool.end()) {
      AllocRange range = it->GetObj();
      if (range.begin == r.end + 1) {
         pool.erase(it);
         range.begin = r.begin;
         range.size += r.end - r.begin + 1;
         auto res =
            pool.insert(VMAccelObject<AllocRange>(obj.GetParentId(), &range));
         return res != pool.end();
      }
      if (range.end + 1 == r.begin) {
         pool.erase(it);
         range.end = r.begin;
         range.size += r.end - r.begin + 1;
         auto res =
            pool.insert(VMAccelObject<AllocRange>(obj.GetParentId(), &range));
         return res != pool.end();
      }
   }
   auto res = pool.insert(obj);
   return res != pool.end();
}

inline void Log_AllocRange(const char *prefix, const AllocRange *range) {
   Log("%s size=%zu\n", prefix, range->size);
   Log("%s begin=%zu, end=%zu\n", prefix, range->begin, range->end);
}

#endif /* defined _VMACCEL_TYPES_ALLOCRANGE_HPP_ */
