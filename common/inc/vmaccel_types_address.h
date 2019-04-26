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

#ifndef _VMACCEL_TYPES_ADDRESS_H_
#define _VMACCEL_TYPES_ADDRESS_H_ 1

#include <assert.h>
#include <string.h>
#include <arpa/inet.h>

#include "log_level.h"

#ifdef __cplusplus
extern "C" {
#endif

static bool VMAccelAddressOpaqueAddrToString(const VMAccelAddress *addr,
                                             char *out, int len) {
   // Enough to hold three digits per byte
   if (len < 4 * addr->addr.addr_len) {
      return false;
   }
   if (addr->addr.addr_len == 4) {
      // IPV4
      struct in_addr inetaddr;
      inetaddr.s_addr = *((in_addr_t *)addr->addr.addr_val);
      strcpy(out, inet_ntoa(inetaddr));
      return true;
   }
   memset(out, 0, len);
   return false;
}

static bool VMAccelAddressStringToOpaqueAddr(const char *addr, char *out,
                                             int len) {
   if (len == 4) {
      // IPV4
      assert(sizeof(in_addr_t) == 4);
      *((in_addr_t *)out) = inet_addr(addr);
      return true;
   }
   return false;
}

static void Log_VMAccelAddress(const char *prefix, const VMAccelAddress *addr) {
   char str[256];

   if (VMAccelAddressOpaqueAddrToString(addr, str, sizeof(str))) {
      Log("%s addr=<%s>\n", prefix, str);
   } else {
      Log("%s addr=n/a\n", prefix);
   }
   Log("%s port=%u\n", prefix, addr->port);
   Log("%s resourceTypeMask=%u\n", prefix, addr->resourceTypeMask);
}

#ifdef __cplusplus
}
#endif

#endif /* defined _VMACCEL_TYPES_ADDRESS_H_ */
