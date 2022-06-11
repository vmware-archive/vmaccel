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

#ifndef _VMACCEL_DEFS_H_
#define _VMACCEL_DEFS_H_ 1

/*
 * VMAccelerator features
 */
#ifndef ENABLE_NON_BLOCKING_SOCKETS
#define ENABLE_NON_BLOCKING_SOCKETS 0
#endif

#ifndef ENABLE_DATA_STREAMING
#define ENABLE_DATA_STREAMING 0
#endif

#ifndef VMACCEL_VMCL_BASE_PORT
#define VMACCEL_VMCL_BASE_PORT 5100
#endif

#ifndef VMACCEL_MAX_STREAMS
#define VMACCEL_MAX_STREAMS 4
#endif

#define VMACCEL_MAX_SURFACE_INSTANCE 1
#define VMACCEL_STREAM_PRIORITY_DELTA 1

#ifndef ENABLE_IMAGE_DOWNLOAD
#define ENABLE_IMAGE_DOWNLOAD 0
#endif

/*
 * VMAccelerator global definitions.
 */
#define VMACCEL_MAX_ACCELERATORS 256
#define VMACCEL_MAX_CONTEXTS 4
#define VMACCEL_MAX_SURFACES 8
#define VMACCEL_MAX_KERNELS 4
#define VMACCEL_MAX_REF_OBJECTS                                                \
   (VMACCEL_MAX_CONTEXTS * (VMACCEL_MAX_SURFACES + VMACCEL_MAX_KERNELS))
#define VMACCEL_INVALID_ID -1

typedef enum VMAccelStatusCodeEnum {
   VMACCEL_SUCCESS = 0,
   VMACCEL_FAIL = 1,
   VMACCEL_SEMANTIC_ERROR = 2,
   VMACCEL_DEVICE_ERROR = 3,
   VMACCEL_RESOURCE_UNAVAILABLE = 4,
   VMACCEL_DEVICE_LOST = 5,
   VMACCEL_OUT_OF_COMPUTE_RESOURCES = 6,
   VMACCEL_OUT_OF_MEMORY = 7,
   VMACCEL_TIMEOUT = 8,
   VMACCEL_STATUS_CODE_MAX = 9,
} VMAccelStatusCodeEnum;

/*
 * Resource type hierarchy
 *
 * Resource
 * +-> Execution Callback
 *     Interconnect
 *     Processor
 *        +-> Memory (Local)
 *        +-> Router
 *
 * Example accelerator fabric for Big Data Visualization:
 *
 *   <=+== Interconnect ==+===============+=>
 *     |                  |               |
 *     + Compute          + Graphics      + Video Encode
 *       Accelerator        Accelerator     Accelerator
 *       + Memory           + Memory
 */

typedef enum VMAccelResourceTypeEnum {
   VMACCEL_NONE = 0,
   VMACCEL_CALLBACK = 1,
   VMACCEL_INTERCONNECT = 2,
   VMACCEL_MEMORY = 3,
   VMACCEL_SURFACE = 4,
   VMACCEL_COMPUTE_ACCELERATOR = 5,
   VMACCEL_TYPE_MAX = 6,
} VMAccelResourceTypeEnum;

#define VMACCEL_CALLBACK_MASK (1 << VMACCEL_CALLBACK)
#define VMACCEL_INTERCONNECT_MASK (1 << VMACCEL_INTERCONNECT)
#define VMACCEL_MEMORY_MASK (1 << VMACCEL_MEMORY)
#define VMACCEL_SURFACE_MASK (1 << VMACCEL_SURFACE)
#define VMACCEL_COMPUTE_ACCELERATOR_MASK (1 << VMACCEL_COMPUTE_ACCELERATOR)

typedef enum VMAccelArchitectureEnum {
   VMACCEL_GPU = 0,
   VMACCEL_FPGA = 1,
   VMACCEL_ASIC = 2,
   VMACCEL_CPU = 3,
   VMACCEL_SELECT_MAX = 4,
} VMAccelArchitectureEnum;

