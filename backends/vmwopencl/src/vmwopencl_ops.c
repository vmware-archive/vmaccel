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
#include <string.h>

#include "vmwopencl.h"
#include "vmaccel_utils.h"
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
   cl_device_id deviceId;
   int majorVersion;
   int minorVersion;
} VMWOpenCLContext;

typedef struct VMWOpenCLSurface {
   VMAccelSurfaceDesc desc;
   cl_mem mem;
} VMWOpenCLSurface;

typedef struct VMWOpenCLQueue {
   VMAccelQueueDesc desc;
   cl_command_queue queue;
} VMWOpenCLQueue;

typedef struct VMWOpenCLFence {
   VMAccelFenceDesc desc;
   unsigned int pad;
} VMWOpenCLFence;

typedef struct VMWOpenCLEvent {
   VMAccelEventDesc desc;
   unsigned int pad;
} VMWOpenCLEvent;

typedef struct VMWOpenCLSampler { cl_sampler sampler; } VMWOpenCLSampler;

typedef struct VMWOpenCLKernel {
   /*
    * ???: Does one program per kernel affect variable sharing?
    */
   cl_program program;
   cl_kernel kernel;
} VMWOpenCLKernel;

typedef struct VMWOpenCLMapping { void *ptr; } VMWOpenCLMapping;

static VMWOpenCLCaps caps[VMACCEL_SELECT_MAX];

static VMWOpenCLContext *contexts = NULL;
static IdentifierDB *contextIds = NULL;

static VMWOpenCLSurface *surfaces = NULL;
static IdentifierDB *surfaceIds = NULL;

static VMWOpenCLQueue *queues = NULL;
static IdentifierDB *queueIds = NULL;

static VMWOpenCLFence *fences = NULL;
static IdentifierDB *fenceIds = NULL;

static VMWOpenCLEvent *events = NULL;
static IdentifierDB *eventIds = NULL;

static VMWOpenCLSampler *samplers = NULL;
static IdentifierDB *samplerIds = NULL;

static VMWOpenCLKernel *kernels = NULL;
static IdentifierDB *kernelIds = NULL;

static VMWOpenCLMapping *mappings = NULL;

const cl_int clDeviceTypes[VMACCEL_SELECT_MAX] = {
   CL_DEVICE_TYPE_GPU, CL_DEVICE_TYPE_ACCELERATOR, CL_DEVICE_TYPE_ACCELERATOR,
   CL_DEVICE_TYPE_CPU,
};

VMAccelStatus *vmwopencl_poweroff();

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
   int majorVersion;
   int minorVersion;
   int i, j;
   bool platformFound = false;

   memset(&result, 0, sizeof(result));

   Log("Powering on vmwopencl backend...\n");

   errNum = clGetPlatformIDs(sizeof(platforms) / sizeof(platforms[0]),
                             &platforms[0], &numPlatforms);

   if (errNum != CL_SUCCESS) {
      Warning("Failed to query platforms\n");
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   Log("Number of platforms: %d\n", numPlatforms);

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
         Warning("Failed to query platform version\n");
         result.status = VMACCEL_FAIL;
         return (&result);
      }

      Log("Version: %s\n", platformVersion);

      errNum = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME,
                                 sizeof(platformName), platformName, &sizeRet);

      if (errNum != CL_SUCCESS) {
         Warning("Failed to query platform name\n");
         result.status = VMACCEL_FAIL;
         return (&result);
      }

      Log("Platform: %s\n", platformName);

      errNum =
         clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR,
                           sizeof(platformVendor), platformVendor, &sizeRet);

      if (errNum != CL_SUCCESS) {
         Warning("Failed to query platform vendor\n");
         result.status = VMACCEL_FAIL;
         return (&result);
      }

      Log("Vendor: %s\n", platformVendor);

      errNum = clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS,
                                 sizeof(platformExtensions), platformExtensions,
                                 &sizeRet);

      if (errNum != CL_SUCCESS) {
         Warning("Failed to query platform extensions\n");
      }

      Log("Extensions: %s\n", platformExtensions);

      caps[accelArch].deviceId = deviceId;

      errNum = clGetDeviceInfo(deviceId, CL_DEVICE_NAME, sizeof(deviceName),
                               deviceName, &sizeRet);

      if (errNum != CL_SUCCESS) {
         Warning("Unable to query CL_DEVICE_NAME\n");
         continue;
      }

      snprintf(prefix, sizeof(prefix), "Device[%p]", deviceId);

      Log("%s: DEVICE_NAME = %s\n", prefix, deviceName);

