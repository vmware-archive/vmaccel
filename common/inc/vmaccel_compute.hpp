/******************************************************************************

Copyright (c) 2019 VMware, Inc.
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

/**
 * vmaccel_compute.hpp
 *
 * Accelerator C++11 external interface. The interface class will connect to a
 * given Accelerator Manager Server and negotiate Accelerators based upon the
 * requested operation.
 *
 * The classes below use safe types, such as std::string vs. char *, to avoid
 * security issues with unbounded access. To retain an object over the lifetime
 * of an Accelerator operation, std::shared_ptr is required for all arguments.
 */

#ifndef _VMACCEL_COMPUTE_HPP_
#define _VMACCEL_COMPUTE_HPP_ 1

#include "vmaccel.hpp"
#include "vmaccel_defs.h"
#include "vmcl_defs.h"
#include "vmcl_rpc.h"
#include <cassert>
#include <map>
#include <string>
#include <vector>

#define DEBUG_TEMPLATE_TYPES 0
#define DEBUG_COMPUTE_OPERATION 0

namespace vmaccel {

typedef unsigned int VMCLKernelArchitecture;

/**
 * prepareComputeArgs
 *
 * Recursively prepare Compute Accelerator arguments for consumption by the
 * Compute Kernel. Arguments must be declared in the order as they are
 * declared in the Compute Kernel.
 */
template <typename T>
bool prepareComputeArgs(CLIENT *clnt, unsigned int contextId,
                        unsigned int queueId, VMCLKernelArgDesc *kernelArgs,
                        unsigned int *surfaceIds, unsigned int argIndex,
                        T arg) {
   VMAccelSurfaceAllocateReturnStatus *result_1;
   VMCLSurfaceAllocateDesc vmcl_surfacealloc_1_arg;
   VMAccelSurfaceMapReturnStatus *result_2;
   VMCLSurfaceMapOp vmcl_surfacemap_1_arg;
   VMAccelReturnStatus *result_3;
   VMCLSurfaceUnmapOp vmcl_surfaceunmap_1_arg;

#if DEBUG_TEMPLATE_TYPES
   Log("%s: argIndex = %u, type = %s\n", __FUNCTION__, argIndex,
       typeid(T).name());
#endif

   kernelArgs[argIndex].surf.id = VMACCEL_INVALID_ID;
   kernelArgs[argIndex].index = -1;

   memset(&vmcl_surfacealloc_1_arg, 0, sizeof(vmcl_surfacealloc_1_arg));
   vmcl_surfacealloc_1_arg.client.cid = contextId;
   vmcl_surfacealloc_1_arg.client.accel.id = argIndex;
   vmcl_surfacealloc_1_arg.desc.type = VMACCEL_SURFACE_BUFFER;
   vmcl_surfacealloc_1_arg.desc.width = arg.get_size();
   vmcl_surfacealloc_1_arg.desc.format = VMACCEL_FORMAT_R8_TYPELESS;
   vmcl_surfacealloc_1_arg.desc.usage = arg.get_usage();
   vmcl_surfacealloc_1_arg.desc.bindFlags = VMACCEL_BIND_UNORDERED_ACCESS_FLAG;

   result_1 = vmcl_surfacealloc_1(&vmcl_surfacealloc_1_arg, clnt);
   if (result_1 == NULL) {
      return false;
   }

   kernelArgs[argIndex].surf.id = vmcl_surfacealloc_1_arg.client.accel.id;

   memset(&vmcl_surfacemap_1_arg, 0, sizeof(vmcl_surfacemap_1_arg));
   vmcl_surfacemap_1_arg.queue.cid = contextId;
   vmcl_surfacemap_1_arg.queue.id = queueId;
   vmcl_surfacemap_1_arg.op.surf.id = (VMAccelId)argIndex;
   vmcl_surfacemap_1_arg.op.size.x = arg.get_size();
   vmcl_surfacemap_1_arg.op.mapFlags =
      VMACCEL_MAP_READ_FLAG | VMACCEL_MAP_WRITE_FLAG;

   result_2 = vmcl_surfacemap_1(&vmcl_surfacemap_1_arg, clnt);

   if (result_2 != NULL &&
       result_2->VMAccelSurfaceMapReturnStatus_u.ret->status ==
          VMACCEL_SUCCESS) {
      void *ptr = result_2->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_val;

      assert(result_2->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_len ==
             vmcl_surfacemap_1_arg.op.size.x);

      /*
       * Memory copy the contents into the value
       */
      memcpy(ptr, arg.get_ptr(), vmcl_surfacemap_1_arg.op.size.x);

#if DEBUG_TEMPLATE_TYPES
      for (int i = 0;
           i < vmcl_surfacemap_1_arg.op.size.x / sizeof(unsigned int); i++) {
         Log("%s: in_uint32[%d]=%d\n", __FUNCTION__, i,
             ((unsigned int *)ptr)[i]);
      }
#endif

      memset(&vmcl_surfaceunmap_1_arg, 0, sizeof(vmcl_surfaceunmap_1_arg));
      vmcl_surfaceunmap_1_arg.queue.cid = contextId;
      vmcl_surfaceunmap_1_arg.queue.id = queueId;
      vmcl_surfaceunmap_1_arg.op.surf.id = argIndex;
      vmcl_surfaceunmap_1_arg.op.ptr.ptr_len = vmcl_surfacemap_1_arg.op.size.x;
      vmcl_surfaceunmap_1_arg.op.ptr.ptr_val = (char *)ptr;

      result_3 = vmcl_surfaceunmap_1(&vmcl_surfaceunmap_1_arg, clnt);
      if (result_3 == NULL) {
         return false;
      }

      /*
       * Free the allocation from vmcl_surfacemap_1 -> xdr_bytes. Note this
       * should match the allocation function for XDR.
       */
      free(ptr);

      kernelArgs[argIndex].index = argIndex;
      kernelArgs[argIndex].type = VMCL_ARG_SURFACE;
      kernelArgs[argIndex].surf.id = vmcl_surfacealloc_1_arg.client.accel.id;
   }

   return true;
}

template <typename T, typename... R>
bool prepareComputeArgs(CLIENT *clnt, unsigned int contextId,
                        unsigned int queueId, VMCLKernelArgDesc *kernelArgs,
                        unsigned int *surfaceIds, unsigned int argIndex, T arg,
                        R... args) {
   if (!prepareComputeArgs(clnt, contextId, queueId, kernelArgs, surfaceIds,
                           argIndex, arg)) {
      return false;
   }
   return prepareComputeArgs(clnt, contextId, queueId, kernelArgs, surfaceIds,
                             ++argIndex, args...);
}

/**
 * quiesceComputeArgs
 *
 * Recursively retrieve Compute Accelerator arguments for consumption by the
 * application. Arguments must be declared in the order as they are declared
 * in the Compute Kernel.
 */
template <typename T>
bool quiesceComputeArgs(CLIENT *clnt, unsigned int contextId,
                        unsigned int queueId, VMCLKernelArgDesc *kernelArgs,
                        unsigned int *surfaceIds, unsigned int argIndex,
                        T arg) {
   VMAccelSurfaceMapReturnStatus *result_1;
   VMCLSurfaceMapOp vmcl_surfacemap_1_arg;
   VMAccelReturnStatus *result_2;
   VMCLSurfaceUnmapOp vmcl_surfaceunmap_1_arg;
   VMAccelReturnStatus *result_3;
   VMCLSurfaceId vmcl_surfacedestroy_1_arg;

#if DEBUG_TEMPLATE_TYPES
   Log("%s: argIndex = %u, type = %s\n", __FUNCTION__, argIndex,
       typeid(T).name());
#endif

   if (kernelArgs[argIndex].surf.id == VMACCEL_INVALID_ID) {
      return true;
   }

   if (arg.get_usage() != VMACCEL_SURFACE_USAGE_READONLY) {
      memset(&vmcl_surfacemap_1_arg, 0, sizeof(vmcl_surfacemap_1_arg));
      vmcl_surfacemap_1_arg.queue.cid = contextId;
      vmcl_surfacemap_1_arg.queue.id = queueId;
      vmcl_surfacemap_1_arg.op.surf.id =
         (VMAccelId)kernelArgs[argIndex].surf.id;
      vmcl_surfacemap_1_arg.op.size.x = arg.get_size();
      vmcl_surfacemap_1_arg.op.mapFlags = VMACCEL_MAP_READ_FLAG;

      result_1 = vmcl_surfacemap_1(&vmcl_surfacemap_1_arg, clnt);

      if (result_1 != NULL &&
          result_1->VMAccelSurfaceMapReturnStatus_u.ret->status ==
             VMACCEL_SUCCESS) {
         unsigned int *ptr =
            (unsigned int *)
               result_1->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_val;

         assert(result_1->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_len ==
                vmcl_surfacemap_1_arg.op.size.x);

#if DEBUG_TEMPLATE_TYPES
         for (int i = 0;
              i < vmcl_surfacemap_1_arg.op.size.x / sizeof(unsigned int); i++) {
            Log("%s: out_uint32[%d]=%d\n", __FUNCTION__, i,
                ((unsigned int *)ptr)[i]);
         }
#endif

         memcpy(arg.get_ptr(), ptr, vmcl_surfacemap_1_arg.op.size.x);

         memset(&vmcl_surfaceunmap_1_arg, 0, sizeof(vmcl_surfaceunmap_1_arg));
         vmcl_surfaceunmap_1_arg.queue.cid = contextId;
         vmcl_surfaceunmap_1_arg.queue.id = queueId;
         vmcl_surfaceunmap_1_arg.op.surf.id = kernelArgs[argIndex].surf.id;
         vmcl_surfaceunmap_1_arg.op.ptr.ptr_len =
            vmcl_surfacemap_1_arg.op.size.x;
         vmcl_surfaceunmap_1_arg.op.ptr.ptr_val = (char *)ptr;

         result_2 = vmcl_surfaceunmap_1(&vmcl_surfaceunmap_1_arg, clnt);
         if (result_2 == NULL) {
            return false;
         }
      }
   }

   memset(&vmcl_surfacedestroy_1_arg, 0, sizeof(vmcl_surfacedestroy_1_arg));
   vmcl_surfacedestroy_1_arg.cid = contextId;
   vmcl_surfacedestroy_1_arg.accel.id = kernelArgs[argIndex].surf.id;

   result_3 = vmcl_surfacedestroy_1(&vmcl_surfacedestroy_1_arg, clnt);
   if (result_3 == NULL) {
      return false;
   }

   return true;
}

template <typename T, typename... R>
bool quiesceComputeArgs(CLIENT *clnt, unsigned int contextId,
                        unsigned int queueId, VMCLKernelArgDesc *kernelArgs,
                        unsigned int *surfaceIds, unsigned int argIndex, T arg,
                        R... args) {
   if (!quiesceComputeArgs(clnt, contextId, queueId, kernelArgs, surfaceIds,
                           argIndex, arg)) {
      return false;
   }
   return quiesceComputeArgs(clnt, contextId, queueId, kernelArgs, surfaceIds,
                             ++argIndex, args...);
}

/**
 * Compute operation for compute kernels.
 *
 * Operation will allocate and instantiate a compute kernel on an available
 * Compute Accelerator. The specified kernel function is instantiated
 * per-thread, allowing for multi-dimensional partitioning of the working
 * set. The number of concurrent threads is defined as follows:
 *
 *   n = numDimensions
 *   # of threads = n * SUM(globalWorkSizes[0] ... globalWorkSizes[n]) *
 *                  SUM(localWorkSizes[0] ... localWorkSizes[n])
 *
 * The working set comprises of Input and Output to the function, and
 * is specified in the args parameter.
 *
 * @param accel Accelerator class used to instantiate the compute operation.
 * @param kernelType Kernel type as defined in vmcl_defs.h, e.g.
 *                   VMCL_SPIRV_1_0.
 * @param kernel A vector of per-architecture kernels.
 * @param kernelFunction Name of the function in the kernel to instantiate.
 * @param computeTopology Topology for the Accelerator's threading model.
 * @param args Packed list of arguments to pass to the kernel function, will be
 *             evaluated in declared order for both input and output.
 * @return VMAccelStatusCodeEnum value.
 */

template <class... ARGTYPES>
int compute(
   vmaccel::accelerator &accel, const VMCLKernelLanguageType kernelType,
   const std::map<VMCLKernelArchitecture, vmaccel::ref_object<char>> &kernel,
   const std::string &kernelFunction,
   const vmaccel::work_topology &computeTopology, ARGTYPES... args) {
   CLIENT *clnt;
   VMAccelAllocateReturnStatus *result_1;
   VMAccelDesc vmaccelmgr_alloc_1_arg;
   VMCLContextAllocateReturnStatus *result_2;
   VMCLContextAllocateDesc vmcl_contextalloc_1_arg;
   VMAccelQueueReturnStatus *result_3;
   VMCLQueueAllocateDesc vmcl_queuealloc_1_arg;
   VMCLKernelAllocateReturnStatus *result_4;
   VMCLKernelAllocateDesc vmcl_kernelalloc_1_arg;
   VMAccelReturnStatus *result_5;
   VMCLDispatchOp vmcl_dispatch_1_arg;
   VMAccelReturnStatus *result_6;
   VMCLQueueId vmcl_queueflush_1_arg;
   VMAccelReturnStatus *result_7;
   VMCLKernelId vmcl_kerneldestroy_1_arg;
   VMAccelReturnStatus *result_8;
   VMCLQueueId vmcl_queuedestroy_1_arg;
   VMAccelReturnStatus *result_9;
   VMCLContextId vmcl_contextdestroy_1_arg;
   VMAccelReturnStatus *result_10;
   VMAccelId vmaccelmgr_free_1_arg;
   VMCLKernelArgDesc *kernelArgs = NULL;
   unsigned int *surfaceIds = NULL;
   unsigned int numArguments = sizeof...(args);
   unsigned int contextId = 0;
   unsigned int queueId = 0;
   unsigned int kernelId = 0;
   unsigned int i;

   /*
    * Query an accelerator manager for the Compute Resource.
    */
   char host[4 * VMACCEL_MAX_LOCATION_SIZE];

   memset(&vmaccelmgr_alloc_1_arg, 0, sizeof(vmaccelmgr_alloc_1_arg));
   vmaccelmgr_alloc_1_arg.type = VMACCEL_COMPUTE_ACCELERATOR;
   vmaccelmgr_alloc_1_arg.capacity.megaFlops = 1;
   result_1 = vmaccelmgr_alloc_1(&vmaccelmgr_alloc_1_arg, accel.get_manager());
   if (result_1 != NULL) {
      if (!VMAccelAddressOpaqueAddrToString(
             &result_1->VMAccelAllocateReturnStatus_u.ret->desc.parentAddr,
             host, sizeof(host))) {
         Warning("%s: Unable to resolve Compute Accelerator host\n",
                 __FUNCTION__);
         vmaccelmgr_free_1_arg =
            result_1->VMAccelAllocateReturnStatus_u.ret->id;
         vmaccelmgr_free_1(&vmaccelmgr_free_1_arg, accel.get_manager());
         return VMACCEL_FAIL;
      }
   } else {
      Warning("%s: Unable to connect to Accelerator manager %p, attempting "
              "direct connect to VMCL localhost\n",
              __FUNCTION__, accel.get_manager());
      strcpy(&host[0], "127.0.0.1");
   }

   clnt = clnt_create(host, VMCL, VMCL_VERSION, "udp");
   if (clnt == NULL) {
      Warning("%s: Unable to instantiate VMCL for host = %s\n", __FUNCTION__,
              host);
      vmaccelmgr_free_1_arg = result_1->VMAccelAllocateReturnStatus_u.ret->id;
      vmaccelmgr_free_1(&vmaccelmgr_free_1_arg, accel.get_manager());
      return VMACCEL_FAIL;
   }

   /*
    * Allocate a context from the Compute Accelerator.
    */
   memset(&vmcl_contextalloc_1_arg, 0, sizeof(vmcl_contextalloc_1_arg));
   vmcl_contextalloc_1_arg.accelId = 0;
   vmcl_contextalloc_1_arg.clientId = contextId;
   vmcl_contextalloc_1_arg.selectionMask = VMACCEL_CPU_MASK | VMACCEL_GPU_MASK;
   vmcl_contextalloc_1_arg.requiredCaps = 0;
   //      (spirv != NULL) ? VMCL_SPIRV_1_0_CAP : 0;

   result_2 = vmcl_contextalloc_1(&vmcl_contextalloc_1_arg, clnt);
   if (result_2 == NULL) {
      Warning("%s: Unable to create a VMCL context\n", __FUNCTION__);
      if (result_1 != NULL) {
         vmaccelmgr_free_1_arg =
            result_1->VMAccelAllocateReturnStatus_u.ret->id;
         vmaccelmgr_free_1(&vmaccelmgr_free_1_arg, accel.get_manager());
      }
      return VMACCEL_FAIL;
   }

   /*
    * Allocate a queue from the Compute Accelerator.
    */
   memset(&vmcl_queuealloc_1_arg, 0, sizeof(vmcl_queuealloc_1_arg));
   vmcl_queuealloc_1_arg.client.cid = contextId;
   vmcl_queuealloc_1_arg.client.id = queueId;
   vmcl_queuealloc_1_arg.desc.flags = VMACCEL_QUEUE_ON_DEVICE_FLAG;
   vmcl_queuealloc_1_arg.desc.size = -1; /* Unbounded? */

   result_3 = vmcl_queuealloc_1(&vmcl_queuealloc_1_arg, clnt);
   if (result_3 == NULL) {
      Warning("%s: Unable to create a VMCL queue\n", __FUNCTION__);
      vmcl_contextdestroy_1_arg = contextId;
      vmcl_contextdestroy_1(&vmcl_contextdestroy_1_arg, clnt);
      if (result_1 != NULL) {
         vmaccelmgr_free_1_arg =
            result_1->VMAccelAllocateReturnStatus_u.ret->id;
         vmaccelmgr_free_1(&vmaccelmgr_free_1_arg, accel.get_manager());
      }
      return VMACCEL_FAIL;
   }

#if DEBUG_COMPUTE_OPERATION
   /*
    * Setup Compute Kernel.
    */
   for (auto const &k : kernel) {
      Log("%s: Kernel Architecture - %u\n", __FUNCTION__, k.first);
      Log("%s:\n%s\n", __FUNCTION__, (char *)k.second.get_ptr());
   }

   /*
    * Setup Compute Kernel arguments up-front, since the memory for surfaces
    * will be the scarce resource.
    */
   Log("%s: Function Name = %s\n", __FUNCTION__, kernelFunction.c_str());
   Log("%s: Number of Arguments = %u\n", __FUNCTION__, numArguments);
#endif

   kernelArgs =
      (VMCLKernelArgDesc *)malloc(sizeof(VMCLKernelArgDesc) * numArguments);
   surfaceIds = (unsigned int *)malloc(sizeof(unsigned int) * numArguments);

   if (kernelArgs == NULL || surfaceIds == NULL) {
      Warning("%s: Unable to create a kernel arguments and surface ids\n",
              __FUNCTION__);
      vmcl_queuedestroy_1_arg = vmcl_queuealloc_1_arg.client;
      vmcl_queuedestroy_1(&vmcl_queuedestroy_1_arg, clnt);
      vmcl_contextdestroy_1_arg = contextId;
      vmcl_contextdestroy_1(&vmcl_contextdestroy_1_arg, clnt);
      if (result_1 != NULL) {
         vmaccelmgr_free_1_arg =
            result_1->VMAccelAllocateReturnStatus_u.ret->id;
         vmaccelmgr_free_1(&vmaccelmgr_free_1_arg, accel.get_manager());
      }
      return VMACCEL_FAIL;
   }

   /*
    * Zero out the arguments to ensure variable sized members are not
    * encoded by the RPC stack.
    */
   memset(&kernelArgs[0], 0, sizeof(VMCLKernelArgDesc) * numArguments);

   for (i = 0; i < numArguments; i++) {
      surfaceIds[i] = VMACCEL_INVALID_ID;
   }

   if (prepareComputeArgs(clnt, contextId, queueId, kernelArgs, surfaceIds, 0,
                          args...)) {
      /*
       * Allocate a Compute Kernel given the source/binaries provided.
       */
      memset(&vmcl_kernelalloc_1_arg, 0, sizeof(vmcl_kernelalloc_1_arg));
      vmcl_kernelalloc_1_arg.client.cid = contextId;
      vmcl_kernelalloc_1_arg.client.id = kernelId;
      // Include the NULL termination of the string.
      vmcl_kernelalloc_1_arg.kernelName.kernelName_len =
         kernelFunction.length() + 1;
      vmcl_kernelalloc_1_arg.kernelName.kernelName_val =
         (char *)kernelFunction.c_str();
      vmcl_kernelalloc_1_arg.language = kernelType;
      vmcl_kernelalloc_1_arg.source.source_len =
         kernel.find(VMCL_IR_NATIVE)->second.get_size();
      vmcl_kernelalloc_1_arg.source.source_val =
         (char *)kernel.find(VMCL_IR_NATIVE)->second.get_ptr();

      result_4 = vmcl_kernelalloc_1(&vmcl_kernelalloc_1_arg, clnt);
      if (result_4 != NULL) {
         /*
          * Execute the compute kernel
          */
         memset(&vmcl_dispatch_1_arg, 0, sizeof(vmcl_dispatch_1_arg));
         vmcl_dispatch_1_arg.queue.cid = contextId;
         vmcl_dispatch_1_arg.queue.id = queueId;
         vmcl_dispatch_1_arg.kernel.id = kernelId;
         vmcl_dispatch_1_arg.dimension = 1;
         vmcl_dispatch_1_arg.globalWorkOffset.globalWorkOffset_len =
            computeTopology.get_num_dimensions();
         vmcl_dispatch_1_arg.globalWorkOffset.globalWorkOffset_val =
            (u_int *)computeTopology.get_global_offsets();
         vmcl_dispatch_1_arg.globalWorkSize.globalWorkSize_len =
            computeTopology.get_num_dimensions();
         vmcl_dispatch_1_arg.globalWorkSize.globalWorkSize_val =
            (u_int *)computeTopology.get_global_sizes();
         vmcl_dispatch_1_arg.localWorkSize.localWorkSize_len =
            computeTopology.get_num_dimensions();
         vmcl_dispatch_1_arg.localWorkSize.localWorkSize_val =
            (u_int *)computeTopology.get_local_sizes();
         vmcl_dispatch_1_arg.args.args_len = numArguments;
         vmcl_dispatch_1_arg.args.args_val = &kernelArgs[0];

         result_5 = vmcl_dispatch_1(&vmcl_dispatch_1_arg, clnt);
         if (result_5 != NULL) {
            vmcl_queueflush_1_arg = vmcl_queuealloc_1_arg.client;

            result_6 = vmcl_queueflush_1(&vmcl_queueflush_1_arg, clnt);
            if (result_6 != NULL) {
               /*
                * Quiesce the Compute Kernel arguments and retrieve the data.
                */
               quiesceComputeArgs(clnt, contextId, queueId, kernelArgs,
                                  surfaceIds, 0, args...);
            }
         }
      }
   }

   free(kernelArgs);
   free(surfaceIds);

   /*
    * Destroy the Compute resources.
    */
   vmcl_kerneldestroy_1_arg = vmcl_kernelalloc_1_arg.client;

   result_7 = vmcl_kerneldestroy_1(&vmcl_kerneldestroy_1_arg, clnt);
   if (result_7 == NULL) {
      Warning("%s: Unable to destroy kernel id = %u\n", __FUNCTION__,
              vmcl_kerneldestroy_1_arg.id);
   }

   vmcl_queuedestroy_1_arg = vmcl_queuealloc_1_arg.client;

   result_8 = vmcl_queuedestroy_1(&vmcl_queuedestroy_1_arg, clnt);
   if (result_8 == NULL) {
      Warning("%s: Unable to destroy queue id = %u\n", __FUNCTION__,
              vmcl_queuedestroy_1_arg.id);
   }

   vmcl_contextdestroy_1_arg = contextId;
   result_9 = vmcl_contextdestroy_1(&vmcl_contextdestroy_1_arg, clnt);
   if (result_9 == NULL) {
      Warning("%s: Unable to destroy context id = %u\n", __FUNCTION__,
              vmcl_contextdestroy_1_arg);
   }

   clnt_destroy(clnt);

   if (result_1 != NULL) {
      vmaccelmgr_free_1_arg = result_1->VMAccelAllocateReturnStatus_u.ret->id;
      result_10 = vmaccelmgr_free_1(&vmaccelmgr_free_1_arg, clnt);
      if (result_10 == NULL) {
         Warning("%s: Unable to free compute resource = %u\n", __FUNCTION__,
                 vmaccelmgr_free_1_arg);
      }
   }

   return VMACCEL_SUCCESS;
}
};

#endif /* defined _VMACCEL_COMPUTE_HPP_ */
