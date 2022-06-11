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

#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "vmaccel_stream.h"
#include "vmaccel_utils.h"
#include "vmwopencl.h"
#include "vmwopencl_utils.h"

typedef struct VMWOpenCLCaps {
   cl_device_id deviceId;

#define CAP(__TYPE, __NAME) __TYPE __NAME;
#include "vmwopencl_caps.h"
#undef CAP

   size_t *DEVICE_MAX_WORK_ITEM_SIZES;
} VMWOpenCLCaps;

typedef struct VMWOpenCLContext {
   cl_context context;
   cl_platform_id platformId;
   cl_device_id deviceIds[VMCL_MAX_SUBDEVICES];
   int majorVersion;
   int minorVersion;
   VMWOpenCLCaps *caps;
} VMWOpenCLContext;

typedef struct VMWOpenCLMapping {
   void *ptr;
   unsigned int refCount;
} VMWOpenCLMapping;

typedef struct VMWOpenCLSurfaceInstance {
   cl_mem mem;
   void *svm_ptr;
   unsigned int generation;
   VMWOpenCLMapping mapping;
   pthread_mutex_t mutex;
} VMWOpenCLSurfaceInstance;

typedef struct VMWOpenCLSurface {
   unsigned int cid;
   VMAccelSurfaceDesc desc;
   VMWOpenCLSurfaceInstance inst[VMACCEL_MAX_SURFACE_INSTANCE];
   pthread_mutex_t mutex;
} VMWOpenCLSurface;

typedef struct VMWOpenCLQueue {
   VMAccelQueueDesc desc;
   cl_command_queue queue;
} VMWOpenCLQueue;

typedef struct VMWOpenCLSampler {
   cl_sampler sampler;
} VMWOpenCLSampler;

typedef struct VMWOpenCLKernel {
   /*
    * ???: Does one program per kernel affect variable sharing?
    */
   cl_program program;
   cl_kernel kernel;
} VMWOpenCLKernel;

static VMWOpenCLCaps caps[VMACCEL_SELECT_MAX];

static VMWOpenCLContext *contexts = NULL;
static IdentifierDB *contextIds = NULL;

static VMWOpenCLSurface *surfaces = NULL;
static IdentifierDB *surfaceIds = NULL;

static VMWOpenCLQueue *queues = NULL;
static IdentifierDB *queueIds = NULL;

static VMWOpenCLSampler *samplers = NULL;
static IdentifierDB *samplerIds = NULL;

static VMWOpenCLKernel *kernels = NULL;
static IdentifierDB *kernelIds = NULL;

const cl_int clDeviceTypes[VMACCEL_SELECT_MAX] = {
   CL_DEVICE_TYPE_GPU,
   CL_DEVICE_TYPE_ACCELERATOR,
   CL_DEVICE_TYPE_ACCELERATOR,
   CL_DEVICE_TYPE_CPU,
};

VMAccelStatus *vmwopencl_poweroff();
VMAccelSurfaceMapStatus *vmwopencl_surfacemap_1(VMCLSurfaceMapOp *argp);
VMAccelStatus *vmwopencl_surfaceunmap_1(VMCLSurfaceUnmapOp *argp);

VMAccelAllocateStatus *vmwopencl_poweron(VMCLOps *ops, unsigned int accelArch,
                                         unsigned int accelIndex) {
   static VMAccelAllocateStatus result;
   size_t sizeRet;
   cl_platform_id platforms[16];
   cl_uint numPlatforms = 0;
   cl_int errNum = CL_SUCCESS;
   char platformName[128] = {'\0'};
   char platformVendor[128] = {'\0'};
   char platformVersion[128] = {'\0'};
   char platformExtensions[128] = {'\0'};
   char deviceName[128];
   char prefix[64];
   char capPrefix[128];
   cl_device_id deviceId;
   int i, j;
   bool platformFound = false;

   memset(&result, 0, sizeof(result));

   VMACCEL_LOG("Powering on vmwopencl backend...\n");

   errNum = clGetPlatformIDs(sizeof(platforms) / sizeof(platforms[0]),
                             &platforms[0], &numPlatforms);

   if (errNum != CL_SUCCESS) {
      VMACCEL_WARNING("Failed to query platforms\n");
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   VMACCEL_LOG("Number of platforms: %d\n", numPlatforms);

   for (i = 0; i < numPlatforms; i++) {
      errNum = clGetDeviceIDs(platforms[i], clDeviceTypes[accelArch], 1,
                              &deviceId, NULL);

      if (errNum != CL_SUCCESS) {
         continue;
      }

      errNum =
         clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION,
                           sizeof(platformVersion), platformVersion, &sizeRet);

      if (errNum != CL_SUCCESS) {
         VMACCEL_WARNING("Failed to query platform version\n");
         result.status = VMACCEL_FAIL;
         return (&result);
      }

      VMACCEL_LOG("Version: %s\n", platformVersion);

      errNum = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME,
                                 sizeof(platformName), platformName, &sizeRet);

      if (errNum != CL_SUCCESS) {
         VMACCEL_WARNING("Failed to query platform name\n");
         result.status = VMACCEL_FAIL;
         return (&result);
      }

      VMACCEL_LOG("Platform: %s\n", platformName);

      errNum =
         clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR,
                           sizeof(platformVendor), platformVendor, &sizeRet);

      if (errNum != CL_SUCCESS) {
         VMACCEL_WARNING("Failed to query platform vendor\n");
         result.status = VMACCEL_FAIL;
         return (&result);
      }

      VMACCEL_LOG("Vendor: %s\n", platformVendor);

      errNum = clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS,
                                 sizeof(platformExtensions), platformExtensions,
                                 &sizeRet);

      if (errNum != CL_SUCCESS) {
         VMACCEL_WARNING("Failed to query platform extensions\n");
      }

      VMACCEL_LOG("Extensions: %s\n", platformExtensions);

      caps[accelArch].deviceId = deviceId;

      errNum = clGetDeviceInfo(deviceId, CL_DEVICE_NAME, sizeof(deviceName),
                               deviceName, &sizeRet);

      if (errNum != CL_SUCCESS) {
         VMACCEL_WARNING("Unable to query CL_DEVICE_NAME\n");
         continue;
      }

      snprintf(prefix, sizeof(prefix), "Device[%p]", deviceId);

      VMACCEL_LOG("%s: DEVICE_NAME = %s\n", prefix, deviceName);

