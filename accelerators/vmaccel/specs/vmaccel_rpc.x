/******************************************************************************

Copyright (c) 2016-2022 VMware, Inc.
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
%#include "vmaccel_defs.h"
#endif

/*
 * Accelerator RPC base definition.
 *
 * Accelerator RPC communication occurs at IP granularity, therefore fan-out of
 * a server's accelerator pool should be managed carefully.
 *
 * Example accelerator fabric:
 *
 *   <=+== Interconnect ==+=================>
 *     |                  |
 *     + Compute          + Graphics
 *       Accelerator Pool   Accelerator Pool
 *       + Accelerator      + Accelerator
 *         + CPU              + GPU
 *         + Memory           + Memory
 *           + Surface          + Surface
 */

/*
 * Used for communication validation (64-bits).
 */
const VMACCEL_MAX_NONCE_SIZE = 4;

/*
 * Used to locate an object in the VMAccel world (64-bits).
 */
const VMACCEL_MAX_LOCATION_SIZE = 4;

typedef unsigned int VMAccelStatusCode;
typedef unsigned int VMAccelResourceType;
typedef unsigned int VMAccelArchitectureType;

/*
 * CT_ASSERT(VMACCEL_SELECT_MAX <= 32);
 */
typedef unsigned int VMAccelSelectionMask;

/*
#ifndef RPC_HDR
%ct_assert(VMACCEL_CAP_MAX <= 32);
#endif
*/

typedef unsigned int VMAccelId;
typedef unsigned int VMAccelCaps;
typedef unsigned int VMAccelSurfacePool;
typedef unsigned int VMAccelSurfaceType;
typedef unsigned int VMAccelSurfaceUsage;
typedef unsigned int VMAccelSurfaceMapFlags;
typedef unsigned int VMAccelSurfaceReadConsistency;
typedef unsigned int VMAccelSurfaceWriteConsistency;

/*
 * CT_ASSERT(VMACCEL_BIND_MAX <= 32);
 */
typedef unsigned int VMAccelFormatCaps;
typedef unsigned int VMAccelSurfaceBindFlags;
typedef unsigned int VMAccelSurfaceFormat;

/*
 * CT_ASSERT(VMACCEL_FORMAT_CAP_MAX_SHIFT <= 32);
 */
typedef unsigned int VMAccelSurfaceFormatCaps;

/*
 * Accelerator return structure.
 */
struct VMAccelStatus {
   opaque                 nonce<VMACCEL_MAX_NONCE_SIZE>;
   VMAccelStatusCode      status;
};

/*
 * Accelerator address structure.
 *
 * Used by each accelerator to address local and non-local resources.
 * The following diagram illustrates the addressing hierarchy for an
 * Accelerator. Each arrow represents an abstraction boundary transition.
 *
 * (Process Id, Client Id) -> Global Id
 * (Client IP, Global Id) -> Server Local Id
 *
 * Example:
 *
 * (0, 1) -> 2 -> (Client IP, 2) within context -> Context Local Id
 *
 */
struct VMAccelAddress {
   opaque                    addr<VMACCEL_MAX_LOCATION_SIZE>;
   unsigned int              port;

   /*
    * Encode the accelerator type so routing code can direct access
    * appropriately.
    *
    * See: inc/vmaccel_defs.h::VMAccelResourceTypeEnum
    */
   VMAccelResourceType       resourceType;
   unsigned int              subDevice;
};

/*
 * Accelerator callback structure.
 *
 * A callback is a stateless callback with an endpoint that only has knowledge
 * of the data it expects. If that data is not received, it is ignored.
 */
struct VMAccelCallback {
   VMAccelAddress            addr;
   opaque                    payload<>;
};

/*
 * Per-format desc structure.
 */
struct VMAccelFormatDesc {
   unsigned int              format;
   VMAccelFormatCaps         caps;
};

/*
 * Accelerator workload descriptor.
 */
struct VMAccelWorkloadDesc {
   /*
    * Accelerator speed quantified in millions of *ops/sec.
    */
   unsigned int              megaFlops;
   unsigned int              megaOps;

