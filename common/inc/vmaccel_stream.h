/******************************************************************************

Copyright (c) 2020-2022 VMware, Inc.
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

#ifndef _VMACCEL_STREAM_H_
#define _VMACCEL_STREAM_H_ 1

#include "vmaccel_defs.h"
#include "vmaccel_rpc.h"
#include "vmcl_rpc.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
   VMACCEL_STREAM_TYPE_VMCL_UPLOAD = 0,
   VMACCEL_STREAM_TYPE_MAX,
} VMAccelStreamType;

typedef struct {
   VMAccelSurfaceMapStatus *(*clSurfacemap_1)(VMCLSurfaceMapOp *);
   VMAccelStatus *(*clSurfaceunmap_1)(VMCLSurfaceUnmapOp *);
} VMAccelStreamCallbacks;

typedef struct {
   VMAccelAddress accel;
   unsigned int index;
   unsigned int type;
} VMAccelStream;

typedef struct {
   VMAccelStream stream;
   VMAccelStreamCallbacks cb;
} VMAccelStreamContext;

typedef struct {
   unsigned int type;
   u_int len;
   union {
      VMCLSurfaceMapOp cl;
   } desc;
} VMAccelStreamPacket;

typedef struct {
   VMAccelAddress accel;
   unsigned int type;
   unsigned int index;
   union {
      VMCLSurfaceMapOp cl;
   } desc;
   struct {
      u_int ptr_len;
      char *ptr_val;
   } ptr;
} VMAccelStreamSend;


int vmaccel_stream_poweron();
int vmaccel_stream_server(unsigned int type, unsigned int port,
                          VMAccelStreamCallbacks *cb);
int vmaccel_stream_send_async(VMAccelAddress *s, unsigned int type, void *args,
                              char *ptr_val, size_t ptr_len);
void vmaccel_stream_poweroff();

#ifdef __cplusplus
}
#endif

#endif /* _VMACCEL_STREAM_H_ */