#define CAP(__TYPE, __NAME)                                                    \
   do {                                                                        \
      __TYPE val;                                                              \
      errNum = clGetDeviceInfo(deviceId, CL_##__NAME, sizeof(__TYPE), &val,    \
                               &sizeRet);                                      \
      if (errNum != CL_SUCCESS) {                                              \
         VMACCEL_WARNING("Unable to query %s\n", "CL_" #__NAME);               \
      } else {                                                                 \
         snprintf(capPrefix, sizeof(capPrefix), "%s: %s", prefix, #__NAME);    \
         Log_##__TYPE(capPrefix, val);                                         \
         caps[accelArch].__NAME = val;                                         \
      }                                                                        \
   } while (0);
#include "vmwopencl_caps.h"
#undef CAP

      caps[accelArch].DEVICE_MAX_WORK_ITEM_SIZES = calloc(
         caps[accelArch].DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t));

      errNum = clGetDeviceInfo(
         deviceId, CL_DEVICE_MAX_WORK_ITEM_SIZES,
         caps[accelArch].DEVICE_MAX_WORK_ITEM_DIMENSIONS * sizeof(size_t),
         caps[accelArch].DEVICE_MAX_WORK_ITEM_SIZES, &sizeRet);

      if (errNum != CL_SUCCESS) {
         VMACCEL_WARNING("Unable to query CL_DEVICE_MAX_WORK_ITEM_SIZES\n");
         continue;
      } else {
         for (j = 0; j < caps[accelArch].DEVICE_MAX_WORK_ITEM_DIMENSIONS; j++) {
            snprintf(capPrefix, sizeof(capPrefix),
                     "%s: DEVICE_MAX_WORK_ITEM_SIZES[%d]", prefix, j);
            Log_size_t(capPrefix,
                       caps[accelArch].DEVICE_MAX_WORK_ITEM_SIZES[j]);
         }
      }

      /*
       * Report back first platform found.
       */
      platformFound = true;
      break;
   }

   if (!platformFound) {
      result.status = VMACCEL_FAIL;
      return &result;
   }

   if ((strstr(platformVersion, "OpenCL 1.2") == NULL) &&
       (strstr(platformVersion, "OpenCL 2.0") == NULL) &&
       (strstr(platformVersion, "OpenCL 2.") == NULL) &&
       (strstr(platformVersion, "OpenCL 3.") == NULL)) {
      VMACCEL_WARNING("Unknown version detected: %s\n", platformVersion);
   }

   /*
    * Derive VMAccelAllocateStatus from the OpenCL device.
    *
    * We have shown that when there is a contention for an Accelerator's
    * local memory resource, the cost of paging out a resource and paging
    * in new content for a resource is not only the transfer of the data.
    * Since, a high level API interface can only enqueue paging operations,
    * all previous operations must complete before paging can occur. If
    * this ultimate blocks the backend, due to an operating system level
    * response to a lack of physical memory to pin for the DMA operation,
    * the backpressure is realized as "resource contention latency".
    *
    * The goal is to never over-commit the processor, but saturate it to
    * ~99% usage and leave a portion of the processor time for management
    * tasks. We achieve this by exporting the capabilities below to
    * maintain a stable system.
    */
   result.desc.typeMask = VMACCEL_COMPUTE_ACCELERATOR_MASK;
   result.desc.architecture = accelArch;
   result.desc.caps = VMACCEL_SURFACEMAP;

   /*
    * Format caps.
    */
   result.desc.formatCaps.formatCaps_len = 0;
   result.desc.formatCaps.formatCaps_val = NULL;

   /*
    * OpenCL work hierarchy:
    *
    * Work Group
    * +-> Work Items(x, y, z, ...)
    */
   result.desc.capacity.megaFlops = caps[accelArch].DEVICE_MAX_COMPUTE_UNITS *
                                    caps[accelArch].DEVICE_MAX_CLOCK_FREQUENCY;

   result.desc.capacity.megaOps = result.desc.capacity.megaFlops;

   /*
    * Cache capabilities.
    */
   result.desc.capacity.llcSizeKB =
      caps[accelArch].DEVICE_GLOBAL_MEM_CACHE_SIZE / 1000;
   result.desc.capacity.llcBandwidthMBSec =
      result.desc.capacity.llcSizeKB *
      (caps[accelArch].DEVICE_MAX_CLOCK_FREQUENCY / 1000);

   /*
    * Local memory capabilities.
    */
   result.desc.capacity.localMemSizeKB =
      caps[accelArch].DEVICE_LOCAL_MEM_SIZE / 1000;

   // TODO: Query clock/bus information for the MMU.
   result.desc.capacity.localMemBandwidthMBSec = 0;

   /*
    * Non-local memory capabilities.
    */
   result.desc.capacity.nonLocalMemSizeKB =
      caps[accelArch].DEVICE_GLOBAL_MEM_SIZE / 1000;

   // TODO: Query the clock/bus information for PCI-E or NUMA.
   result.desc.capacity.nonLocalMemBandwidthMBSec = 0;

   /*
    * Accelerator Interconnect capabilities.
    *
    * TODO: Query the clock/bus information for PCI-E or NUMA.
    */
   result.desc.capacity.interconnectBandwidthMBSec = 0;

   contexts = calloc(VMCL_MAX_CONTEXTS, sizeof(VMWOpenCLContext));
   contextIds = IdentifierDB_Alloc(VMCL_MAX_CONTEXTS);

   surfaces = calloc(VMCL_MAX_SURFACES, sizeof(VMWOpenCLSurface));
   surfaceIds = IdentifierDB_Alloc(VMCL_MAX_SURFACES);

   queues = calloc(VMCL_MAX_QUEUES, sizeof(VMWOpenCLQueue));
   queueIds = IdentifierDB_Alloc(VMCL_MAX_QUEUES);

   samplers = calloc(VMCL_MAX_SAMPLERS, sizeof(VMWOpenCLSampler));
   samplerIds = IdentifierDB_Alloc(VMCL_MAX_SAMPLERS);

   kernels = calloc(VMCL_MAX_KERNELS, sizeof(VMWOpenCLKernel));
   kernelIds = IdentifierDB_Alloc(VMCL_MAX_KERNELS);

   /*
    * Final check for allocation failure.
    */
   if ((contexts == NULL) || (contextIds == NULL) || (surfaces == NULL) ||
       (surfaceIds == NULL) || (queues == NULL) || (queueIds == NULL) ||
       (samplers == NULL) || (samplerIds == NULL) ||
       (kernels == NULL) || (kernelIds == NULL)) {
      VMACCEL_WARNING("Unable to allocate object database...\n");
      result.status = VMACCEL_FAIL;
      vmwopencl_poweroff();
   } else {
      result.status = VMACCEL_SUCCESS;
   }

   result.desc.maxContexts = VMCL_MAX_CONTEXTS;
   result.desc.maxQueues = VMCL_MAX_QUEUES;
   result.desc.maxSurfaces = VMCL_MAX_SURFACES;
   result.desc.maxMappings = VMCL_MAX_SURFACES;

#if ENABLE_DATA_STREAMING
   VMAccelStreamCallbacks cb;
   cb.clSurfacemap_1 = vmwopencl_surfacemap_1;
   cb.clSurfaceunmap_1 = vmwopencl_surfaceunmap_1;
   vmaccel_stream_server(VMACCEL_STREAM_TYPE_VMCL_UPLOAD,
                         VMACCEL_VMCL_BASE_PORT, &cb);
#endif

   return (&result);
}

VMAccelStatus *vmwopencl_poweroff() {
   static VMAccelStatus result;
   int i;

   memset(&result, 0, sizeof(result));

   for (i = 0; i < VMACCEL_SELECT_MAX; i++) {
      free(caps[i].DEVICE_MAX_WORK_ITEM_SIZES);
   }

   memset(&caps, 0, sizeof(caps));

   IdentifierDB_Free(contextIds);
   free(contexts);

   IdentifierDB_Free(surfaceIds);
   free(surfaces);

   IdentifierDB_Free(queueIds);
   free(queues);

   IdentifierDB_Free(samplerIds);
   free(samplers);

   IdentifierDB_Free(kernelIds);
   free(kernels);

   return (&result);
}