   /*
    * Cache hierarchy size and bandwidth, llc --> Last Level Cache.
    */
   unsigned int              llcSizeKB;
   unsigned int              llcBandwidthMBSec;

   /*
    * Local memory hierarchy size and bandwidth.
    */
   unsigned int              localMemSizeKB;
   unsigned int              localMemBandwidthMBSec;

   /*
    * Non-local memory hierarchy size and bandwidth.
    */
   unsigned int              nonLocalMemSizeKB;
   unsigned int              nonLocalMemBandwidthMBSec;

   /*
    * Interconnect bandwidth, e.g. PCI-E, USB, Fibre Channel, etc...
    */
   unsigned int              interconnectBandwidthMBSec;
};

/*
 * Accelerator descriptor structure.
 */
struct VMAccelDesc {
   /*
    * The parent object in the hierarchy. This is typically an interconnect or
    * compute resource.
    */
   VMAccelId                 parentId;
   VMAccelAddress            parentAddr;

   VMAccelResourceType       typeMask;
   VMAccelArchitectureType   architecture;
   VMAccelCaps               caps;
   VMAccelFormatDesc         formatCaps<>;
   VMAccelWorkloadDesc       capacity;

   /*
    * Generic tracked state objects.
    */
   unsigned int              maxContexts;
   unsigned int              maxQueues;
   unsigned int              maxEvents;
   unsigned int              maxFences;
   unsigned int              maxSurfaces;
   unsigned int              maxMappings;
};

/*
 * Accelerator resource descriptor structure.
 */
struct VMAccelResourceDesc {
   /*
    * The parent object in the hierarchy. This is typically an interconnect or
    * compute resource.
    */
   VMAccelId                 parentId;
   VMAccelAddress            parentAddr;

   VMAccelResourceType       typeMask;
   VMAccelSelectionMask      selectionMask;
   VMAccelCaps               caps;
   unsigned int              num;

   /*
    * Memory allocation parameters, e.g. calloc.
    */
   unsigned int              size;
};

/*
 * Accelerator allocation.
 */
struct VMAccelAllocateStatus {
   VMAccelStatusCode         status;
   VMAccelId                 id;
   VMAccelDesc               desc;   
};

/*
 * Accelerator resource registration descriptor structure.
 */
struct VMAccelRegisterDesc {
   VMAccelDesc               desc;
   VMAccelResourceType       typeMask;
};

/*
 * Accelerator resource allocation.
 */
struct VMAccelResourceAllocateStatus {
   VMAccelStatusCode         status;
   VMAccelId                 id;
};

/*
 * Accelerator resource registration descriptor structure.
 */
struct VMAccelRegisterAllocationDesc {
   VMAccelAddress            parentAddr;
   VMAccelResourceType       typeMask;
   unsigned int              num;

   /*
    * Memory allocation parameters, e.g. calloc.
    */
   unsigned int              size;
};

/*
 * Accelerator resource registration.
 */
struct VMAccelRegisterStatus {
   VMAccelStatusCode         status;
   VMAccelId                 id;
};

/*
 * Surface and image element definitions.
 */
struct VMAccelElementDouble4D {
   double x;
   double y;
   double z;
   double w;
};

struct VMAccelElementFloat4D {
   float x;
   float y;
   float z;
   float w;
};

struct VMAccelElementUINT4D {
   unsigned int r;
   unsigned int g;
   unsigned int b;
   unsigned int a;
};

struct VMAccelCoordinate2DUINT {
   unsigned int x;
   unsigned int y;
};

struct VMAccelCoordinate3DUINT {
   unsigned int x;
   unsigned int y;
   unsigned int z;
};

struct VMAccelCoordinate4DUINT {
   unsigned int x;
   unsigned int y;
   unsigned int z;
   unsigned int w;
};

/*
 * Surface, image, and buffer addressing topology.
 */
struct VMAccelSurfaceDesc {
   /*
    * Identifier for parent object, e.g. shared surface or explicit
    * accelerator assignment.
    */
   VMAccelId                 parentId;

   /*
    * Type of the surface.
    */
   VMAccelSurfaceType        type;