#define VMACCEL_AUTO_SELECT_MASK 0
#define VMACCEL_GPU_MASK (1 << VMACCEL_GPU)
#define VMACCEL_FPGA_MASK (1 << VMACCEL_FPGA)
#define VMACCEL_ASIC_MASK (1 << VMACCEL_ASIC)
#define VMACCEL_CPU_MASK (1 << VMACCEL_CPU)
#define VMACCEL_SELECT_MASK ((1 << VMACCEL_SELECT_MAX) - 1)

typedef enum VMAccelCapsEnum {
   /*
    * Shared memory interop is possible between accelerator servers.
    */
   VMACCEL_INTEROP = 0,

   /*
    * Callbacks supported for asynchronous feedback from events.
    */
   VMACCEL_CALLBACKS = 1,

   /*
    * Arbitrary halting is available for an accelerator.
    */
   VMACCEL_HALT = 2,

   /*
    * Support for surfaces and images.
    */
   VMACCEL_IMAGE = 3,

   /*
    * Mapping of a surface to the client's address space, and the necessary
    * data consistency contracts to keep the client and server in sync.
    *
    * Note: Servers must handle garbage collection of open surface
    *       mappings.
    */
   VMACCEL_SURFACEMAP = 4,

   /*
    * Shared Virtual Memory is available when defining/using a surface.
    *
    * Fine-grained SVM is an optimization to this if the server and client
    * exist on the same host.
    */
   VMACCEL_SVM = 5,

   /*
    * Peer-to-peer communication between servers.
    *
    * Requires secure tokens...
    */
   VMACCEL_P2P = 6,

   VMACCEL_CAP_MAX = 7,
} VMAccelCapsEnum;

#define VMACCEL_INTEROP_CAP (1 << VMACCEL_INTEROP)
#define VMACCEL_CALLBACKS_CAP (1 << VMACCEL_CALLBACKS)
#define VMACCEL_HALT_CAP (1 << VMACCEL_HALT)
#define VMACCEL_IMAGE_CAP (1 << VMACCEL_IMAGE)
#define VMACCEL_MAPSURFACE_CAP (1 << VMACCEL_MAPSURFACE)
#define VMACCEL_SVM_CAP (1 << VMACCEL_SVM)
#define VMACCEL_P2P_CAP (1 << VMACCEL_P2P)
#define VMACCEL_CAP_MASK ((1 << VMACCEL_CAP_MAX) - 1)

/*
 * Surface type hierarchy
 *
 * +--------------------------------+
 * | Surface                        |
 * |  +--------------------------+  |
 * |  | Image                    |  |
 * |  |  +--------------------+  |  |
 * |  |  | Buffer             |  |  |
 * |  |  +--------------------+  |  |
 * |  |  | ...                +  |  |
 * |  |  +--------------------+  |  |
 * |  +--------------------------+  |
 * |  | ...                      |  |
 * |  +--------------------------+  |
 * |                                |
 * +--------------------------------+
 */
typedef enum VMAccelSurfacePoolEnum {
   VMACCEL_SURFACE_POOL_AUTO,
   VMACCEL_SURFACE_POOL_ACCELERATOR,
   VMACCEL_SURFACE_POOL_SYSTEM_MEMORY,
} VMAccelSurfacePoolEnum;

typedef enum VMAccelSurfaceTypeEnum {
   /*
    * Shared Virtual Memory surface type, can be accessed by any context
    * for this accelerator as a memory address. However, access is limited
    * to the associated accelerator server. Interop between accelerator
    * servers requires VMACCEL_SURFACE_BUFFER with VMACCEL_BIND_INTEROP_FLAG.
    */
   VMACCEL_SURFACE_SVM,
   VMACCEL_SURFACE_BUFFER,
   VMACCEL_SURFACE_1D_IMAGE,
   VMACCEL_SURFACE_2D_IMAGE,
   VMACCEL_SURFACE_3D_IMAGE,
   VMACCEL_SURFACE_TYPE_MAX,
} VMAccelSurfaceTypeEnum;

