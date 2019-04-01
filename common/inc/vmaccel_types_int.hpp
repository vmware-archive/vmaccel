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

#ifndef _VMACCEL_TYPES_INT_HPP_
#define _VMACCEL_TYPES_INT_HPP_ 1

struct Int {
   int x;
};

inline bool operator<(const Int &lhs, const Int &rhs) {
   return lhs.x < rhs.x;
};

inline bool operator<=(const Int &lhs, const Int &rhs) {
   return lhs.x <= rhs.x;
};

inline Int operator+=(Int lhs, const Int &rhs) {
   lhs.x += rhs.x;
   return lhs;
};

inline Int operator-=(Int lhs, const Int &rhs) {
   lhs.x -= rhs.x;
   return lhs;
};

inline Int operator+(const Int &lhs, const Int &rhs) {
   Int out;
   out.x = lhs.x + rhs.x;
   return out;
};

inline Int operator-(const Int &lhs, const Int &rhs) {
   Int out;
   out.x = lhs.x - rhs.x;
   return out;
};

struct IntCmp {
   bool operator()(const VMAccelObject<Int> &lhs,
                   const VMAccelObject<Int> &rhs) const {
      return (lhs.GetParentId() != rhs.GetParentId() ||
              lhs.GetObj().x < rhs.GetObj().x);
   }
};

static void Constructor(Int &obj) {
   obj.x = 0;
}

static void Destructor(Int &obj) {
}

static void DeepCopy(Int &lhs, const Int &rhs) {
   if (&lhs != &rhs) {
      lhs.x = rhs.x;
   }
}

static void Move(Int &lhs, Int &rhs) {
   if (&lhs != &rhs) {
      lhs.x = rhs.x;
      memset(&rhs, 0, sizeof(rhs));
   }
}

static bool IsEmpty(const Int val) {
   return val.x == 0;
}

static bool Reserve(const Int &in, const Int &req, Int &out, Int &remainder) {
   if (in.x < req.x) {
      return false;
   }

   out.x = req.x;
   remainder.x = in.x - out.x;

   return true;
}

static bool FreeObj(std::multiset<VMAccelObject<Int>, IntCmp> &pool,
                    const VMAccelObject<Int> &obj) {
   /*
    * Update and replace the value, only one per-parent.
    */
   auto it = pool.begin();
   for (; it != pool.end(); it++) {
      if (it->GetParentId() == obj.GetParentId()) {
         break;
      }
   }

   if (it == pool.end()) {
      auto res = pool.insert(obj);
      return res != pool.end();
   } else {
      Int val = it->GetObj();
      pool.erase(it);
      val.x += obj.GetObj().x;
      auto res = pool.insert(VMAccelObject<Int>(obj.GetParentId(), &val));
      return res != pool.end();
   }
}

#endif /* defined _VMACCEL_TYPES_INT_HPP_ */