   /*
    * Size in increasing dimension order.
    */
   unsigned int              width;
   unsigned int              height;
   unsigned int              depth;
   unsigned int              mipLevels;
   unsigned int              arraySize;
   unsigned int              samples;

   VMAccelSurfaceFormat      format;
   VMAccelSurfaceFormatCaps  formatCaps;

   /*
    * Surface pool.
    */
   VMAccelSurfacePool        pool;

   /*
    * Usage hints.
    */
   VMAccelSurfaceUsage       usage;

   /*
    * Pipeline stage bind flags.
    */
   VMAccelSurfaceBindFlags   bindFlags;

   /*
    * Byte addressing pitch values, may differ for block formats.
    */
   unsigned int              slicePitch;
   unsigned int              rowPitch;
};

/*
 * Identifier and location values are in increasing detail order.
 */

/*
 * Shared handles are active for the lifetime of the origin accelerator
 * context.
 */
typedef unsigned int VMAccelHandleType;

struct VMAccelSurfaceId {
   VMAccelSurfaceType        type;
   VMAccelHandleType         handleType;
   VMAccelId                 id;
   unsigned int              instance;
   unsigned int              offset;
   unsigned int              generation;
};

/*
 * Acclerator surface allocation.
 */
struct VMAccelSurfaceAllocateStatus {
   VMAccelStatusCode         status;
   VMAccelSurfaceDesc        surfaceDesc;
};

/*
 * Accelerator flow control primitives and operations.
 */

/*
 * Queues are objects defined on the *host* for deferred execution.
 */
typedef unsigned int VMAccelQueueFlags;

/*
 * CT_ASSERT(VMAccelQueueFlags << 31);
 */
struct VMAccelQueueDesc {
   VMAccelQueueFlags         flags;
   unsigned int              size;
};

struct VMAccelQueueStatus {
   VMAccelStatusCode         status;
};

/*
 * Accelerator surface copy operation.
 */
struct VMAccelSurfaceRegion {
   unsigned int              mipLevel;
   VMAccelCoordinate3DUINT   coord;
   VMAccelCoordinate3DUINT   size;
};

struct VMAccelSurfaceCopyOp {
   VMAccelSurfaceRegion      dstRegion;
   VMAccelSurfaceRegion      srcRegion;
};

/*
 * Accelerator image fill operation.
 */
struct VMAccelImageFillOp {
   VMAccelSurfaceRegion      dstRegion;

   /*
    * Fill values for different format types.
    */
   VMAccelElementDouble4D    d;
   VMAccelElementFloat4D     f;
   VMAccelElementUINT4D      u;
};

/*
 * Accelerator image transfer operation.
 */
struct VMAccelImageTransferOp {
   VMAccelSurfaceRegion      imgRegion;

   /*
    * Upload source memory, download destination for SVM/DMA mode only
    * Assumes tightly packed representation.
    */
   opaque                    ptr<>;

   /*
    * Callback address that would receive the data.
    */
   VMAccelCallback           callbacks<>;
};

/*
 * Accelerator Download operation status.
 */
struct VMAccelDownloadStatus {
   VMAccelStatusCode         status;

   /*
    * Fence used to determine completion of the Download operation. Non-local
    * fences must be queried through the VMAccel server.
    */   
   VMAccelId                 fence;

   /*
    * Memory pointer to the location where contents will be copied to.
    */
   opaque                    ptr<>;
};

/*
 * Accelerator buffer map/unmap operation.
 *
 * Use sparingly, as these mechanisms keep an open mapping from the server
 * surface to the client. This mapping requires live data consistency
 * contracts. Mapping differs from Upload/Download semantics, since the
 * surface can have an implied lock if needed.
 */
struct VMAccelSurfaceMapOp {
   VMAccelSurfaceId          surf;
   VMAccelCoordinate3DUINT   coord;
   VMAccelCoordinate2DUINT   size;

   VMAccelSurfaceMapFlags    mapFlags;
};

struct VMAccelSurfaceUnmapOp {
   VMAccelSurfaceId          surf;
   VMAccelSurfaceMapFlags    mapFlags;