typedef enum VMAccelSurfaceUsageEnum {
   VMACCEL_SURFACE_USAGE_READONLY,
   VMACCEL_SURFACE_USAGE_WRITEONLY,
   VMACCEL_SURFACE_USAGE_READWRITE,
} VMAccelSurfaceUsageEnum;

typedef enum VMAccelSurfaceReadConsistencyEnum {
   VMACCEL_SURFACE_READ_SYNCHRONOUS,
   VMACCEL_SURFACE_READ_ASYNCHRONOUS,
} VMAccelSurfaceReadConsistencyEnum;

typedef enum VMAccelSurfaceWriteConsistencyEnum {
   VMACCEL_SURFACE_WRITE_DISCARD,
   VMACCEL_SURFACE_NO_OVERWRITE,
   VMACCEL_SURFACE_WRITE_ASYNCHRONOUS,
} VMAccelSurfaceWriteConsistencyEnum;

typedef enum VMAccelSurfaceMapEnum {
   VMACCEL_SURFACE_MAP_READ,
   VMACCEL_SURFACE_MAP_WRITE,
   VMACCEL_SURFACE_MAP_WRITE_DISCARD,
   VMACCEL_SURFACE_MAP_NO_OVERWRITE,
   VMACCEL_SURFACE_MAP_NO_FREE_PTR,
   VMACCEL_SURFACE_MAP_ASYNC,
   VMACCEL_SURFACE_MAP_MAX,
} VMAccelSurfaceMapEnum;

#define VMACCEL_MAP_READ_FLAG (1 << VMACCEL_SURFACE_MAP_READ)
#define VMACCEL_MAP_WRITE_FLAG (1 << VMACCEL_SURFACE_MAP_WRITE)
#define VMACCEL_MAP_WRITE_DISCARD_FLAG (1 << VMACCEL_SURFACE_MAP_WRITE_DISCARD)
#define VMACCEL_MAP_NO_OVERWRITE_FLAG (1 << VMACCEL_SURFACE_MAP_NO_OVERWRITE)
#define VMACCEL_MAP_NO_FREE_PTR_FLAG (1 << VMACCEL_SURFACE_MAP_NO_FREE_PTR)
#define VMACCEL_MAP_ASYNC_FLAG (1 << VMACCEL_SURFACE_MAP_ASYNC)
#define VMACCEL_MAP_MASK ((1 << VMACCEL_SURFACE_MAP_MAX) - 1)

typedef enum VMAccelPipelineBindPointsEnum {
   /*
    * Basic surface topologies.
    */
   VMACCEL_BIND_SVM,
   VMACCEL_BIND_BUFFER,
   VMACCEL_BIND_SURFACE_1D_IMAGE,
   VMACCEL_BIND_SURFACE_2D_IMAGE,
   VMACCEL_BIND_SURFACE_3D_IMAGE,

   /*
    * Buffer access bind flags.
    */
   VMACCEL_BIND_INTEROP,
   VMACCEL_BIND_STAGING,
   VMACCEL_BIND_CONSTANT_BUFFER,
   VMACCEL_BIND_SHADER_RESOURCE,
   VMACCEL_BIND_UNORDERED_ACCESS,

   VMACCEL_BIND_MAX,
} VMAccelPipelineBindPointsEnum;

