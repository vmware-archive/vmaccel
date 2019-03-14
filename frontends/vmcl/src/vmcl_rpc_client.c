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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmcl_rpc.h"

const char *kernelName = "hello_kernel";
const char *kernelSource = "__kernel void hello_kernel(__global int *a)\n"
                           "{\n"
                           "    int gid = get_global_id(0);\n"
                           "    a[gid] = a[gid] + a[gid];\n"
                           "}\n";

unsigned int buffer[16];

void vmcl_1(char *host, char *spirv) {
   CLIENT *clnt;
   VMCLContextAllocateReturnStatus *result_1;
   VMCLContextAllocateDesc vmcl_contextalloc_1_arg;
   VMAccelReturnStatus *result_2;
   VMCLContextId vmcl_contextdestroy_1_arg;
   VMAccelSurfaceAllocateReturnStatus *result_3;
   VMCLSurfaceAllocateDesc vmcl_surfacealloc_1_arg;
   VMAccelReturnStatus *result_4;
   VMCLSurfaceId vmcl_surfacedestroy_1_arg;
   VMAccelSharedHandleReturnStatus *result_5;
   VMCLSurfaceId vmcl_surfacegetsharedhandle_1_arg;
   VMAccelReturnStatus *result_6;
   VMCLSharedHandle vmcl_surfacereleasesharedhandle_1_arg;
   VMAccelQueueReturnStatus *result_7;
   VMCLQueueAllocateDesc vmcl_queuealloc_1_arg;
   VMAccelReturnStatus *result_8;
   VMCLQueueId vmcl_queuedestroy_1_arg;
   VMAccelEventReturnStatus *result_9;
   VMCLEventAllocateDesc vmcl_eventalloc_1_arg;
   VMAccelEventReturnStatus *result_10;
   VMCLEventId vmcl_eventgetstatus_1_arg;
   VMAccelEventReturnStatus *result_11;
   VMCLEventId vmcl_eventdestroy_1_arg;
   VMAccelFenceReturnStatus *result_12;
   VMCLFenceAllocateDesc vmcl_fencealloc_1_arg;
   VMAccelFenceReturnStatus *result_13;
   VMCLFenceId vmcl_fencegetstatus_1_arg;
   VMAccelFenceReturnStatus *result_14;
   VMCLFenceId vmcl_fencedestroy_1_arg;
   VMAccelReturnStatus *result_15;
   VMCLQueueId vmcl_queueflush_1_arg;
   VMAccelReturnStatus *result_16;
   VMCLEventInsertOp vmcl_eventinsert_1_arg;
   VMAccelReturnStatus *result_17;
   VMCLFenceInsertOp vmcl_fenceinsert_1_arg;
   VMAccelReturnStatus *result_18;
   VMCLImageUploadOp vmcl_imageupload_1_arg;
   VMAccelDownloadReturnStatus *result_19;
   VMCLImageDownloadOp vmcl_imagedownload_1_arg;
   VMAccelSurfaceMapReturnStatus *result_20;
   VMCLSurfaceMapOp vmcl_surfacemap_1_arg;
   VMAccelReturnStatus *result_21;
   VMCLSurfaceUnmapOp vmcl_surfaceunmap_1_arg;
   VMAccelReturnStatus *result_22;
   VMCLSurfaceCopyOp vmcl_surfacecopy_1_arg;
   VMAccelReturnStatus *result_23;
   VMCLImageFillOp vmcl_imagefill_1_arg;
   VMCLSamplerAllocateReturnStatus *result_24;
   VMCLSamplerAllocateDesc vmcl_sampleralloc_1_arg;
   VMAccelReturnStatus *result_25;
   VMCLSamplerId vmcl_samplerdestroy_1_arg;
   VMCLKernelAllocateReturnStatus *result_26;
   VMCLKernelAllocateDesc vmcl_kernelalloc_1_arg;
   VMAccelReturnStatus *result_27;
   VMCLKernelId vmcl_kerneldestroy_1_arg;
   VMAccelReturnStatus *result_28;
   VMCLDispatchOp vmcl_dispatch_1_arg;
   VMCLKernelArgDesc kernelArgs[1];
   unsigned int globalWorkOffset[1] = {
      0,
   };
   unsigned int globalWorkSize[1] = {
      0,
   };
   unsigned int localWorkSize[1] = {
      0,
   };
   unsigned int cid = 0;
   unsigned int qid = 0;
   unsigned int sid = 0;
   unsigned int kid = 0;
   unsigned int i;
   char *kernelBinary = NULL;

   clnt = clnt_create(host, VMCL, VMCL_VERSION, "udp");
   if (clnt == NULL) {
      clnt_pcreateerror(host);
      exit(1);
   }

   for (i = 0; i < sizeof(buffer) / sizeof(buffer[0]); i++) {
      buffer[i] = i;
   }

   memset(&vmcl_contextalloc_1_arg, 0, sizeof(vmcl_contextalloc_1_arg));
   vmcl_contextalloc_1_arg.accelId = 0;
   vmcl_contextalloc_1_arg.clientId = cid;
   vmcl_contextalloc_1_arg.selectionMask = VMACCEL_CPU_MASK | VMACCEL_GPU_MASK;
   vmcl_contextalloc_1_arg.requiredCaps =
      (spirv != NULL) ? VMCL_SPIRV_1_0_CAP : 0;

   result_1 = vmcl_contextalloc_1(&vmcl_contextalloc_1_arg, clnt);
   if (result_1 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

   memset(&vmcl_surfacealloc_1_arg, 0, sizeof(vmcl_surfacealloc_1_arg));
   vmcl_surfacealloc_1_arg.client.cid = cid;
   vmcl_surfacealloc_1_arg.client.accel.id = sid;
   vmcl_surfacealloc_1_arg.desc.type = VMACCEL_SURFACE_BUFFER;
   vmcl_surfacealloc_1_arg.desc.width = sizeof(buffer);
   vmcl_surfacealloc_1_arg.desc.format = VMACCEL_FORMAT_R8_TYPELESS;
   vmcl_surfacealloc_1_arg.desc.usage = VMACCEL_SURFACE_USAGE_READWRITE;
   vmcl_surfacealloc_1_arg.desc.bindFlags = VMACCEL_BIND_UNORDERED_ACCESS_FLAG;

   result_3 = vmcl_surfacealloc_1(&vmcl_surfacealloc_1_arg, clnt);
   if (result_3 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

#if 0
	result_5 = vmcl_surfacegetsharedhandle_1(&vmcl_surfacegetsharedhandle_1_arg, clnt);
	if (result_5 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_6 = vmcl_surfacereleasesharedhandle_1(&vmcl_surfacereleasesharedhandle_1_arg, clnt);
	if (result_6 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
#endif

   memset(&vmcl_queuealloc_1_arg, 0, sizeof(vmcl_queuealloc_1_arg));
   vmcl_queuealloc_1_arg.client.cid = cid;
   vmcl_queuealloc_1_arg.client.id = qid;
   vmcl_queuealloc_1_arg.desc.flags = VMACCEL_QUEUE_ON_DEVICE_FLAG;
   vmcl_queuealloc_1_arg.desc.size = -1; /* Unbounded? */

   result_7 = vmcl_queuealloc_1(&vmcl_queuealloc_1_arg, clnt);
   if (result_7 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

#if 0
	result_9 = vmcl_eventalloc_1(&vmcl_eventalloc_1_arg, clnt);
	if (result_9 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_10 = vmcl_eventgetstatus_1(&vmcl_eventgetstatus_1_arg, clnt);
	if (result_10 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_11 = vmcl_eventdestroy_1(&vmcl_eventdestroy_1_arg, clnt);
	if (result_11 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_12 = vmcl_fencealloc_1(&vmcl_fencealloc_1_arg, clnt);
	if (result_12 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_13 = vmcl_fencegetstatus_1(&vmcl_fencegetstatus_1_arg, clnt);
	if (result_13 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_14 = vmcl_fencedestroy_1(&vmcl_fencedestroy_1_arg, clnt);
	if (result_14 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_16 = vmcl_eventinsert_1(&vmcl_eventinsert_1_arg, clnt);
	if (result_16 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_17 = vmcl_fenceinsert_1(&vmcl_fenceinsert_1_arg, clnt);
	if (result_17 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_18 = vmcl_imageupload_1(&vmcl_imageupload_1_arg, clnt);
	if (result_18 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_19 = vmcl_imagedownload_1(&vmcl_imagedownload_1_arg, clnt);
	if (result_19 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
#endif

   memset(&vmcl_surfacemap_1_arg, 0, sizeof(vmcl_surfacemap_1_arg));
   vmcl_surfacemap_1_arg.queue.cid = cid;
   vmcl_surfacemap_1_arg.queue.id = qid;
   vmcl_surfacemap_1_arg.op.surf.id = (VMAccelId)sid;
   vmcl_surfacemap_1_arg.op.size.x = sizeof(buffer);
   vmcl_surfacemap_1_arg.op.mapFlags =
      VMACCEL_MAP_READ_FLAG | VMACCEL_MAP_WRITE_FLAG;

   result_20 = vmcl_surfacemap_1(&vmcl_surfacemap_1_arg, clnt);
   if (result_20 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

   if (result_20->VMAccelSurfaceMapReturnStatus_u.ret->status ==
       VMACCEL_SUCCESS) {
      void *ptr = result_20->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_val;

      assert(result_20->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_len ==
             vmcl_surfacemap_1_arg.op.size.x);

      /*
       * Memory copy the contents into the value
       */
      memcpy(ptr, &buffer, vmcl_surfacemap_1_arg.op.size.x);

      memset(&vmcl_surfaceunmap_1_arg, 0, sizeof(vmcl_surfaceunmap_1_arg));
      vmcl_surfaceunmap_1_arg.queue.cid = cid;
      vmcl_surfaceunmap_1_arg.queue.id = qid;
      vmcl_surfaceunmap_1_arg.op.surf.id = sid;
      vmcl_surfaceunmap_1_arg.op.ptr.ptr_len = vmcl_surfacemap_1_arg.op.size.x;
      vmcl_surfaceunmap_1_arg.op.ptr.ptr_val = ptr;

      result_21 = vmcl_surfaceunmap_1(&vmcl_surfaceunmap_1_arg, clnt);
      if (result_21 == NULL) {
         clnt_perror(clnt, "call failed:");
      }
   }

#if 0
	result_22 = vmcl_surfacecopy_1(&vmcl_surfacecopy_1_arg, clnt);
	if (result_22 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_23 = vmcl_imagefill_1(&vmcl_imagefill_1_arg, clnt);
	if (result_23 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_24 = vmcl_sampleralloc_1(&vmcl_sampleralloc_1_arg, clnt);
	if (result_24 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
	result_25 = vmcl_samplerdestroy_1(&vmcl_samplerdestroy_1_arg, clnt);
	if (result_25 == NULL) {
		clnt_perror(clnt, "call failed:");
	}
#endif

   memset(&vmcl_kernelalloc_1_arg, 0, sizeof(vmcl_kernelalloc_1_arg));
   vmcl_kernelalloc_1_arg.client.cid = cid;
   vmcl_kernelalloc_1_arg.client.id = kid;
   vmcl_kernelalloc_1_arg.kernelName.kernelName_len = strlen(kernelName);
   vmcl_kernelalloc_1_arg.kernelName.kernelName_val = kernelName;

   if (spirv) {
      FILE *fp;
      int kernelSize;

      fp = fopen(spirv, "rb");

      if (!fp) {
         printf("Unable to load file\n");
         exit(1);
      }

      fseek(fp, 0L, SEEK_END);
      kernelSize = ftell(fp);
      rewind(fp);

      kernelBinary = malloc(kernelSize);

      if (!kernelBinary) {
         fclose(fp);
         exit(1);
      }

      fread(kernelBinary, kernelSize, 1, fp);

      fclose(fp);

      vmcl_kernelalloc_1_arg.language = VMCL_SPIRV_1_0;
      vmcl_kernelalloc_1_arg.source.source_len = kernelSize;
      vmcl_kernelalloc_1_arg.source.source_val = kernelBinary;
   } else {
      vmcl_kernelalloc_1_arg.language = VMCL_OPENCL_C_1_0;
      vmcl_kernelalloc_1_arg.source.source_len = strlen(kernelSource);
      vmcl_kernelalloc_1_arg.source.source_val = kernelSource;
   }

   result_26 = vmcl_kernelalloc_1(&vmcl_kernelalloc_1_arg, clnt);
   if (result_26 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

   if (kernelBinary) {
      free(kernelBinary);
   }

   /*
    * Execute the compute kernel
    */
   globalWorkOffset[0] = 0;
   globalWorkSize[0] = sizeof(buffer) / sizeof(buffer[0]);
   localWorkSize[0] = 1;

   memset(&kernelArgs, 0, sizeof(kernelArgs));
   kernelArgs[0].index = 0;
   kernelArgs[0].type = VMCL_ARG_SURFACE;
   kernelArgs[0].surf = vmcl_surfacealloc_1_arg.client.accel;

   memset(&vmcl_dispatch_1_arg, 0, sizeof(vmcl_dispatch_1_arg));
   vmcl_dispatch_1_arg.queue.cid = cid;
   vmcl_dispatch_1_arg.queue.id = qid;
   vmcl_dispatch_1_arg.kernel.id = kid;
   vmcl_dispatch_1_arg.dimension = 1;
   vmcl_dispatch_1_arg.globalWorkOffset.globalWorkOffset_len =
      sizeof(globalWorkOffset) / sizeof(globalWorkOffset[0]);
   vmcl_dispatch_1_arg.globalWorkOffset.globalWorkOffset_val =
      &globalWorkOffset[0];
   vmcl_dispatch_1_arg.globalWorkSize.globalWorkSize_len =
      sizeof(globalWorkSize) / sizeof(globalWorkSize[0]);
   vmcl_dispatch_1_arg.globalWorkSize.globalWorkSize_val = &globalWorkSize[0];
   vmcl_dispatch_1_arg.localWorkSize.localWorkSize_len =
      sizeof(localWorkSize) / sizeof(localWorkSize[0]);
   vmcl_dispatch_1_arg.localWorkSize.localWorkSize_val = &localWorkSize[0];
   vmcl_dispatch_1_arg.args.args_len =
      sizeof(kernelArgs) / sizeof(kernelArgs[0]);
   vmcl_dispatch_1_arg.args.args_val = &kernelArgs[0];

   result_28 = vmcl_dispatch_1(&vmcl_dispatch_1_arg, clnt);
   if (result_28 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

   memset(&vmcl_surfacemap_1_arg, 0, sizeof(vmcl_surfacemap_1_arg));
   vmcl_surfacemap_1_arg.queue.cid = cid;
   vmcl_surfacemap_1_arg.queue.id = qid;
   vmcl_surfacemap_1_arg.op.surf.id = (VMAccelId)sid;
   vmcl_surfacemap_1_arg.op.size.x = sizeof(buffer);
   vmcl_surfacemap_1_arg.op.mapFlags = VMACCEL_MAP_READ_FLAG;

   result_20 = vmcl_surfacemap_1(&vmcl_surfacemap_1_arg, clnt);
   if (result_20 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

   if (result_20->VMAccelSurfaceMapReturnStatus_u.ret->status ==
       VMACCEL_SUCCESS) {
      unsigned int *ptr =
         result_20->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_val;

      assert(result_20->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_len ==
             vmcl_surfacemap_1_arg.op.size.x);

      for (i = 0; i < vmcl_surfacemap_1_arg.op.size.x / sizeof(buffer[0]);
           i++) {
         printf("ptr[%d]=%d\n", i, ptr[i]);
      }

      memset(&vmcl_surfaceunmap_1_arg, 0, sizeof(vmcl_surfaceunmap_1_arg));
      vmcl_surfaceunmap_1_arg.queue.cid = cid;
      vmcl_surfaceunmap_1_arg.queue.id = qid;
      vmcl_surfaceunmap_1_arg.op.surf.id = sid;
      vmcl_surfaceunmap_1_arg.op.ptr.ptr_len = vmcl_surfacemap_1_arg.op.size.x;
      vmcl_surfaceunmap_1_arg.op.ptr.ptr_val = ptr;

      result_21 = vmcl_surfaceunmap_1(&vmcl_surfaceunmap_1_arg, clnt);
      if (result_21 == NULL) {
         clnt_perror(clnt, "call failed:");
      }
   }

   vmcl_kerneldestroy_1_arg = vmcl_kernelalloc_1_arg.client;

   result_27 = vmcl_kerneldestroy_1(&vmcl_kerneldestroy_1_arg, clnt);
   if (result_27 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

   vmcl_queueflush_1_arg = vmcl_queuealloc_1_arg.client;

   result_15 = vmcl_queueflush_1(&vmcl_queueflush_1_arg, clnt);
   if (result_15 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

   vmcl_queuedestroy_1_arg = vmcl_queuealloc_1_arg.client;

   result_8 = vmcl_queuedestroy_1(&vmcl_queuedestroy_1_arg, clnt);
   if (result_8 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

   vmcl_surfacedestroy_1_arg = vmcl_surfacealloc_1_arg.client;

   result_4 = vmcl_surfacedestroy_1(&vmcl_surfacedestroy_1_arg, clnt);
   if (result_4 == NULL) {
      clnt_perror(clnt, "call failed:");
   }

   vmcl_contextdestroy_1_arg = cid;
   result_2 = vmcl_contextdestroy_1(&vmcl_contextdestroy_1_arg, clnt);
   if (result_2 == NULL) {
      clnt_perror(clnt, "call failed:");
   }
   clnt_destroy(clnt);
}

int main(int argc, char *argv[]) {
   char *host;
   char *spirv = NULL;

   if (argc < 2) {
      printf("usage: %s server_host\n", argv[0]);
      exit(1);
   }

   host = argv[1];

   if (argc > 2) {
      spirv = argv[2];
   }

   vmcl_1(host, spirv);

   return 0;
}