#define CAP(__TYPE, __NAME)                                                    \
   do {                                                                        \
      __TYPE val;                                                              \
      errNum = clGetDeviceInfo(deviceId, CL_##__NAME, sizeof(__TYPE), &val,    \
                               &sizeRet);                                      \
      if (errNum != CL_SUCCESS) {                                              \
         Warning("Unable to query %s\n", "CL_" #__NAME);                       \
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
         Warning("Unable to query CL_DEVICE_MAX_WORK_ITEM_SIZES\n");
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
       (strstr(platformVersion, "OpenCL 2.") == NULL)) {
      Warning("Unknown version detected: %s\n", platformVersion);
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
   result.desc.type = VMACCEL_COMPUTE_ACCELERATOR;
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

   fences = calloc(VMCL_MAX_FENCES, sizeof(VMWOpenCLFence));
   fenceIds = IdentifierDB_Alloc(VMCL_MAX_FENCES);

   events = calloc(VMCL_MAX_EVENTS, sizeof(VMWOpenCLEvent));
   eventIds = IdentifierDB_Alloc(VMCL_MAX_EVENTS);

   samplers = calloc(VMCL_MAX_SAMPLERS, sizeof(VMWOpenCLSampler));
   samplerIds = IdentifierDB_Alloc(VMCL_MAX_SAMPLERS);

   kernels = calloc(VMCL_MAX_KERNELS, sizeof(VMWOpenCLKernel));
   kernelIds = IdentifierDB_Alloc(VMCL_MAX_KERNELS);

   /*
    * Create a one-to-one mapping to surfaces.
    */
   mappings = calloc(VMCL_MAX_SURFACES, sizeof(VMWOpenCLMapping));

   /*
    * Final check for allocation failure.
    */
   if ((contexts == NULL) || (contextIds == NULL) || (surfaces == NULL) ||
       (surfaceIds == NULL) || (queues == NULL) || (queueIds == NULL) ||
       (fences == NULL) || (fenceIds == NULL) || (events == NULL) ||
       (eventIds == NULL) || (samplers == NULL) || (samplerIds == NULL) ||
       (kernels == NULL) || (kernelIds == NULL) || (mappings == NULL)) {
      Warning("Unable to allocate object database...\n");
      result.status = VMACCEL_FAIL;
      vmwopencl_poweroff();
   } else {
      result.status = VMACCEL_SUCCESS;
   }

   result.desc.maxContexts = VMCL_MAX_CONTEXTS;
   result.desc.maxQueues = VMCL_MAX_QUEUES;
   result.desc.maxFences = VMCL_MAX_FENCES;
   result.desc.maxEvents = VMCL_MAX_EVENTS;
   result.desc.maxSurfaces = VMCL_MAX_SURFACES;
   result.desc.maxMappings = VMCL_MAX_SURFACES;

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

   IdentifierDB_Free(fenceIds);
   free(fences);

   IdentifierDB_Free(eventIds);
   free(events);

   IdentifierDB_Free(samplerIds);
   free(samplers);

   IdentifierDB_Free(kernelIds);
   free(kernels);

   free(mappings);

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
   cl_device_id deviceId;
   cl_int errNum;
   char platformName[128] = {'\0'};
   char platformVersion[128] = {'\0'};
   char platformExtensions[128] = {'\0'};
   int majorVersion;
   int minorVersion;
   int i = 0, j = 0;

   memset(&result, 0, sizeof(result));

   if (IdentifierDB_ActiveId(contextIds, cid)) {
      Warning("Context ID %d already active...\n", cid);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   // First, select an OpenCL platform to run on.  For this example, we
   // simply choose the first available platform.  Normally, you would
   // query for all available platforms and select the most appropriate one.
   errNum = clGetPlatformIDs(sizeof(platforms) / sizeof(platforms[0]),
                             &platforms[0], &numPlatforms);

   if (errNum != CL_SUCCESS || numPlatforms <= 0) {
      Warning("Failed to find any OpenCL platforms.\n");
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   for (i = 0; i < numPlatforms; i++) {
      // Next, create an OpenCL context on the platform.  Attempt to
      // create a GPU-based context, and if that fails, try to create
      // a CPU-based context.
      cl_context_properties contextProperties[] = {
         CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[i], 0};

      errNum =
         clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION,
                           sizeof(platformVersion), platformVersion, &sizeRet);

      if (errNum != CL_SUCCESS) {
         Warning("Failed to query platform version\n");
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
      } else {
         Warning("Unknown version detected: %s\n", platformVersion);
         continue;
      }

      // SPIR-V 1.0 requires OpenCL 2.1
      if ((argp->requiredCaps & VMCL_SPIRV_1_0_CAP) &&
          ((majorVersion < 2) || (minorVersion < 1))) {
         Warning("SPIRV 1.0 requires OpenCL 2.1+\n");
         continue;
      } else {
         result.caps |= VMCL_SPIRV_1_0_CAP;
      }

      // SPIR-V 1.1/1.2 requires OpenCL 2.2
      if (((argp->requiredCaps & VMCL_SPIRV_1_1_CAP) ||
           (argp->requiredCaps & VMCL_SPIRV_1_2_CAP)) &&
          ((majorVersion < 2) || (minorVersion < 2))) {
         Warning("SPIRV 1.1/1.2 requires OpenCL 2.2+\n");
         continue;
      } else {
         result.caps |= VMCL_SPIRV_1_1_CAP | VMCL_SPIRV_1_2_CAP;
      }

      errNum = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME,
                                 sizeof(platformName), platformName, &sizeRet);

      if (errNum != CL_SUCCESS) {
         Warning("Failed to query platform name\n");
         continue;
      }

      Log("Using Platform: %s\n", platformName);
      Log("  Version: %s\n", platformVersion);

      for (j = 0; j < VMACCEL_SELECT_MAX; j++) {
         char deviceName[128];

         if ((argp->selectionMask & (1 << j)) == 0) {
            continue;
         }

         errNum =
            clGetDeviceIDs(platforms[i], clDeviceTypes[j], 1, &deviceId, NULL);

         if (errNum != CL_SUCCESS) {
            continue;
         }

         errNum = clGetDeviceInfo(deviceId, CL_DEVICE_NAME, sizeof(deviceName),
                                  deviceName, &sizeRet);

         if (errNum != CL_SUCCESS) {
            Warning("Failed to query device name\n");
            continue;
         }

         Log("Device Allocated: %s\n", deviceName);

         context = clCreateContext(0, 1, &deviceId, NULL, NULL, &errNum);

         if (errNum != CL_SUCCESS) {
            Warning("Unable to create context, errNum=%d\n", errNum);
         } else if (context != NULL) {
            break;
         }
      }

      if (context != NULL) {
         break;
      }
   }

   if ((context == NULL) || (errNum != CL_SUCCESS)) {
      Warning("Failed to create an OpenCL context.\n");
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   if (IdentifierDB_AcquireId(contextIds, cid)) {
      contexts[cid].context = context;
      contexts[cid].platformId = platforms[i];
      contexts[cid].deviceId = deviceId;
      contexts[cid].majorVersion = majorVersion;
      contexts[cid].minorVersion = minorVersion;
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

   assert(IdentifierDB_ActiveId(contextIds, cid));

   clReleaseContext(contexts[cid].context);
   contexts[cid].context = NULL;

   IdentifierDB_ReleaseId(contextIds, cid);

   return (&result);
}

VMAccelSurfaceAllocateStatus *
vmwopencl_surfacealloc_1(VMCLSurfaceAllocateDesc *argp) {
   static VMAccelSurfaceAllocateStatus result;
   unsigned int cid = (unsigned int)argp->client.cid;
   unsigned int sid = (unsigned int)argp->client.accel.id;
   cl_context context = contexts[cid].context;
   cl_mem memObject = NULL;
   unsigned int clMemFlags = 0;

   memset(&result, 0, sizeof(result));

   if (IdentifierDB_ActiveId(surfaceIds, sid)) {
      Warning("Surface ID %d already active...\n", sid);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   if (argp->desc.type == VMACCEL_SURFACE_BUFFER) {
      if (argp->desc.usage == VMACCEL_SURFACE_USAGE_READONLY) {
         clMemFlags |= CL_MEM_READ_ONLY;
      } else if (argp->desc.usage == VMACCEL_SURFACE_USAGE_WRITEONLY) {
         clMemFlags |= CL_MEM_WRITE_ONLY;
      } else if (argp->desc.usage == VMACCEL_SURFACE_USAGE_READWRITE) {
         clMemFlags |= CL_MEM_READ_WRITE;
      }
      assert(argp->desc.format == VMACCEL_FORMAT_R8_TYPELESS);

      memObject =
         clCreateBuffer(context, clMemFlags, argp->desc.width, NULL, NULL);
   } else {
      assert(argp->desc.usage == VMACCEL_SURFACE_USAGE_READWRITE);
      clMemFlags |= CL_MEM_READ_WRITE;

      /* Image from buffer? */

      result.status = VMACCEL_FAIL;
      return (&result);
   }

   if (memObject == NULL) {
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   if (IdentifierDB_AcquireId(surfaceIds, sid)) {
      surfaces[sid].desc = argp->desc;
      surfaces[sid].mem = memObject;
   } else {
      assert(0);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
   }

   return (&result);
}

VMAccelStatus *vmwopencl_surfacedestroy_1(VMCLSurfaceId *argp) {
   static VMAccelStatus result;
   unsigned int sid = (unsigned int)argp->accel.id;

   memset(&result, 0, sizeof(result));

   assert(IdentifierDB_ActiveId(surfaceIds, sid));

   clReleaseMemObject(surfaces[sid].mem);
   memset(&surfaces[sid], 0, sizeof(surfaces[0]));

   IdentifierDB_ReleaseId(surfaceIds, sid);

   return (&result);
}

VMAccelSharedHandleStatus *
vmwopencl_surfacegetsharedhandle_1(VMCLSurfaceId *argp) {
   static VMAccelSharedHandleStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelStatus *vmwopencl_surfacereleasesharedhandle_1(VMCLSharedHandle *argp) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelQueueStatus *vmwopencl_queuealloc_1(VMCLQueueAllocateDesc *argp) {
   static VMAccelQueueStatus result;
   unsigned int cid = (unsigned int)argp->client.cid;
   unsigned int qid = (unsigned int)argp->client.id;
   cl_context context = contexts[cid].context;
   cl_int errNum;
   cl_device_id device;
   cl_device_id *devices;
   cl_command_queue commandQueue = NULL;
   size_t deviceBufferSize = -1;

   memset(&result, 0, sizeof(result));

   if (IdentifierDB_ActiveId(queueIds, qid)) {
      Warning("Queue ID %d already active...\n", qid);
      result.status = VMACCEL_RESOURCE_UNAVAILABLE;
      return (&result);
   }

   // First get the size of the devices buffer
   errNum =
      clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);

   if (errNum != CL_SUCCESS) {
      Warning("Failed call to clGetContextInfo(...,GL_CONTEXT_DEVICES,...)");
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   if (deviceBufferSize <= 0) {
      Warning("No devices available.");
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   // Allocate memory for the devices buffer
   devices =
      calloc(deviceBufferSize / sizeof(cl_device_id), sizeof(cl_device_id));

   errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize,
                             devices, NULL);

   if (errNum != CL_SUCCESS) {
      free(devices);
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   // In this example, we just choose the first available device.  In a
   // real program, you would likely use all available devices or choose
   // the highest performance device based on OpenCL device queries
   commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

   if (commandQueue == NULL) {
      Warning("Failed to create commandQueue for device 0");
      free(devices);
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   device = devices[0];
   free(devices);

   if (IdentifierDB_AcquireId(queueIds, qid)) {
      queues[qid].desc = argp->desc;
      queues[qid].queue = commandQueue;
   } else {
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

VMAccelEventStatus *vmwopencl_eventalloc_1(VMCLEventAllocateDesc *argp) {
   static VMAccelEventStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelEventStatus *vmwopencl_eventgetstatus_1(VMCLEventId *argp) {
   static VMAccelEventStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelEventStatus *vmwopencl_eventdestroy_1(VMCLEventId *argp) {
   static VMAccelEventStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelFenceStatus *vmwopencl_fencealloc_1(VMCLFenceAllocateDesc *argp) {
   static VMAccelFenceStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelFenceStatus *vmwopencl_fencegetstatus_1(VMCLFenceId *argp) {
   static VMAccelFenceStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelFenceStatus *vmwopencl_fencedestroy_1(VMCLFenceId *argp) {
   static VMAccelFenceStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelStatus *vmwopencl_queueflush_1(VMCLQueueId *argp) {
   static VMAccelStatus result;
   unsigned int cid = (unsigned int)argp->cid;
   unsigned int qid = (unsigned int)argp->id;
   cl_int errNum;

   memset(&result, 0, sizeof(result));

   errNum = clFlush(queues[qid].queue);

   if (errNum != CL_SUCCESS) {
      result.status = VMACCEL_FAIL;
   }

   return (&result);
}

VMAccelStatus *vmwopencl_eventinsert_1(VMCLEventInsertOp *argp) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelStatus *vmwopencl_fenceinsert_1(VMCLFenceInsertOp *argp) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelStatus *vmwopencl_imageupload_1(VMCLImageUploadOp *argp) {
   static VMAccelStatus result;
   unsigned int cid = (unsigned int)argp->queue.cid;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->img.accel.id;
   cl_context context = contexts[cid].context;
   cl_command_queue queue = queues[qid].queue;
   cl_map_flags flags = 0;
   cl_int errNum;

   memset(&result, 0, sizeof(result));

   assert(cid == argp->img.cid);

   if (surfaces[sid].desc.type == VMACCEL_SURFACE_BUFFER) {
      errNum = clEnqueueWriteBuffer(
         queue, surfaces[sid].mem, FALSE, argp->op.imgRegion.coord.x,
         argp->op.imgRegion.size.x, argp->op.ptr.ptr_val, 0, NULL, NULL);

      if (errNum != CL_SUCCESS) {
         result.status = VMACCEL_FAIL;
      }
   } else {
      assert(0);

      result.status = VMACCEL_FAIL;
   }

   return (&result);
}

VMAccelDownloadStatus *vmwopencl_imagedownload_1(VMCLImageDownloadOp *argp) {
   static VMAccelDownloadStatus result;
   unsigned int cid = (unsigned int)argp->queue.cid;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->img.accel.id;
   cl_context context = contexts[cid].context;
   cl_command_queue queue = queues[qid].queue;
   void *ptr;
   cl_map_flags flags = 0;
   cl_int errNum;

   memset(&result, 0, sizeof(result));

   assert(cid == argp->img.cid);

   if (surfaces[sid].desc.type == VMACCEL_SURFACE_BUFFER) {
      unsigned int blocking = 0;

      ptr = calloc(1, argp->op.imgRegion.size.x);

      if (argp->mode == VMACCEL_SURFACE_READ_SYNCHRONOUS) {
         blocking = TRUE;
      }

      errNum = clEnqueueReadBuffer(
         queue, surfaces[sid].mem, blocking, argp->op.imgRegion.coord.x,
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

   return (&result);
}

VMAccelSurfaceMapStatus *vmwopencl_surfacemap_1(VMCLSurfaceMapOp *argp) {
   static VMAccelSurfaceMapStatus result;
   unsigned int cid = (unsigned int)argp->queue.cid;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->op.surf.id;
   cl_context context = contexts[cid].context;
   cl_command_queue queue = queues[qid].queue;
   void *ptr;
   cl_map_flags flags = 0;
   cl_int errNum;

   memset(&result, 0, sizeof(result));

   /*
    * Only allow one mapping per surface.
    */
   assert(mappings[sid].ptr == NULL);

   if (argp->op.mapFlags & VMACCEL_MAP_READ_FLAG) {
      flags |= CL_MAP_READ;
   }

   if (argp->op.mapFlags & VMACCEL_MAP_WRITE_FLAG) {
      flags |= CL_MAP_WRITE;
   }

   if (argp->op.mapFlags & VMACCEL_MAP_WRITE_DISCARD_FLAG) {
      flags |= CL_MAP_WRITE_INVALIDATE_REGION;
   }

   if (surfaces[sid].desc.type == VMACCEL_SURFACE_BUFFER) {
      ptr = clEnqueueMapBuffer(queue, surfaces[sid].mem, TRUE, flags,
                               argp->op.coord.x, argp->op.size.x, 0, NULL, NULL,
                               &errNum);

      if (errNum != CL_SUCCESS) {
         result.status = VMACCEL_FAIL;
      } else {
         mappings[sid].ptr = ptr;

         result.ptr.ptr_val = ptr;
         result.ptr.ptr_len = argp->op.size.x;
      }
   } else {
      assert(0);
      result.status = VMACCEL_FAIL;
   }

   return (&result);
}

VMAccelStatus *vmwopencl_surfaceunmap_1(VMCLSurfaceUnmapOp *argp) {
   static VMAccelStatus result;
   unsigned int cid = (unsigned int)argp->queue.cid;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int sid = (unsigned int)argp->op.surf.id;
   cl_context context = contexts[cid].context;
   cl_command_queue queue = queues[qid].queue;
   void *ptr;
   cl_int errNum;

   memset(&result, 0, sizeof(result));

   /*
    * memcpy the data from the incoming mapping object.
    */
   ptr = mappings[sid].ptr;

   memcpy(ptr, argp->op.ptr.ptr_val, argp->op.ptr.ptr_len);

   errNum = clEnqueueUnmapMemObject(queues[qid].queue, surfaces[sid].mem, ptr,
                                    0, NULL, NULL);

   mappings[sid].ptr = NULL;

   if (errNum != CL_SUCCESS) {
      result.status = VMACCEL_FAIL;
   }

   return (&result);
}

VMAccelStatus *vmwopencl_surfacecopy_1(VMCLSurfaceCopyOp *argp) {
   static VMAccelStatus result;
   unsigned int cid = (unsigned int)argp->queue.cid;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int dstSid = (unsigned int)argp->dst.accel.id;
   unsigned int srcSid = (unsigned int)argp->src.accel.id;
   cl_context context = contexts[cid].context;
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

   if ((surfaces[dstSid].desc.type == VMACCEL_SURFACE_BUFFER) &&
       (surfaces[srcSid].desc.type == VMACCEL_SURFACE_BUFFER)) {
      errNum = clEnqueueCopyBuffer(
         queue, surfaces[srcSid].mem, surfaces[dstSid].mem,
         argp->op.srcRegion.coord.x, argp->op.dstRegion.coord.x,
         argp->op.dstRegion.size.x, 0, NULL, NULL);

      if (errNum != CL_SUCCESS) {
         result.status = VMACCEL_FAIL;
      }
   } else {
      assert(0);
      result.status = VMACCEL_FAIL;
   }

   return (&result);
}

VMAccelStatus *vmwopencl_imagefill_1(VMCLImageFillOp *argp) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   assert(0);

   result.status = VMACCEL_FAIL;

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
   cl_context context = contexts[cid].context;
   cl_device_id deviceId = contexts[cid].deviceId;
   cl_kernel kernel = 0;
   cl_int errNum;
   cl_program program;
   size_t sourceLength = argp->source.source_len;

   memset(&result, 0, sizeof(result));

   Log("Allocating kernel id=%d\n", kid);

   if (IdentifierDB_ActiveId(kernelIds, kid)) {
      Warning("Kernel ID %d already active...\n", kid);
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
      Log("Creating OpenCL C program\n");

      program = clCreateProgramWithSource(
         context, 1, (const char **)&argp->source.source_val, &sourceLength,
         &errNum);
#if CL_VERSION_2_2
   } else if ((contexts[cid].majorVersion >= 2) &&
              (contexts[cid].minorVersion >= 2) &&
              (argp->language == VMCL_OPENCL_CPP_1_0)) {
      Log("Creating OpenCL CPP program\n");

      program = clCreateProgramWithSource(
         context, 1, (const char **)&argp->source.source_val, &sourceLength,
         &errNum);
   } else if ((contexts[cid].majorVersion >= 2) &&
              (contexts[cid].minorVersion >= 2) &&
              ((argp->language == VMCL_SPIRV_1_1) ||
               (argp->language == VMCL_SPIRV_1_2))) {
      Log("Creating SPIR-V 1.1/1.2 program\n");

      program = clCreateProgramWithIL(context, (void *)argp->source.source_val,
                                      argp->source.source_len, &errNum);
#endif
#if CL_VERSION_2_1
   } else if ((contexts[cid].majorVersion >= 2) &&
              (contexts[cid].minorVersion >= 1) &&
              (argp->language == VMCL_SPIRV_1_0)) {
      Log("Creating SPIR-V 1.0 program\n");

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
      Warning("Failed to create CL program\n");
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   Log("Building program %p\n", program);

   errNum = clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);

   if (errNum != CL_SUCCESS) {
      char *buildLog;

      Warning("Error in kernel:\n");

      buildLog = malloc(sizeof(char) * 16384);

      if (buildLog) {
         clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, 16384,
                               buildLog, NULL);
         Warning("%s\n", buildLog);
         free(buildLog);
      }

      clReleaseProgram(program);
      result.status = VMACCEL_FAIL;
      return (&result);
   }

   // Create OpenCL kernel
   kernel = clCreateKernel(program, argp->kernelName.kernelName_val, NULL);
   if (kernel == NULL) {
      Warning("Failed to create kernel\n");
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
   unsigned int cid = (unsigned int)argp->queue.cid;
   unsigned int qid = (unsigned int)argp->queue.id;
   unsigned int kid = (unsigned int)argp->kernel.id;
   cl_context context = contexts[cid].context;
   cl_command_queue queue = queues[qid].queue;
   cl_kernel kernel = kernels[kid].kernel;
   cl_int errNum;
   size_t *globalWorkOffset;
   size_t *globalWorkSize;
   size_t *localWorkSize;
   unsigned int i;

   for (i = 0; i < argp->args.args_len; i++) {
      if (argp->args.args_val[i].type == VMCL_ARG_SURFACE) {
         unsigned int sid = (unsigned int)argp->args.args_val[i].surf.id;
         cl_mem mem = surfaces[sid].mem;

         errNum = clSetKernelArg(kernel, argp->args.args_val[i].index,
                                 sizeof(cl_mem), &mem);

         if (errNum != CL_SUCCESS) {
            result.status = VMACCEL_FAIL;
            return (&result);
         }
      } else {
         assert(0);
         result.status = VMACCEL_FAIL;
         return (&result);
      }
   }

   /*
    * TODO SVM: argp->refs --> clSetKernelExecInfo
    */
   assert(argp->dimension <= CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);

   globalWorkOffset = calloc(argp->dimension, sizeof(*globalWorkOffset));
   globalWorkSize = calloc(argp->dimension, sizeof(*globalWorkSize));
   localWorkSize = calloc(argp->dimension, sizeof(*localWorkSize));

   for (i = 0; i < argp->dimension; i++) {
      globalWorkOffset[i] = argp->globalWorkOffset.globalWorkOffset_val[i];
      globalWorkSize[i] = argp->globalWorkSize.globalWorkSize_val[i];
      localWorkSize[i] = argp->localWorkSize.localWorkSize_val[i];
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

   free(localWorkSize);
   free(globalWorkSize);
   free(globalWorkOffset);

   return (&result);
}

/*
 * Setup the backend op dispatch
 */
VMCLOps vmwopenclOps = {
   vmwopencl_poweron, vmwopencl_poweroff, NULL, NULL, vmwopencl_contextalloc_1,
   vmwopencl_contextdestroy_1, vmwopencl_surfacealloc_1,
   vmwopencl_surfacedestroy_1, vmwopencl_surfacegetsharedhandle_1,
   vmwopencl_surfacereleasesharedhandle_1, vmwopencl_queuealloc_1,
   vmwopencl_queuedestroy_1, vmwopencl_eventalloc_1, vmwopencl_eventdestroy_1,
   vmwopencl_fencealloc_1, vmwopencl_fencedestroy_1, vmwopencl_sampleralloc_1,
   vmwopencl_samplerdestroy_1, vmwopencl_kernelalloc_1,
   vmwopencl_kerneldestroy_1, vmwopencl_queueflush_1, vmwopencl_eventinsert_1,
   vmwopencl_eventgetstatus_1, vmwopencl_fenceinsert_1,
   vmwopencl_fencegetstatus_1, vmwopencl_imageupload_1,
   vmwopencl_imagedownload_1, vmwopencl_surfacemap_1, vmwopencl_surfaceunmap_1,
   vmwopencl_surfacecopy_1, vmwopencl_imagefill_1, vmwopencl_dispatch_1,
};
