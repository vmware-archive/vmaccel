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

#ifdef RPC_HDR
%#include "vmaccel_rpc.h"
%#include "vmcl_defs.h"
#endif

typedef unsigned int VMCLCaps;
typedef unsigned int VMCLKernelLanguage;

/*
 * Accelerator identifier.
 */
typedef VMAccelId VMCLContextId;

/*
 * Accelerator context allocation descriptor structure.
 */
struct VMCLContextAllocateDesc {
   VMAccelId                 accelId;
   VMCLContextId             clientId;

   /*
    * Selection mask for context placement hints.
    */
   VMAccelSelectionMask      selectionMask;

   /*
    * Required capabilities for context allocation.
    */
   VMCLCaps                  requiredCaps;
};

/*
 * Accelerator context allocation.
 */
struct VMCLContextAllocateStatus {
   VMAccelStatusCode         status;
   VMCLCaps                  caps;
};

/*
 * Accelerator surface allocation descriptor structure.
 */
struct VMCLSurfaceId {
   VMCLContextId             cid;
   VMAccelSurfaceId          accel;
};

struct VMCLSurfaceAllocateDesc {
   /*
    * Used to create a mapping for the client's requested surfaces to
    * avoid feedback of the ID in a return status.
    */
   VMCLSurfaceId             client;
   VMAccelSurfaceDesc        desc;
};

/*
 * Accelerator shared handle structure.
 */
struct VMCLSharedHandle {
   VMCLContextId             cid;
   VMAccelSurfaceId          surf;
};

/*
 * Accelerator flow control primitives
 */
struct VMCLQueueId {
   VMCLContextId             cid;
   VMAccelId                 id;
};

struct VMCLQueueAllocateDesc {
   VMCLQueueId               client;
   VMAccelQueueDesc          desc;
};

struct VMCLEventId {
   VMCLContextId             cid;
   VMAccelId                 id;
};

struct VMCLEventAllocateDesc {
   VMCLEventId               client;
   VMAccelEventDesc          desc;
};

struct VMCLFenceId {
   VMCLContextId             cid;
   VMAccelId                 id;
};

struct VMCLFenceAllocateDesc {
   VMCLFenceId               client;
   VMAccelFenceDesc          desc;
};

/*
 * Accelerator queue flush operation, executes operations in the supplied
 * queue. If any of the referenced objects are not available, the operation
 * as a whole is discarded. For this reason, use of shared surfaces in a
 * queue is not recommended.
 */
struct VMCLQueueFlushOp {
   VMCLQueueId               queue;
};

/*
 * Accelerator operations. If qid is zero, then operation is dispatched
 * immediately, otherwise operation is inserted into the supplied queue
 * and deferred.
 */
/*
 * Accelerator event insertion operation.
 */
struct VMCLEventInsertOp {
   VMCLQueueId               queue;
   VMAccelId                 id;
};

/*
 * Accelerator fence insertion operation.
 */
struct VMCLFenceInsertOp {
   VMCLQueueId               queue;
   VMAccelId                 id;
};

/*
 * Accelerator surface copy operation.
 */
struct VMCLSurfaceCopyOp {
   VMCLQueueId               queue;
   VMCLSurfaceId             dst;
   VMCLSurfaceId             src;
   VMAccelSurfaceCopyOp      op;
};

/*
 * Accelerator image fill operation.
 */
struct VMCLImageFillOp {
   VMCLQueueId               queue;
   VMCLSurfaceId             img;
   VMAccelImageFillOp        op;
};

/*
 * Accelerator image transfer operations
 */
struct VMCLImageUploadOp {
   VMCLQueueId               queue;
   VMCLSurfaceId             img;

   VMAccelImageTransferOp           op;
   VMAccelSurfaceWriteConsistency   mode;
};

struct VMCLImageDownloadOp {
   VMCLQueueId               queue;
   VMCLSurfaceId             img;

   VMAccelImageTransferOp        op;
   VMAccelSurfaceReadConsistency mode;
};

/*
 * Accelerator buffer map/unmap operations
 */
struct VMCLSurfaceMapOp {
   VMCLQueueId               queue;
   VMAccelSurfaceMapOp       op;
};

struct VMCLSurfaceUnmapOp {
   VMCLQueueId               queue;
   VMAccelSurfaceUnmapOp     op;
};