#define VMACCEL_BIND_SVM_FLAG (1 << VMACCEL_BIND_SVM)
#define VMACCEL_BIND_BUFFER_FLAG (1 << VMACCEL_BIND_BUFFER)
#define VMACCEL_BIND_SURFACE_1D_IMAGE_FLAG (1 << VMACCEL_BIND_SURFACE_1D_IMAGE)
#define VMACCEL_BIND_SURFACE_2D_IMAGE_FLAG (1 << VMACCEL_BIND_SURFACE_2D_IMAGE)
#define VMACCEL_BIND_SURFACE_3D_IMAGE_FLAG (1 << VMACCEL_BIND_SURFACE_3D_IMAGE)
#define VMACCEL_BIND_INTEROP_FLAG (1 << VMACCEL_BIND_INTEROP)
#define VMACCEL_BIND_CONSTANT_BUFFER_FLAG (1 << VMACCEL_BIND_CONSTANT_BUFFER)
#define VMACCEL_BIND_SHADER_RESOURCE_FLAG (1 << VMACCEL_BIND_SHADER_RESOURCE)
#define VMACCEL_BIND_UNORDERED_ACCESS_FLAG (1 << VMACCEL_BIND_UNORDERED_ACCESS)
#define VMACCEL_BIND_MASK ((1 << VMACCEL_BIND_MAX) - 1)

typedef enum VMAccelHandleTypeEnum {
   VMACCEL_HANDLE_ID,
   VMACCEL_HANDLE_HOST_SHARED_FILE,
   VMACCEL_HANDLE_HOST_SHARED_GPU_API,
} VMAccelHandleTypeEnum;

/*
 * Format types.
 */
typedef enum VMAccelSurfaceFormatEnum {
/*
 * Vectors.
 */
#include "vmaccel_formats_enum.h"

   VMACCEL_FORMAT_MAX,

   /*
    * Scalars.
    */
   VMACCEL_FORMAT_BYTE = VMACCEL_FORMAT_R8_SINT,
   VMACCEL_FORMAT_UBYTE = VMACCEL_FORMAT_R8_UINT,
   VMACCEL_FORMAT_SHORT = VMACCEL_FORMAT_R16_SINT,
   VMACCEL_FORMAT_USHORT = VMACCEL_FORMAT_R16_UINT,
   VMACCEL_FORMAT_INT = VMACCEL_FORMAT_R32_SINT,
   VMACCEL_FORMAT_UINT = VMACCEL_FORMAT_R32_UINT,
   VMACCEL_FORMAT_FLOAT = VMACCEL_FORMAT_R32_FLOAT,
} VMAccelSurfaceFormatEnum;

/*
 * Format caps are as follows:
 *
 *   <surface types>
 *   <bind flags>
 */
#define VMACCEL_FORMAT_CAP_SURFACE_TYPE_SHIFT 0
#define VMACCEL_FORMAT_CAP_BIND_START_INDEX VMACCEL_SURFACE_TYPE_MAX
#define VMACCEL_FORMAT_CAP_MAX                                                 \
   (VMACCEL_FORMAT_CAP_BIND_START_INDEX + VMACCEL_BIND_MAX)

#define VMACCEL_FORMAT_CAP_MASK ((1 << VMACCEL_FORMAT_CAP_MAX) - 1)

/*
 * Flow control and execution definitions.
 */
typedef enum VMAccelQueueFlagsEnum {
   VMACCEL_QUEUE_ON_DEVICE,
   VMACCEL_QUEUE_OUT_OF_ORDER_EXEC,
   VMACCEL_QUEUE_ENABLE_PROFILING,
   VMACCEL_QUEUE_MAX_FLAGS,
} VMAccelQueueFlagsEnum;

#define VMACCEL_QUEUE_ON_DEVICE_FLAG (1 << VMACCEL_QUEUE_ON_DEVICE)
#define VMACCEL_QUEUE_OUT_OF_ORDER_EXEC_FLAG                                   \
   (1 << VMACCEL_QUEUE_OUT_OF_ORDER_EXEC)
#define VMACCEL_QUEUE_ENABLE_PROFILING_FLAG                                    \
   (1 << VMACCEL_QUEUE_ENABLE_PROFILING)
#define VMACCEL_QUEUE_FLAG_MASK ((1 << VMACCEL_QUEUE_MAX_FLAGS) - 1)

#endif /* _VMACCEL_DEFS_H_ */
