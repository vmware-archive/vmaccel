/******************************************************************************

Copyright (c) 2021-2022 VMware, Inc.
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

#include <assert.h>
#include <iostream>
#include <stdbool.h>
#include <string>
#include <unistd.h>

#include "vmaccel_stream.h"
#include "vmaccel_utils.h"

#include "log_level.h"

using namespace std;

unsigned int clientState[VMACCEL_MAX_STREAMS];

int main(int argc, char **argv) {
   int i;
   int ret;

   VMACCEL_LOG("%s: Running self-test of streaming...\n", __FUNCTION__);

   for (i = 0; i < VMACCEL_MAX_STREAMS; i++) {
      clientState[i] = i + 1;
   }

   VMACCEL_LOG("%s: Powering on VMAccel stream module...\n", __FUNCTION__);

   ret = vmaccel_stream_poweron();

   /*
    * Spawn threads to send to all the stream server ports
    */
   for (i = 0; i < VMACCEL_MAX_STREAMS; i++) {
      VMCLSurfaceMapOp vmcl_surfacemap_1_arg;
      VMAccelAddress a;
      char host[4 * VMACCEL_MAX_LOCATION_SIZE];

      VMACCEL_LOG("%s: Stream upload %d\n", __FUNCTION__, i);

      memset(&vmcl_surfacemap_1_arg, 0, sizeof(vmcl_surfacemap_1_arg));
      vmcl_surfacemap_1_arg.queue.cid = 0;
      vmcl_surfacemap_1_arg.queue.id = 0;
      vmcl_surfacemap_1_arg.op.surf.id = i;
      vmcl_surfacemap_1_arg.op.surf.generation = 1;
      vmcl_surfacemap_1_arg.op.size.x = sizeof(unsigned int);
      vmcl_surfacemap_1_arg.op.size.y = 1;
      vmcl_surfacemap_1_arg.op.mapFlags = VMACCEL_MAP_READ_FLAG |
                                          VMACCEL_MAP_WRITE_FLAG |
                                          VMACCEL_MAP_ASYNC_FLAG;

      a.addr.addr_val =
         (char *)malloc(VMACCEL_MAX_LOCATION_SIZE * sizeof(char));
      a.addr.addr_len = VMACCEL_MAX_LOCATION_SIZE;

      if (!VMAccel_AddressStringToOpaqueAddr("127.0.0.1", a.addr.addr_val,
                                             a.addr.addr_len)) {
         VMACCEL_WARNING("%s: Failed to translate address\n", __FUNCTION__);
         assert(0);
      }

      a.port = VMACCEL_VMCL_BASE_PORT;

      if (!VMAccel_AddressOpaqueAddrToString(&a, host, sizeof(host))) {
         VMACCEL_WARNING("%s: Unable to translate address to string\n",
                         __FUNCTION__);
         assert(0);
      }

      VMACCEL_LOG("%s: Initiating async stream upload %d host=%s port=%d\n",
                  __FUNCTION__, i, host, a.port);

      ret = vmaccel_stream_send_async(
         &a, VMACCEL_STREAM_TYPE_VMCL_UPLOAD, &vmcl_surfacemap_1_arg,
         (char *)&clientState[i], sizeof(unsigned int));
   }

   vmaccel_stream_poweroff();

   VMACCEL_LOG("%s: Self-test complete...\n", __FUNCTION__);

   return 0;
}
