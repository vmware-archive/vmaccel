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
unsigned int scratchState[VMACCEL_MAX_STREAMS];
unsigned int serverState[VMACCEL_MAX_STREAMS];
VMAccelSurfaceMapStatus mapStatus[VMACCEL_MAX_STREAMS];

static VMAccelSurfaceMapStatus *null_surfacemap_1(VMCLSurfaceMapOp *argp) {
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->op.surf.id;
   unsigned int gen = (unsigned int)argp->op.surf.generation;
   unsigned int inst = (unsigned int)argp->op.surf.instance;
   unsigned int blocking = TRUE;

   VMACCEL_LOG("%s: sid=%d, gen=%d, inst=%d\n", __FUNCTION__, sid, gen, inst);

   memset(&mapStatus[sid], 0, sizeof(VMAccelSurfaceMapStatus));

   mapStatus[sid].ptr.ptr_val = (char *)&scratchState[sid];
   mapStatus[sid].ptr.ptr_len = sizeof(scratchState[0]);
   mapStatus[sid].status = VMACCEL_SUCCESS;

   return (&mapStatus[sid]);
}


VMAccelStatus *null_surfaceunmap_1(VMCLSurfaceUnmapOp *argp) {
   static VMAccelStatus result;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->op.surf.id;
   unsigned int gen = (unsigned int)argp->op.surf.generation;
   unsigned int inst = (unsigned int)argp->op.surf.instance;

   memset(&result, 0, sizeof(result));

   VMACCEL_LOG("%s: sid=%d, gen=%d, inst=%d, *ptr=%x\n", __FUNCTION__, sid, gen,
               inst, ((unsigned int *)argp->op.ptr.ptr_val)[0]);

   assert(argp->op.ptr.ptr_len == (sizeof(clientState) / VMACCEL_MAX_STREAMS));

   memcpy(&serverState[sid], argp->op.ptr.ptr_val, argp->op.ptr.ptr_len);

   return (&result);
}


int main(int argc, char **argv) {
   int i;
   int ret;

   VMACCEL_LOG("%s: Running self-test of streaming...\n", __FUNCTION__);

   for (i = 0; i < VMACCEL_MAX_STREAMS; i++) {
      clientState[i] = i + 1;
      scratchState[i] = 0xbaadf00d;
      serverState[i] = 0;
   }

   VMACCEL_LOG("%s: Powering on VMAccel stream module...\n", __FUNCTION__);

   ret = vmaccel_stream_poweron();

   /*
    * Initializing stream server for ports starting at 5100 to
    * 5100+VMACCEL_MAX_STREAMS
    */
   VMACCEL_LOG("%s: Starting VMAccel stream server...\n", __FUNCTION__);

   VMAccelStreamCallbacks cb;
   cb.clSurfacemap_1 = null_surfacemap_1;
   cb.clSurfaceunmap_1 = null_surfaceunmap_1;
   ret = vmaccel_stream_server(VMACCEL_STREAM_TYPE_VMCL_UPLOAD,
                               VMACCEL_VMCL_BASE_PORT, &cb);

   /*
    * Wait for the asynchronous threads to send all the data.
    */
   VMACCEL_LOG("%s: Waiting for async upload...\n", __FUNCTION__);

   while (memcmp(clientState, serverState, sizeof(clientState)) != 0) {
      sleep(10);
   }

   /*
    * Log the different states
    */
   for (i = 0; i < VMACCEL_MAX_STREAMS; i++) {
      VMACCEL_LOG("%s: client[%d]=%x scratch[%d]=%x server[%d]=%x\n",
                  __FUNCTION__, i, clientState[i], i, scratchState[i], i,
                  serverState[i]);
   }

   /*
    * Shutdown the stream server...
    */
   VMACCEL_LOG("%s: Shutting down stream server...\n", __FUNCTION__);

   vmaccel_stream_poweroff();

   VMACCEL_LOG("%s: Self-test complete...\n", __FUNCTION__);

   return 0;
}
