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

#include <bitset>
#include "vmaccel_utils.h"

template <size_t bitsetSize>
int BitSet_FindFirstZero(std::bitset<bitsetSize> bits) {
   int size = bits.size();
   int idx = 0;

   while (size > 0) {
      int ret = BitMask_FindFirstZero((unsigned int)bits);
      if (idx != -1) {
         return idx + ret;
      } else {
         idx += 32;
         size -= 32;
         bits >>= 32;
      }
   }
   return -1;
}

template <size_t bitsetSize>
int BitSet_FindFirstOne(std::bitset<bitsetSize> bits) {
   int size = bits.size();
   int idx = 0;

   while (size > 0) {
      int ret = BitMask_FindFirstOne((unsigned int)bits);
      if (idx != -1) {
         return idx + ret;
      } else {
         idx += 32;
         size -= 32;
         bits >>= 32;
      }
   }
   return -1;
}