   /*
    * Client address.
    */
   opaque                    ptr<>;
};

/*
 * Accelerator map operation status.
 */
struct VMAccelSurfaceMapStatus {
   VMAccelStatusCode         status;

   /*
    * Client address.
    */
   opaque                    ptr<>;
};

/*
 * Advanced accelerator operations.
 */

/*
 * Accelerator compute argument structure, targeted to immediate mode usage
 * and explicit declaration of the samplers inline within the Compute Kernel.
 */
struct VMAccelComputeArgDesc {
   unsigned int              index;
   VMAccelSurfaceUsage       usage;
   VMAccelSurfaceId          surf;
   opaque                    data<>;
};

/*
 * Compute operation that is atomic and only returns control to the caller
 * when the operation on the host has completed.
 */
struct VMAccelComputeOp {
   /*
    * Binaries for the kernel, in SPIR-V.
    */
   unsigned int              kernelType;
   opaque                    kernelSource<>;
   opaque                    kernelBinary32<>;
   opaque                    kernelBinary64<>;

   /*
    * Name of the kernel in the binary.
    */
   opaque                    kernelName<>;

   /*
    * Execution parameters.
    */
   unsigned int              dimension;
   unsigned int              globalWorkOffset<>;
   unsigned int              globalWorkSize<>;
   unsigned int              localWorkSize<>;

   VMAccelComputeArgDesc     args<>;
};

struct VMAccelComputeStatus {
   VMAccelStatusCode         status;
   VMAccelComputeArgDesc     outputs<>;
};


/*
 * Generic result of a VMAccel operation.
 */
union VMAccelReturnStatus switch (int errno) {
   case 0:
      VMAccelStatus *ret;
   default:
      void;
};

/*
 * The result of a VMAccel allocation operation.
 */
union VMAccelAllocateReturnStatus switch (int errno) {
   case 0:
      VMAccelAllocateStatus *ret;
   default:
      void;
};

/*
 * The result of a VMAccel resource allocation operation.
 */
union VMAccelResourceAllocateReturnStatus switch (int errno) {
   case 0:
      VMAccelResourceAllocateStatus *ret;
   default:
      void;
};

/*
 * The result of a VMAccel resource registration operation.
 */
union VMAccelRegisterReturnStatus switch (int errno) {
   case 0:
      VMAccelRegisterStatus *ret;
   default:
      void;
};

/*
 * The result of a VMAccel queue operation.
 */
union VMAccelQueueReturnStatus switch (int errno) {
   case 0:
      VMAccelQueueStatus *ret;
   default:
      void;
};

/*
 * The result of a VMAccel surface allocation.
 */
union VMAccelSurfaceAllocateReturnStatus switch (int errno) {
   case 0:
      VMAccelSurfaceAllocateStatus *ret;
   default:
      void;
};

/*
 * The result of a VMAccel download operation.
 */
union VMAccelDownloadReturnStatus switch (int errno) {
   case 0:
      VMAccelDownloadStatus *ret;
   default:
      void;
};

/*
 * The result of a VMAccel map operation.
 */
union VMAccelSurfaceMapReturnStatus switch (int errno) {
   case 0:
      VMAccelSurfaceMapStatus *ret;
   default:
      void;
};

/*
 * The result of a Compute operation.
 */
union VMAccelComputeReturnStatus switch (int errno) {
   case 0:
      VMAccelComputeStatus *ret;
   default:
      void;
};

/*
 * VM Accelerator process definition.
 */
program VMACCEL {
   version VMACCEL_VERSION {
      VMAccelResourceAllocateReturnStatus
         VMACCEL_RESOURCEALLOC(VMAccelResourceDesc) = 1;
      VMAccelReturnStatus
         VMACCEL_RESOURCERELEASE(VMAccelId) = 2;
 
      /*
       * Compute operation is a high level atomic operation for executing
       * compute kernels. This operation will not return control to the
       * caller until the operation has completed on the host.
       */
      VMAccelComputeReturnStatus
         VMACCEL_COMPUTE(VMAccelComputeOp) = 3;
   } = 1;
} = 0x20000079;