/*
 * Sampler operations.
 */
struct VMCLSamplerId {
   VMCLContextId             cid;
   VMAccelId                 id;
};

/*
 * Sampler addressing requiring VMACCEL_IMAGING.
 */
enum VMCLSamplerAddressingMode {
   VMCL_ADDRESS_MIRRORED_REPEAT,
   VMCL_ADDRESS_REPEAT,
   VMCL_ADDRESS_CLAMP_TO_EDGE,
   VMCL_ADDRESS_CLAMP,
   VMCL_ADDRESS_NONE
};

enum VMCLSamplerFilterMode {
   VMCL_FILTER_NEAREST,
   VMCL_FILTER_LINEAR
};

/*
 * Accelerator sampler allocation descriptor structure.
 */
struct VMCLSamplerDesc {
   bool                      normalizedCoords;

   /*
    * Requires VMACCEL_IMAGING.
    */
   VMCLSamplerAddressingMode addressMode;
   VMCLSamplerFilterMode     filterMode;
};

struct VMCLSamplerAllocateDesc {
   VMCLSamplerId             client;
   VMCLSamplerDesc           desc;
};

/*
 * Accelerator sampler allocation.
 */
struct VMCLSamplerAllocateStatus {
   VMAccelStatusCode         status;
};

/*
 * Compute kernel operations
 */
struct VMCLKernelId {
   VMCLContextId             cid;
   VMAccelId                 id;
};

/*
 * Accelerator kernel allocation descriptor structure.
 */
struct VMCLKernelAllocateDesc {
   VMCLKernelId              client;
   VMCLKernelLanguage        language;
   opaque                    source<>;

   /*
    * Name of the kernel in the source/binary.
    */
   opaque                    kernelName<>;
};

/*
 * Accelerator kernel allocation.
 */
struct VMCLKernelAllocateStatus {
   VMAccelStatusCode         status;
};

/*
 * Accelerator kernel semantic names.
 */
enum VMCLKernelSemanticType {
   VMCL_SEMANTIC_DECLARED,
   VMCL_SEMANTIC_WORK_DIMENSION,
   VMCL_SEMANTIC_GLOBAL_ID,
   VMCL_SEMANTIC_GLOBAL_SIZE,
   VMCL_SEMANTIC_GLOBAL_OFFSET,
   VMCL_SEMANTIC_GROUP_ID,
   VMCL_SEMANTIC_LOCAL_ID,
   VMCL_SEMANTIC_LOCAL_SIZE,
   VMCL_SEMANTIC_NUM_GROUPS,
   VMCL_SEMANTIC_MAX
};

enum VMCLKernelArgType {
   VMCL_ARG_IMMEDIATE,
   VMCL_ARG_SURFACE,
   VMCL_ARG_SAMPLER
};

/*
 * Accelerator kernel argument descriptor, immediate mode arguments are not
 * perimitted due to low-level data consistency contracts.
 */
struct VMCLKernelArgDesc {
   unsigned int              index;
   VMCLKernelArgType         type;
   VMAccelSurfaceUsage       usage;
   VMAccelSurfaceId          surf;
   VMCLSamplerId             sampler;
};

/*
 * Dispatch operation.
 */
struct VMCLDispatchOp {
   VMCLQueueId               queue;
   VMCLKernelId              kernel;

   unsigned int              dimension;
   unsigned int              globalWorkOffset<>;
   unsigned int              globalWorkSize<>;
   unsigned int              localWorkSize<>;

   /*
    * Parameters and runtime objects:
    *
    * clSetKernelArg
    * clSetKernelArgSVMPointer
    * clSetKernelExecInfo
    */
   VMCLKernelArgDesc         args<>;
   VMAccelSurfaceId          refs<>;
};

/*
 * The result of a Context allocation operation.
 */
union VMCLContextAllocateReturnStatus switch (int errno) {
   case 0:
      VMCLContextAllocateStatus *ret;
   default:
      void;
};

/*
 * The result of a Sampler allocation operation.
 */
union VMCLSamplerAllocateReturnStatus switch (int errno) {
   case 0:
      VMCLSamplerAllocateStatus *ret;
   default:
      void;
};

/*
 * The result of a Kernel allocation operation.
 */