VMCLContextAllocateStatus *
vmwopencl_contextalloc_1(VMCLContextAllocateDesc *argp) {
   unsigned int cid = argp->clientId;
   static VMCLContextAllocateStatus result;
   size_t sizeRet;
   cl_uint numPlatforms;
   cl_platform_id platforms[16];
   cl_context context = NULL;
   cl_device_id deviceIds[VMCL_MAX_SUBDEVICES] = {
      0,
   };
   cl_int errNum;
   char platformName[128] = {'\0'};
   char platformVersion[128] = {'\0'};
   int majorVersion;
   int minorVersion;
   VMWOpenCLCaps *ctxCaps = NULL;
   int i = 0, j = 0, k = 0;

   memset(&result, 0, sizeof(result));

   if (IdentifierDB_ActiveId(contextIds, cid)) {
      VMACCEL_WARNING("%s: ERROR: Context ID %d already active...\n",
                      __FUNCTION__, cid);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   // First, select an OpenCL platform to run on.  For this example, we
   // simply choose the first available platform.  Normally, you would
   // query for all available platforms and select the most appropriate one.
   errNum = clGetPlatformIDs(sizeof(platforms) / sizeof(platforms[0]),
                             &platforms[0], &numPlatforms);

   if (errNum != CL_SUCCESS || numPlatforms <= 0) {
      VMACCEL_WARNING("Failed to find any OpenCL platforms.\n");
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   for (i = 0; i < numPlatforms; i++) {
      // Next, create an OpenCL context on the platform.  Attempt to
      // create a GPU-based context, and if that fails, try to create
      // a CPU-based context.
      errNum =
         clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION,
                           sizeof(platformVersion), platformVersion, &sizeRet);

      if (errNum != CL_SUCCESS) {
         VMACCEL_WARNING("Failed to query platform version\n");
         continue;
      }

      if (strstr(platformVersion, "OpenCL 1.2") != NULL) {
         majorVersion = 1;
         minorVersion = 2;
      } else if (strstr(platformVersion, "OpenCL 2.0") != NULL) {
         majorVersion = 2;
         minorVersion = 0;
      } else if (strstr(platformVersion, "OpenCL 2.") != NULL) {
         majorVersion = 2;
         minorVersion = 1;
      } else if (strstr(platformVersion, "OpenCL 3.") != NULL) {
         majorVersion = 3;
         minorVersion = 0;
      } else {
         majorVersion = 3;
         minorVersion = 0;
         VMACCEL_WARNING("Unknown version detected: %s\n", platformVersion);
      }

      // SPIR-V 1.0 requires OpenCL 2.1
      if ((argp->requiredCaps & VMCL_SPIRV_1_0_CAP) &&
          ((majorVersion < 2) || (minorVersion < 1))) {
         VMACCEL_WARNING("SPIRV 1.0 requires OpenCL 2.1+\n");
         continue;
      } else {
         result.caps |= VMCL_SPIRV_1_0_CAP;
      }

      // SPIR-V 1.1/1.2 requires OpenCL 2.2
      if (((argp->requiredCaps & VMCL_SPIRV_1_1_CAP) ||
           (argp->requiredCaps & VMCL_SPIRV_1_2_CAP)) &&
          ((majorVersion < 2) || (minorVersion < 2))) {
         VMACCEL_WARNING("SPIRV 1.1/1.2 requires OpenCL 2.2+\n");
         continue;
      } else {
         result.caps |= VMCL_SPIRV_1_1_CAP | VMCL_SPIRV_1_2_CAP;
      }

      errNum = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME,
                                 sizeof(platformName), platformName, &sizeRet);

      if (errNum != CL_SUCCESS) {
         VMACCEL_WARNING("Failed to query platform name\n");
         continue;
      }

      VMACCEL_LOG("Using Platform: %s\n", platformName);
      VMACCEL_LOG("  Version: %s\n", platformVersion);

      for (j = 0; j < VMACCEL_SELECT_MAX; j++) {
         char deviceName[128];
         unsigned int numSubDevices = MAX(1, argp->numSubDevices);

         if ((argp->selectionMask & (1 << j)) == 0) {
            continue;
         }

         errNum = clGetDeviceIDs(platforms[i], clDeviceTypes[j], numSubDevices,
                                 &deviceIds[0], NULL);

         if (errNum != CL_SUCCESS) {
            continue;
         }

         for (k = 0; k < numSubDevices; k++) {
            cl_uint id;

            errNum = clGetDeviceInfo(deviceIds[k], CL_DEVICE_NAME,
                                     sizeof(deviceName), deviceName, &sizeRet);

            if (errNum != CL_SUCCESS) {
               VMACCEL_WARNING("Failed to query device name\n");
               continue;
            }

            VMACCEL_LOG("Device[%d]: Allocated %s\n", k, deviceName);

#define CL_DEVICE_PCI_BUS_ID_NV 0x4008
            errNum = clGetDeviceInfo(deviceIds[k], CL_DEVICE_PCI_BUS_ID_NV,
                                     sizeof(cl_uint), &id, NULL);

            if (errNum == CL_SUCCESS) {
               VMACCEL_LOG("Device[%d]: id=%02x\n", k, id);
            }
         }

         context = clCreateContext(0, numSubDevices, &deviceIds[0], NULL, NULL,
                                   &errNum);

         if (errNum != CL_SUCCESS) {
            VMACCEL_WARNING("Unable to create context, errNum=%d\n", errNum);
         } else if (context != NULL) {
            ctxCaps = &caps[j];
            break;
         }
      }

      if (context != NULL) {
         break;
      }
   }

   if ((context == NULL) || (errNum != CL_SUCCESS)) {
      VMACCEL_WARNING("Failed to create an OpenCL context.\n");
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   if (IdentifierDB_AcquireId(contextIds, cid)) {
      contexts[cid].context = context;
      contexts[cid].platformId = platforms[i];
      memcpy(contexts[cid].deviceIds, deviceIds, sizeof(deviceIds));
      contexts[cid].majorVersion = majorVersion;
      contexts[cid].minorVersion = minorVersion;
      contexts[cid].caps = ctxCaps;
   } else {
      assert(0);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
   }

   return (&result);
}

VMAccelStatus *vmwopencl_contextdestroy_1(VMCLContextId *argp) {
   static VMAccelStatus result;
   unsigned int cid = *((unsigned int *)argp);

   memset(&result, 0, sizeof(result));

   if (IdentifierDB_ActiveId(contextIds, cid) && contexts[cid].context) {
      clReleaseContext(contexts[cid].context);
      contexts[cid].context = NULL;
   } else {
      VMACCEL_WARNING("%s: Destroying id %d, already destroyed\n", __FUNCTION__,
                      cid);
   }

   IdentifierDB_ReleaseId(contextIds, cid);

   if (IdentifierDB_Count(contextIds) == 0) {
      if (IdentifierDB_Count(surfaceIds) != 0) {
         VMACCEL_WARNING(
            "%s: Semantic error: Missing destructor call, %d surfaces "
            "still active...\n",
            __FUNCTION__, IdentifierDB_Count(surfaceIds));
      }

      if (IdentifierDB_Count(queueIds) != 0) {
         VMACCEL_WARNING(
            "%s: Semantic error: Missing destructor call, %d queues "
            "still active...\n",
            __FUNCTION__, IdentifierDB_Count(queueIds));
      }

      if (IdentifierDB_Count(samplerIds) != 0) {
         VMACCEL_WARNING(
            "%s: Semantic error: Missing destructor call, %d samplers "
            "still active...\n",
            __FUNCTION__, IdentifierDB_Count(samplerIds));
      }

      if (IdentifierDB_Count(kernelIds) != 0) {
         VMACCEL_WARNING(
            "%s: Semantic error: Missing destructor call, %d kernels "
            "still active...\n",
            __FUNCTION__, IdentifierDB_Count(kernelIds));
      }
   }

   return (&result);
}

VMAccelSurfaceAllocateStatus *
vmwopencl_surfacealloc_1(VMCLSurfaceAllocateDesc *argp) {
   static VMAccelSurfaceAllocateStatus result;
   unsigned int cid = (unsigned int)argp->client.cid;
   unsigned int sid = (unsigned int)argp->client.accel.id;
   cl_context context = contexts[cid].context;
   cl_mem memObject[VMACCEL_MAX_SURFACE_INSTANCE] = {
      NULL,
   };
   void *svmPtr[VMACCEL_MAX_SURFACE_INSTANCE] = {
      NULL,
   };
   unsigned int clMemFlags = 0;

   memset(&result, 0, sizeof(result));

   if (IdentifierDB_ActiveId(surfaceIds, sid)) {
      VMACCEL_WARNING("%s: ERROR: Surface ID %d already active...\n",
                      __FUNCTION__, sid);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   if (argp->desc.usage == VMACCEL_SURFACE_USAGE_READONLY) {
      clMemFlags |= CL_MEM_READ_ONLY;
   } else if (argp->desc.usage == VMACCEL_SURFACE_USAGE_WRITEONLY) {
      clMemFlags |= CL_MEM_WRITE_ONLY;
   } else if (argp->desc.usage == VMACCEL_SURFACE_USAGE_READWRITE) {
      clMemFlags |= CL_MEM_READ_WRITE;
   }

#if CL_VERSION_2_0
   if ((argp->desc.type == VMACCEL_SURFACE_BUFFER) &&
       (argp->desc.pool == VMACCEL_SURFACE_POOL_SYSTEM_MEMORY) &&
       (contexts[cid].majorVersion >= 2) && (contexts[cid].minorVersion >= 0)) {
      for (int i = 0; i < VMACCEL_MAX_SURFACE_INSTANCE; i++) {
         svmPtr[i] = clSVMAlloc(context, clMemFlags, argp->desc.width, 0);
         if (svmPtr[i] == NULL) {
            result.status = VMACCEL_FAIL;
            for (i; i > 0; i--) {
               clSVMFree(context, svmPtr[i - 1]);
            }
            return (&result);
         }
      }

   } else
#endif
      if (argp->desc.type == VMACCEL_SURFACE_BUFFER) {
      assert(argp->desc.format == VMACCEL_FORMAT_R8_TYPELESS);

      for (int i = 0; i < VMACCEL_MAX_SURFACE_INSTANCE; i++) {
         memObject[i] =
            clCreateBuffer(context, clMemFlags, argp->desc.width, NULL, NULL);
         if (memObject[i] == NULL) {
            result.status = VMACCEL_FAIL;
            for (i; i > 0; i--) {
               clReleaseMemObject(memObject[i - 1]);
            }
            return (&result);
         }
      }
   } else {
      assert(argp->desc.usage == VMACCEL_SURFACE_USAGE_READWRITE);

      /* Image from buffer? */

      result.status = VMACCEL_FAIL;
      return (&result);
   }

   if (memObject == NULL) {
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   /*
    * Initialize mutexes to provide data coherency and consistency
    * contracts.
    */
   pthread_mutexattr_t attr;
   pthread_mutexattr_init(&attr);
   pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

   pthread_mutex_init(&surfaces[sid].mutex, &attr);
   if (IdentifierDB_AcquireId(surfaceIds, sid)) {
      pthread_mutex_lock(&surfaces[sid].mutex);
      surfaces[sid].cid = cid;
      surfaces[sid].desc = argp->desc;
      for (int i = 0; i < VMACCEL_MAX_SURFACE_INSTANCE; i++) {
         surfaces[sid].inst[i].mem = memObject[i];
         surfaces[sid].inst[i].svm_ptr = svmPtr[i];
         surfaces[sid].inst[i].mapping.refCount = 0;
         pthread_mutex_init(&surfaces[sid].inst[i].mutex, &attr);
      }
      pthread_mutex_unlock(&surfaces[sid].mutex);
   } else {
      assert(0);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
   }

   return (&result);
}

VMAccelStatus *vmwopencl_surfacedestroy_1(VMCLSurfaceId *argp) {
   static VMAccelStatus result;
   unsigned int sid = (unsigned int)argp->accel.id;

   pthread_mutex_lock(&surfaces[sid].mutex);

   memset(&result, 0, sizeof(result));

   assert(IdentifierDB_ActiveId(surfaceIds, sid));

   for (int i = 0; i < VMACCEL_MAX_SURFACE_INSTANCE; i++) {
      pthread_mutex_lock(&surfaces[sid].inst[i].mutex);
#if CL_VERSION_2_0
      if (surfaces[sid].inst[i].svm_ptr) {
         unsigned int cid = (unsigned int)surfaces[sid].cid;
         cl_context context = contexts[cid].context;
         clSVMFree(context, surfaces[sid].inst[i].svm_ptr);
      } else
#endif
      {
         clReleaseMemObject(surfaces[sid].inst[i].mem);
      }
      surfaces[sid].inst[i].mem = NULL;
      pthread_mutex_unlock(&surfaces[sid].inst[i].mutex);
      pthread_mutex_destroy(&surfaces[sid].inst[i].mutex);
   }

   memset(&surfaces[sid], 0, sizeof(surfaces[0]));

   pthread_mutex_unlock(&surfaces[sid].mutex);
   pthread_mutex_destroy(&surfaces[sid].mutex);

   IdentifierDB_ReleaseId(surfaceIds, sid);
   result.status = VMACCEL_SUCCESS;

   return (&result);
}

VMAccelQueueStatus *vmwopencl_queuealloc_1(VMCLQueueAllocateDesc *argp) {
   static VMAccelQueueStatus result;
   unsigned int cid = (unsigned int)argp->client.cid;
   unsigned int qid = (unsigned int)argp->client.id;
   unsigned int subDevice = (unsigned int)argp->subDevice;
   cl_context context = contexts[cid].context;
   cl_int errNum;
   cl_device_id *devices;
   cl_command_queue commandQueue = NULL;
   size_t deviceBufferSize = -1;

   memset(&result, 0, sizeof(result));

   if (IdentifierDB_ActiveId(queueIds, qid)) {
      VMACCEL_WARNING("%s: ERROR: Queue ID %d already active...\n",
                      __FUNCTION__, qid);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   // First get the size of the devices buffer
   errNum =
      clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);

   if (errNum != CL_SUCCESS) {
      VMACCEL_WARNING(
         "Failed call to clGetContextInfo(...,GL_CONTEXT_DEVICES,...)");
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   if (deviceBufferSize <= 0) {
      VMACCEL_WARNING("No devices available.");
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   // Allocate memory for the devices buffer
   devices =
      calloc(deviceBufferSize / sizeof(cl_device_id), sizeof(cl_device_id));

   errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize,
                             devices, NULL);

   if (errNum != CL_SUCCESS ||
       subDevice >= (deviceBufferSize / sizeof(cl_device_id))) {
      free(devices);
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   // In this example, we just choose the first available device.  In a
   // real program, you would likely use all available devices or choose
   // the highest performance device based on OpenCL device queries
   commandQueue =
      clCreateCommandQueueWithProperties(context, devices[subDevice], 0, NULL);
   VMACCEL_WARNING("Device[%d]: Allocating queue %d on device id 0x%x\n",
                   subDevice, qid, devices[subDevice]);
   if (commandQueue == NULL) {
      VMACCEL_WARNING("Failed to create commandQueue for device 0");
      free(devices);
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   free(devices);

   if (IdentifierDB_AcquireId(queueIds, qid)) {
      queues[qid].desc = argp->desc;
      queues[qid].queue = commandQueue;
   } else {
      clReleaseCommandQueue(commandQueue);
      assert(0);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
   }

   return (&result);
}

VMAccelStatus *vmwopencl_queuedestroy_1(VMCLQueueId *argp) {
   static VMAccelStatus result;
   unsigned int qid = (unsigned int)argp->id;

   memset(&result, 0, sizeof(result));

   assert(IdentifierDB_ActiveId(queueIds, qid));

   clReleaseCommandQueue(queues[qid].queue);
   memset(&queues[qid], 0, sizeof(queues[0]));

   IdentifierDB_ReleaseId(queueIds, qid);

   return (&result);
}

VMAccelStatus *vmwopencl_queueflush_1(VMCLQueueId *argp) {
   static VMAccelStatus result;
   unsigned int qid = (unsigned int)argp->id;
   cl_int errNum;

#if DEBUG_SURFACE_CONSISTENCY
   VMACCEL_LOG("%s: flushing qid=%d\n", __FUNCTION__, qid);
#endif

   memset(&result, 0, sizeof(result));

   errNum = clFlush(queues[qid].queue);

   if (errNum != CL_SUCCESS) {
      result.status = VMACCEL_FAIL;
   }

   return (&result);
}

VMAccelStatus *vmwopencl_imageupload_1(VMCLImageUploadOp *argp) {
   static VMAccelStatus result;
   unsigned int cid = (unsigned int)argp->queue.cid;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->img.accel.id;
   unsigned int gen = (unsigned int)argp->img.accel.generation;
   unsigned int inst = (unsigned int)argp->img.accel.instance;
   cl_command_queue queue = queues[qid].queue;
   cl_int errNum;

#if DEBUG_SURFACE_CONSISTENCY
   VMACCEL_LOG("%s: sid=%d, gen=%d, inst=%d\n", __FUNCTION__, sid, gen, inst);
   VMACCEL_LOG("%s: cid=%d, qid=%d\n", __FUNCTION__, cid, qid);

   for (int i = 0; i < argp->op.imgRegion.size.x / 4; i++) {
      VMACCEL_LOG("%s: uint32[%d] = 0x%x\n", __FUNCTION__, i,
                  ((unsigned int *)argp->op.ptr.ptr_val)[i]);
   }
#endif

   pthread_mutex_lock(&surfaces[sid].mutex);
   pthread_mutex_lock(&surfaces[sid].inst[inst].mutex);

   if (surfaces[sid].inst[inst].generation > gen) {
      result.status = VMACCEL_SEMANTIC_ERROR;

      pthread_mutex_unlock(&surfaces[sid].inst[inst].mutex);
      pthread_mutex_unlock(&surfaces[sid].mutex);

      return (&result);
   }

   memset(&result, 0, sizeof(result));

   assert(cid == argp->img.cid);

   if (surfaces[sid].desc.type == VMACCEL_SURFACE_BUFFER &&
       surfaces[sid].inst[inst].svm_ptr == NULL) {
      errNum = clEnqueueWriteBuffer(queue, surfaces[sid].inst[inst].mem,
                                    CL_TRUE, argp->op.imgRegion.coord.x,
                                    argp->op.imgRegion.size.x,
                                    argp->op.ptr.ptr_val, 0, NULL, NULL);

      if (errNum != CL_SUCCESS) {
         VMACCEL_WARNING("%s: Failed to enqueuew update\n", __FUNCTION__);
         result.status = VMACCEL_FAIL;
      } else {
         surfaces[sid].inst[inst].generation = gen;
      }
   } else {
      assert(0);

      result.status = VMACCEL_FAIL;
   }

   pthread_mutex_unlock(&surfaces[sid].inst[inst].mutex);
   pthread_mutex_unlock(&surfaces[sid].mutex);

   return (&result);
}

VMAccelDownloadStatus *vmwopencl_imagedownload_1(VMCLImageDownloadOp *argp) {
   static VMAccelDownloadStatus result;
   unsigned int cid = (unsigned int)argp->queue.cid;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->img.accel.id;
   unsigned int gen = (unsigned int)argp->img.accel.generation;
   unsigned int inst = (unsigned int)argp->img.accel.instance;
   cl_command_queue queue = queues[qid].queue;
   void *ptr;
   cl_int errNum;

   pthread_mutex_lock(&surfaces[sid].mutex);
   pthread_mutex_lock(&surfaces[sid].inst[inst].mutex);

   if (surfaces[sid].inst[inst].generation != gen) {
      if (surfaces[sid].inst[inst].generation > gen) {
         VMACCEL_WARNING("Out-of-order update detected, client/server"
                         " out of sync...\n");
         result.status = VMACCEL_SEMANTIC_ERROR;
      } else {
         result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      }

      pthread_mutex_unlock(&surfaces[sid].inst[inst].mutex);
      pthread_mutex_unlock(&surfaces[sid].mutex);

      return (&result);
   }

   memset(&result, 0, sizeof(result));

   assert(cid == argp->img.cid);

   if (surfaces[sid].desc.type == VMACCEL_SURFACE_BUFFER &&
       surfaces[sid].inst[inst].svm_ptr == NULL) {
      unsigned int blocking = 0;

      if (argp->op.ptr.ptr_val == NULL) {
         ptr = calloc(1, argp->op.imgRegion.size.x);
      } else {
         ptr = argp->op.ptr.ptr_val;
      }

      if (argp->mode == VMACCEL_SURFACE_READ_SYNCHRONOUS) {
         VMACCEL_LOG("%s: Enqueing blocking read sid=%d -> %p\n", __FUNCTION__,
                     sid, ptr);
         blocking = TRUE;
      }

      errNum =
         clEnqueueReadBuffer(queue, surfaces[sid].inst[inst].mem, blocking,
                             argp->op.imgRegion.coord.x,
                             argp->op.imgRegion.size.x, ptr, 0, NULL, NULL);

      if (errNum != CL_SUCCESS) {
         result.status = VMACCEL_FAIL;
      } else {
         result.ptr.ptr_len = argp->op.imgRegion.size.x;
         result.ptr.ptr_val = ptr;
      }
   } else {
      assert(0);

      result.status = VMACCEL_FAIL;
   }

   pthread_mutex_unlock(&surfaces[sid].mutex);

   return (&result);
}

VMAccelSurfaceMapStatus *vmwopencl_surfacemap_1(VMCLSurfaceMapOp *argp) {
   static VMAccelSurfaceMapStatus result;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->op.surf.id;
   unsigned int gen = (unsigned int)argp->op.surf.generation;
   unsigned int inst = (unsigned int)argp->op.surf.instance;
   unsigned int blocking = TRUE;
   cl_command_queue queue = queues[qid].queue;
   void *ptr;
   cl_map_flags flags = 0;
   cl_int errNum = CL_SUCCESS;

   memset(&result, 0, sizeof(result));

   pthread_mutex_lock(&surfaces[sid].mutex);
   pthread_mutex_lock(&surfaces[sid].inst[inst].mutex);

   if (surfaces[sid].inst[inst].generation > gen) {
      result.status = VMACCEL_SEMANTIC_ERROR;

      pthread_mutex_unlock(&surfaces[sid].inst[inst].mutex);
      pthread_mutex_unlock(&surfaces[sid].mutex);

      return (&result);
   }

   if (argp->op.mapFlags & VMACCEL_MAP_READ_FLAG) {
      flags |= CL_MAP_READ;
   }

   if (argp->op.mapFlags & VMACCEL_MAP_WRITE_FLAG) {
      flags |= CL_MAP_WRITE;
   }

   if (argp->op.mapFlags & VMACCEL_MAP_WRITE_DISCARD_FLAG) {
      flags |= CL_MAP_WRITE_INVALIDATE_REGION;
   }

   if (argp->op.mapFlags & VMACCEL_MAP_ASYNC_FLAG) {
      blocking = FALSE;
   }

#if DEBUG_SURFACE_CONSISTENCY
   VMACCEL_LOG("%s: sid=%d, gen=%d, inst=%d\n", __FUNCTION__, sid, gen, inst);
#endif

   if (surfaces[sid].desc.type == VMACCEL_SURFACE_BUFFER) {
      if (++surfaces[sid].inst[inst].mapping.refCount == 1) {
         if (surfaces[sid].inst[inst].svm_ptr) {
            if (blocking) {
               errNum = clFinish(queues[qid].queue);
            }
            if (errNum == CL_SUCCESS) {
               ptr = surfaces[sid].inst[inst].svm_ptr;
            }
         } else {
            ptr = clEnqueueMapBuffer(queue, surfaces[sid].inst[inst].mem,
                                     blocking, flags, argp->op.coord.x,
                                     argp->op.size.x, 0, NULL, NULL, &errNum);
         }
      } else {
         errNum = CL_SUCCESS;
         ptr = surfaces[sid].inst[inst].mapping.ptr;
      }

#if DEBUG_SURFACE_CONSISTENCY
      for (int i = 0; i < argp->op.size.x / 4; i++) {
         VMACCEL_LOG("%s: uint32[%d] = 0x%x\n", __FUNCTION__, i,
                     ((unsigned int *)ptr)[i]);
      }
#endif

      if (errNum != CL_SUCCESS) {
         result.status = VMACCEL_FAIL;
      } else {
         surfaces[sid].inst[inst].mapping.ptr = ptr;

         result.ptr.ptr_val = ptr;
         result.ptr.ptr_len = argp->op.size.x;
      }
   } else {
      assert(0);
      result.status = VMACCEL_FAIL;
   }


   pthread_mutex_unlock(&surfaces[sid].inst[inst].mutex);
   pthread_mutex_unlock(&surfaces[sid].mutex);

   return (&result);
}

VMAccelStatus *vmwopencl_surfaceunmap_1(VMCLSurfaceUnmapOp *argp) {
   static VMAccelStatus result;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->op.surf.id;
   unsigned int gen = (unsigned int)argp->op.surf.generation;
   unsigned int inst = (unsigned int)argp->op.surf.instance;
   cl_command_queue queue = queues[qid].queue;
   void *ptr;
   cl_int errNum = CL_SUCCESS;

   memset(&result, 0, sizeof(result));

   pthread_mutex_lock(&surfaces[sid].mutex);

   /*
    * memcpy the data from the incoming mapping object.
    */
   ptr = surfaces[sid].inst[inst].mapping.ptr;

#if DEBUG_SURFACE_CONSISTENCY
   VMACCEL_LOG("%s: sid=%d, gen=%d, inst=%d, *ptr=%x, %x, %x, %x\n",
               __FUNCTION__, sid, gen, inst,
               ((unsigned int *)argp->op.ptr.ptr_val)[0],
               ((unsigned int *)argp->op.ptr.ptr_val)[1],
               ((unsigned int *)argp->op.ptr.ptr_val)[2],
               ((unsigned int *)argp->op.ptr.ptr_val)[3]);
#endif

   memcpy(ptr, argp->op.ptr.ptr_val, argp->op.ptr.ptr_len);

   /*
    * We are done with the contents, free the pointer.
    */
   if ((argp->op.mapFlags & VMACCEL_MAP_NO_FREE_PTR_FLAG) == 0) {
      free(argp->op.ptr.ptr_val);
      argp->op.ptr.ptr_val = NULL;
      argp->op.ptr.ptr_len = 0;
   }

   if (--surfaces[sid].inst[inst].mapping.refCount == 0) {
      if (surfaces[sid].desc.type == VMACCEL_SURFACE_BUFFER) {
         if (surfaces[sid].inst[inst].svm_ptr == NULL) {
            errNum = clEnqueueUnmapMemObject(
               queue, surfaces[sid].inst[inst].mem, ptr, 0, NULL, NULL);
         }
      }
      surfaces[sid].inst[inst].mapping.ptr = NULL;
   }

   if (errNum != CL_SUCCESS) {
      VMACCEL_WARNING("Unable to unmap sid=%d instance=%d generation=%d\n", sid,
                      inst, gen);
      result.status = VMACCEL_FAIL;
   } else {
      surfaces[sid].inst[inst].generation = gen;
   }

   pthread_mutex_unlock(&surfaces[sid].inst[inst].mutex);
   pthread_mutex_unlock(&surfaces[sid].mutex);

   return (&result);
}

VMAccelStatus *vmwopencl_surfacecopy_1(VMCLSurfaceCopyOp *argp) {
   static VMAccelStatus result;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int dstSid = (unsigned int)argp->dst.accel.id;
   unsigned int dstGen = (unsigned int)argp->dst.accel.generation;
   unsigned int dstInst = 0; //(unsigned int)argp->dst.accel.instance;
   unsigned int srcSid = (unsigned int)argp->src.accel.id;
   unsigned int srcGen = (unsigned int)argp->src.accel.generation;
   unsigned int srcInst = 0; //(unsigned int)argp->src.accel.instance;
   cl_command_queue queue = queues[qid].queue;
   cl_int errNum;

   memset(&result, 0, sizeof(result));

   /*
    * Cross context surface copy is not currently supported.
    */
   if (argp->dst.cid != argp->src.cid) {
      assert(0);
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   pthread_mutex_lock(&surfaces[srcSid].mutex);
   pthread_mutex_lock(&surfaces[dstSid].mutex);

   pthread_mutex_lock(&surfaces[srcSid].inst[srcInst].mutex);
   pthread_mutex_lock(&surfaces[dstSid].inst[dstInst].mutex);

   if (surfaces[srcSid].inst[srcInst].generation < srcGen) {
      VMACCEL_LOG("%s: generation %d < %d\n", __FUNCTION__,
                  surfaces[srcSid].inst[srcInst].generation, srcGen);

      result.status = VMACCEL_RESOURCE_UNAVAILABLE;

      pthread_mutex_unlock(&surfaces[dstSid].inst[dstInst].mutex);
      pthread_mutex_unlock(&surfaces[srcSid].inst[srcInst].mutex);

      pthread_mutex_unlock(&surfaces[dstSid].mutex);
      pthread_mutex_unlock(&surfaces[srcSid].mutex);

      return (&result);
   }

   if ((surfaces[srcSid].inst[srcInst].generation > srcGen) ||
       (surfaces[dstSid].inst[dstInst].generation > dstGen)) {
      VMACCEL_LOG("%s: semantic error backend.srcGen=%d srcGen=%d"
                  " backend.dstGen=%d dstGen=%d\n",
                  __FUNCTION__, surfaces[srcSid].inst[srcInst].generation,
                  srcGen, surfaces[dstSid].inst[dstInst].generation, dstGen);

      result.status = VMACCEL_SEMANTIC_ERROR;

      pthread_mutex_unlock(&surfaces[dstSid].inst[dstInst].mutex);
      pthread_mutex_unlock(&surfaces[srcSid].inst[srcInst].mutex);

      pthread_mutex_unlock(&surfaces[dstSid].mutex);
      pthread_mutex_unlock(&surfaces[srcSid].mutex);

      return (&result);
   }

#if DEBUG_SURFACE_CONSISTENCY
   VMACCEL_LOG("%s: srcSid=%d, srcGen=%d, dstSid=%d, dstGen=%d\n", __FUNCTION__,
               srcSid, srcGen, dstSid, dstGen);
#endif

   if (surfaces[srcSid].inst[srcInst].svm_ptr ||
       surfaces[dstSid].inst[dstInst].svm_ptr) {
      VMACCEL_WARNING("%s: Copy with SVM unsupported.\n", __FUNCTION__);
      result.status = VMACCEL_FAIL;
   } else if ((surfaces[dstSid].desc.type == VMACCEL_SURFACE_BUFFER) &&
              (surfaces[srcSid].desc.type == VMACCEL_SURFACE_BUFFER)) {
      errNum = clEnqueueCopyBuffer(
         queue, surfaces[srcSid].inst[srcInst].mem,
         surfaces[dstSid].inst[dstInst].mem, argp->op.srcRegion.coord.x,
         argp->op.dstRegion.coord.x, argp->op.dstRegion.size.x, 0, NULL, NULL);

      if (errNum != CL_SUCCESS) {
         result.status = VMACCEL_FAIL;
      }
   } else {
      assert(0);
      result.status = VMACCEL_FAIL;
   }

   pthread_mutex_unlock(&surfaces[dstSid].inst[dstInst].mutex);
   pthread_mutex_unlock(&surfaces[srcSid].inst[srcInst].mutex);

   pthread_mutex_unlock(&surfaces[dstSid].mutex);
   pthread_mutex_unlock(&surfaces[srcSid].mutex);

   return (&result);
}

VMAccelStatus *vmwopencl_imagefill_1(VMCLImageFillOp *argp) {
   static VMAccelStatus result;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->img.accel.id;
   unsigned int gen = (unsigned int)argp->img.accel.generation;
   unsigned int inst = 0; //(unsigned int)argp->img.accel.instance;
   cl_command_queue queue = queues[qid].queue;
   cl_int errNum;

   memset(&result, 0, sizeof(result));

   /*
    * Cross context surface copy is not currently supported.
    */
   pthread_mutex_lock(&surfaces[sid].mutex);

   pthread_mutex_lock(&surfaces[sid].inst[inst].mutex);

   if (surfaces[sid].inst[inst].generation < gen) {
      VMACCEL_LOG("%s: generation %d < %d\n", __FUNCTION__,
                  surfaces[sid].inst[inst].generation, gen);

      result.status = VMACCEL_RESOURCE_UNAVAILABLE;

      pthread_mutex_unlock(&surfaces[sid].inst[inst].mutex);

      pthread_mutex_unlock(&surfaces[sid].mutex);

      return (&result);
   }

   if (surfaces[sid].inst[inst].generation > gen) {
      VMACCEL_LOG("%s: semantic error backend.gen=%d gen=%d", __FUNCTION__,
                  surfaces[sid].inst[inst].generation, gen);

      result.status = VMACCEL_SEMANTIC_ERROR;

      pthread_mutex_unlock(&surfaces[sid].inst[inst].mutex);

      pthread_mutex_unlock(&surfaces[sid].mutex);

      return (&result);
   }

#if DEBUG_SURFACE_CONSISTENCY
   VMACCEL_LOG("%s: sid=%d, gen=%d\n", __FUNCTION__, sid, gen);
#endif

   if (surfaces[sid].inst[inst].svm_ptr) {
      VMACCEL_WARNING("%s: Fill with SVM unsupported.\n", __FUNCTION__);
      result.status = VMACCEL_FAIL;
   } else if (surfaces[sid].desc.type == VMACCEL_SURFACE_BUFFER) {
      errNum = clEnqueueFillBuffer(
         queue, surfaces[sid].inst[inst].mem, (const void *)&argp->op.u,
         sizeof(argp->op.u), argp->op.dstRegion.coord.x,
         argp->op.dstRegion.size.x, 0, NULL, NULL);

      if (errNum != CL_SUCCESS) {
         VMACCEL_WARNING("%s: Fill failed errNum=%d\n", __FUNCTION__, errNum);
         result.status = VMACCEL_FAIL;
      } else {
         surfaces[sid].inst[inst].generation = gen;
      }
   } else {
      assert(0);
      result.status = VMACCEL_FAIL;
   }

   pthread_mutex_unlock(&surfaces[sid].inst[inst].mutex);

   pthread_mutex_unlock(&surfaces[sid].mutex);

   return (&result);
}

VMCLSamplerAllocateStatus *
vmwopencl_sampleralloc_1(VMCLSamplerAllocateDesc *argp) {
   static VMCLSamplerAllocateStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelStatus *vmwopencl_samplerdestroy_1(VMCLSamplerId *argp) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMCLKernelAllocateStatus *
vmwopencl_kernelalloc_1(VMCLKernelAllocateDesc *argp) {
   static VMCLKernelAllocateStatus result;
   unsigned int cid = (unsigned int)argp->client.cid;
   unsigned int kid = (unsigned int)argp->client.id;
   unsigned int subDevice = (unsigned int)argp->subDevice;
   cl_context context = contexts[cid].context;
   cl_device_id deviceId = contexts[cid].deviceIds[subDevice];
   cl_kernel kernel = 0;
   cl_int errNum;
   cl_program program;
   size_t sourceLength = argp->source.source_len;

   memset(&result, 0, sizeof(result));

   VMACCEL_LOG("Allocating kernel id=%d\n", kid);

   if (IdentifierDB_ActiveId(kernelIds, kid)) {
      VMACCEL_WARNING("%s: ERROR: Kernel ID %d already active...\n",
                      __FUNCTION__, kid);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   if ((argp->language == VMCL_OPENCL_C_1_0) ||
       (argp->language == VMCL_OPENCL_C_1_1) ||
       (argp->language == VMCL_OPENCL_C_1_2)
#if CL_VERSION_2_0
       || ((contexts[cid].majorVersion >= 2) &&
           (contexts[cid].minorVersion >= 2) &&
           (argp->language == VMCL_OPENCL_C_2_0))
#endif
   ) {
      VMACCEL_LOG("Creating OpenCL C program\n");

#if DEBUG_COMPUTE_OPERATION
      VMACCEL_LOG("%s\n", argp->source.source_val);
#endif

      program = clCreateProgramWithSource(
         context, 1, (const char **)&argp->source.source_val, &sourceLength,
         &errNum);
#if CL_VERSION_2_2
   } else if ((contexts[cid].majorVersion >= 2) &&
              (contexts[cid].minorVersion >= 2) &&
              (argp->language == VMCL_OPENCL_CPP_1_0)) {
      VMACCEL_LOG("Creating OpenCL CPP program\n");

      program = clCreateProgramWithSource(
         context, 1, (const char **)&argp->source.source_val, &sourceLength,
         &errNum);
   } else if ((contexts[cid].majorVersion >= 2) &&
              (contexts[cid].minorVersion >= 2) &&
              ((argp->language == VMCL_SPIRV_1_1) ||
               (argp->language == VMCL_SPIRV_1_2))) {
      VMACCEL_LOG("Creating SPIR-V 1.1/1.2 program\n");

      program = clCreateProgramWithIL(context, (void *)argp->source.source_val,
                                      argp->source.source_len, &errNum);
#endif
#if CL_VERSION_2_1
   } else if ((contexts[cid].majorVersion >= 2) &&
              (contexts[cid].minorVersion >= 1) &&
              (argp->language == VMCL_SPIRV_1_0)) {
      VMACCEL_LOG("Creating SPIR-V 1.0 program\n");

      FILE *fp;
      fp = fopen("/tmp/server.spirv", "wb");

      if (fp) {
         fwrite(argp->source.source_val, argp->source.source_len, 1, fp);

         fflush(fp);
         fclose(fp);
      }

      program = clCreateProgramWithIL(context, (void *)argp->source.source_val,
                                      argp->source.source_len, &errNum);
#endif
   } else {
      assert(0);
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   if ((program == NULL) || (errNum != CL_SUCCESS)) {
      VMACCEL_WARNING("Failed to create CL program\n");
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   VMACCEL_LOG("Building program %p\n", program);

   errNum = clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);

   if (errNum != CL_SUCCESS) {
      char *buildLog;

      VMACCEL_WARNING("Error in kernel:\n");

      buildLog = malloc(sizeof(char) * 16384);

      if (buildLog) {
         clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, 16384,
                               buildLog, NULL);
         VMACCEL_WARNING("%s\n", buildLog);
         free(buildLog);
      }

      clReleaseProgram(program);
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   // Create OpenCL kernel
   kernel = clCreateKernel(program, argp->kernelName.kernelName_val, NULL);
   if (kernel == NULL) {
      VMACCEL_WARNING("Failed to create kernel\n");
      clReleaseProgram(program);
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   if (IdentifierDB_AcquireId(kernelIds, kid)) {
      kernels[kid].program = program;
      kernels[kid].kernel = kernel;
   } else {
      assert(0);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
   }

   result.status = VMACCEL_SUCCESS;

   return (&result);
}

VMAccelStatus *vmwopencl_kerneldestroy_1(VMCLKernelId *argp) {
   static VMAccelStatus result;
   unsigned int kid = (unsigned int)argp->id;

   memset(&result, 0, sizeof(result));

   VMACCEL_LOG("Destroying kernel id=%d\n", kid);

   if (!IdentifierDB_ActiveId(kernelIds, kid)) {
      result.status = VMACCEL_SUCCESS;
      return &result;
   }

   clReleaseProgram(kernels[kid].program);
   clReleaseKernel(kernels[kid].kernel);
   memset(&kernels[kid], 0, sizeof(kernels[0]));

   IdentifierDB_ReleaseId(kernelIds, kid);

   result.status = VMACCEL_SUCCESS;

   return (&result);
}

VMAccelStatus *vmwopencl_dispatch_1(VMCLDispatchOp *argp) {
   static VMAccelStatus result;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int kid = (unsigned int)argp->kernel.id;
   cl_command_queue queue = queues[qid].queue;
   cl_kernel kernel = kernels[kid].kernel;
   cl_int errNum;
   size_t *globalWorkOffset = NULL;
   size_t *globalWorkSize = NULL;
   size_t *localWorkSize = NULL;
   int argIndex;

   memset(&result, 0, sizeof(result));

#if DEBUG_COMPUTE_OPERATION
   VMACCEL_LOG("%s: qid=%d, kid=%d, kernel=%p\n", __FUNCTION__, qid, kid,
               kernel);
#endif

   for (argIndex = 0; argIndex < argp->args.args_len; argIndex++) {
      if (argp->args.args_val[argIndex].type == VMCL_ARG_SURFACE) {
         unsigned int sid = (unsigned int)argp->args.args_val[argIndex].surf.id;
         unsigned int gen =
            (unsigned int)argp->args.args_val[argIndex].surf.generation;
         unsigned int inst =
            (unsigned int)argp->args.args_val[argIndex].surf.instance;

         pthread_mutex_lock(&surfaces[sid].mutex);
         pthread_mutex_lock(&surfaces[sid].inst[inst].mutex);

#if DEBUG_COMPUTE_OPERATION
         VMACCEL_LOG("%s: arg[%d]: index=%d, sid=%d, gen=%d, inst=%d,"
                     " mem=%p\n",
                     __FUNCTION__, argIndex,
                     argp->args.args_val[argIndex].index, sid, gen, inst, mem);
#endif

         if (surfaces[sid].inst[inst].generation != gen) {
            if (surfaces[sid].inst[inst].generation > gen) {
               VMACCEL_WARNING("Out-of-order update detected, client/server"
                               " out of sync...\n");
               result.status = VMACCEL_SEMANTIC_ERROR;
            } else {
#if DEBUG_SURFACE_CONSISTENCY
               VMACCEL_WARNING(
                  "%s: arg[%d]: surf[%d].gen=%d, expected gen=%d\n",
                  __FUNCTION__, argIndex, sid,
                  surfaces[sid].inst[inst].generation, gen);
#endif
               result.status = VMACCEL_RESOURCE_UNAVAILABLE;
            }

            goto cleanup;
         }

#if CL_VERSION_2_0
         if (surfaces[sid].inst[inst].svm_ptr != NULL) {
            errNum = clSetKernelArgSVMPointer(
               kernel, argp->args.args_val[argIndex].index,
               surfaces[sid].inst[inst].svm_ptr);
            if (errNum == CL_SUCCESS) {
               errNum = clSetKernelExecInfo(
                  kernel, CL_KERNEL_EXEC_INFO_SVM_PTRS, sizeof(void *),
                  &surfaces[sid].inst[inst].svm_ptr);
            }
         } else
#endif
         {
            cl_mem mem = NULL;

            mem = surfaces[sid].inst[inst].mem;

            errNum = clSetKernelArg(kernel, argp->args.args_val[argIndex].index,
                                    sizeof(cl_mem), &mem);
         }

         if (errNum != CL_SUCCESS) {
            VMACCEL_WARNING("Unable to set kernel argument, errNum=%d\n",
                            errNum);
            result.status = VMACCEL_FAIL;
            goto cleanup;
         }
      } else {
         assert(0);
         result.status = VMACCEL_FAIL;
         goto cleanup;
      }
   }

   /*
    * TODO SVM: argp->refs --> clSetKernelExecInfo
    */
   assert(argp->dimension <= CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);

   globalWorkOffset = calloc(argp->dimension, sizeof(*globalWorkOffset));
   globalWorkSize = calloc(argp->dimension, sizeof(*globalWorkSize));
   localWorkSize = calloc(argp->dimension, sizeof(*localWorkSize));

   for (int i = 0; i < argp->dimension; i++) {
      globalWorkOffset[i] = argp->globalWorkOffset.globalWorkOffset_val[i];
      globalWorkSize[i] = argp->globalWorkSize.globalWorkSize_val[i];
      localWorkSize[i] = argp->localWorkSize.localWorkSize_val[i];
#if DEBUG_COMPUTE_OPERATION
      VMACCEL_LOG("%s: globalWorkOffset[%d]=%d, globalWorkSize[%d]=%d,"
                  " localWorkSize[%d]=%d\n",
                  __FUNCTION__, i, globalWorkOffset[i], i, globalWorkSize[i], i,
                  localWorkSize[i]);
#endif
   }

   /*
    * TODO: Evaluate events blocking this kernel and inserting an event here
    */
   errNum =
      clEnqueueNDRangeKernel(queue, kernel, argp->dimension, globalWorkOffset,
                             globalWorkSize, localWorkSize, 0, NULL, NULL);

   if (errNum != CL_SUCCESS) {
      result.status = VMACCEL_FAIL;
   }

cleanup:

   for (argIndex = MIN(argIndex, argp->args.args_len - 1); argIndex >= 0;
        argIndex--) {
      if (argp->args.args_val[argIndex].type == VMCL_ARG_SURFACE) {
         unsigned int sid = (unsigned int)argp->args.args_val[argIndex].surf.id;
         unsigned int inst =
            (unsigned int)argp->args.args_val[argIndex].surf.instance;

         pthread_mutex_unlock(&surfaces[sid].inst[inst].mutex);
         pthread_mutex_unlock(&surfaces[sid].mutex);
      }
   }

   if (localWorkSize != NULL) {
      free(localWorkSize);
   }

   if (globalWorkSize != NULL) {
      free(globalWorkSize);
   }

   if (globalWorkOffset != NULL) {
      free(globalWorkOffset);
   }

   return (&result);
}

/*
 * Setup the backend op dispatch
 */
VMCLOps vmwopenclOps = {
   vmwopencl_poweron,
   vmwopencl_poweroff,
   NULL,
   NULL,
   vmwopencl_contextalloc_1,
   vmwopencl_contextdestroy_1,
   vmwopencl_surfacealloc_1,
   vmwopencl_surfacedestroy_1,
   vmwopencl_queuealloc_1,
   vmwopencl_queuedestroy_1,
   vmwopencl_sampleralloc_1,
   vmwopencl_samplerdestroy_1,
   vmwopencl_kernelalloc_1,
   vmwopencl_kerneldestroy_1,
   vmwopencl_queueflush_1,
   vmwopencl_imageupload_1,
   vmwopencl_imagedownload_1,
   vmwopencl_surfacemap_1,
   vmwopencl_surfaceunmap_1,
   vmwopencl_surfacecopy_1,
   vmwopencl_imagefill_1,
   vmwopencl_dispatch_1,
};
