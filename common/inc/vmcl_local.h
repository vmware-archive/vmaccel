/******************************************************************************

Copyright (c) 2020 VMware, Inc.
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

#ifndef _VMCL_LOCAL_H_
#define _VMCL_LOCAL_H_

#include "vmcl_rpc.h"

#undef vmcl_contextalloc_1
#define vmcl_contextalloc_1 vmcl_contextalloc_1_svc

#undef vmcl_contextdestroy_1
#define vmcl_contextdestroy_1 vmcl_contextdestroy_1_svc

#undef vmcl_surfacealloc_1
#define vmcl_surfacealloc_1 vmcl_surfacealloc_1_svc

#undef vmcl_surfacedestroy_1
#define vmcl_surfacedestroy_1 vmcl_surfacedestroy_1_svc

#undef vmcl_surfacegetsharedhandle_1
#define vmcl_surfacegetsharedhandle_1 vmcl_surfacegetsharedhandle_1_svc

#undef vmcl_surfacereleasesharedhandle_1
#define vmcl_surfacereleasesharedhandle_1 vmcl_surfacereleasesharedhandle_1_svc

#undef vmcl_queuealloc_1
#define vmcl_queuealloc_1 vmcl_queuealloc_1_svc

#undef vmcl_queuedestroy_1
#define vmcl_queuedestroy_1 vmcl_queuedestroy_1_svc

#undef vmcl_eventalloc_1
#define vmcl_eventalloc_1 vmcl_eventalloc_1_svc

#undef vmcl_eventgetstatus_1
#define vmcl_eventgetstatus_1 vmcl_eventgetstatus_1_svc

#undef vmcl_eventdestroy_1
#define vmcl_eventdestroy_1 vmcl_eventdestroy_1_svc

#undef vmcl_fencealloc_1
#define vmcl_fencealloc_1 vmcl_fencealloc_1_svc

#undef vmcl_fencegetstatus_1
#define vmcl_fencegetstatus_1 vmcl_fencegetstatus_1_svc

#undef vmcl_fencedestroy_1
#define vmcl_fencedestroy_1 vmcl_fencedestroy_1_svc

#undef vmcl_queueflush_1
#define vmcl_queueflush_1 vmcl_queueflush_1_svc

#undef vmcl_eventinsert_1
#define vmcl_eventinsert_1 vmcl_eventinsert_1_svc

#undef vmcl_fenceinsert_1
#define vmcl_fenceinsert_1 vmcl_fenceinsert_1_svc

#undef vmcl_imageupload_1
#define vmcl_imageupload_1 vmcl_imageupload_1_svc

#undef vmcl_imagedownload_1
#define vmcl_imagedownload_1 vmcl_imagedownload_1_svc

#undef vmcl_surfacemap_1
#define vmcl_surfacemap_1 vmcl_surfacemap_1_svc

#undef vmcl_surfaceunmap_1
#define vmcl_surfaceunmap_1 vmcl_surfaceunmap_1_svc

#undef vmcl_surfacecopy_1
#define vmcl_surfacecopy_1 vmcl_surfacecopy_1_svc

#undef vmcl_imagefill_1
#define vmcl_imagefill_1 vmcl_imagefill_1_svc

#undef vmcl_sampleralloc_1
#define vmcl_sampleralloc_1 vmcl_sampleralloc_1_svc

#undef vmcl_samplerdestroy_1
#define vmcl_samplerdestroy_1 vmcl_samplerdestroy_1_svc

#undef vmcl_kernelalloc_1
#define vmcl_kernelalloc_1 vmcl_kernelalloc_1_svc

#undef vmcl_kerneldestroy_1
#define vmcl_kerneldestroy_1 vmcl_kerneldestroy_1_svc

#undef vmcl_dispatch_1
#define vmcl_dispatch_1 vmcl_dispatch_1_svc

#endif /* !_VMCL_LOCAL_H_ */