union VMCLKernelAllocateReturnStatus switch (int errno) {
   case 0:
      VMCLKernelAllocateStatus *ret;
   default:
      void;
};

/*
 * VM Accelerator program definition.
 */
program VMCL {
   version VMCL_VERSION {
      /*
       * Context allocation/destruction.
       */
      VMCLContextAllocateReturnStatus
         VMCL_CONTEXTALLOC(VMCLContextAllocateDesc) = 1;
      VMAccelReturnStatus
         VMCL_CONTEXTDESTROY(VMCLContextId) = 2;

      /*
       * Surface allocation/destruction.
       */
      VMAccelSurfaceAllocateReturnStatus
         VMCL_SURFACEALLOC(VMCLSurfaceAllocateDesc) = 3;
      VMAccelReturnStatus
         VMCL_SURFACEDESTROY(VMCLSurfaceId) = 4;
 
      /*
       * Shared surface interop support.
       */
      VMAccelSharedHandleReturnStatus
         VMCL_SURFACEGETSHAREDHANDLE(VMCLSurfaceId) = 5;
      VMAccelReturnStatus
         VMCL_SURFACERELEASESHAREDHANDLE(VMCLSharedHandle) = 6;

      /*
       * Flow control primitives allocation/destruction.
       */
      VMAccelQueueReturnStatus
         VMCL_QUEUEALLOC(VMCLQueueAllocateDesc) = 7;
      VMAccelReturnStatus
         VMCL_QUEUEDESTROY(VMCLQueueId) = 8;

      VMAccelEventReturnStatus
         VMCL_EVENTALLOC(VMCLEventAllocateDesc) = 9;
      VMAccelEventReturnStatus
         VMCL_EVENTGETSTATUS(VMCLEventId) = 10;
      VMAccelEventReturnStatus
         VMCL_EVENTDESTROY(VMCLEventId) = 11;

      VMAccelFenceReturnStatus
         VMCL_FENCEALLOC(VMCLFenceAllocateDesc) = 12;
      VMAccelFenceReturnStatus
         VMCL_FENCEGETSTATUS(VMCLFenceId) = 13;
      VMAccelFenceReturnStatus
         VMCL_FENCEDESTROY(VMCLFenceId) = 14;

      VMAccelReturnStatus
         VMCL_QUEUEFLUSH(VMCLQueueId) = 15;

      /*
       * Accelerator basic flow control operations.
       */
      VMAccelReturnStatus
         VMCL_EVENTINSERT(VMCLEventInsertOp) = 16;
      VMAccelReturnStatus
         VMCL_FENCEINSERT(VMCLFenceInsertOp) = 17;


      /*
       * Basic surface content operations.
       */
      VMAccelReturnStatus
         VMCL_IMAGEUPLOAD(VMCLImageUploadOp) = 18;
      VMAccelDownloadReturnStatus
         VMCL_IMAGEDOWNLOAD(VMCLImageDownloadOp) = 19;

      VMAccelSurfaceMapReturnStatus
         VMCL_SURFACEMAP(VMCLSurfaceMapOp) = 20;
      VMAccelReturnStatus
         VMCL_SURFACEUNMAP(VMCLSurfaceUnmapOp) = 21;

      /*
       * Accelerated surface content operations.
       */
      VMAccelReturnStatus
         VMCL_SURFACECOPY(VMCLSurfaceCopyOp) = 22;
      VMAccelReturnStatus
         VMCL_IMAGEFILL(VMCLImageFillOp) = 23;

      /*
       * Accelerator specific operations.
       */
      VMCLSamplerAllocateReturnStatus
         VMCL_SAMPLERALLOC(VMCLSamplerAllocateDesc) = 24;
      VMAccelReturnStatus
         VMCL_SAMPLERDESTROY(VMCLSamplerId) = 25;

      VMCLKernelAllocateReturnStatus
         VMCL_KERNELALLOC(VMCLKernelAllocateDesc) = 26;
      VMAccelReturnStatus
         VMCL_KERNELDESTROY(VMCLKernelId) = 27;

      /*
       * Compute Kernel dispatch operation.
       */
      VMAccelReturnStatus
         VMCL_DISPATCH(VMCLDispatchOp) = 28;
  } = 1;
} = 0x20000081;
