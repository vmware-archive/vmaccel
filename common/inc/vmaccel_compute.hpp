/******************************************************************************

Copyright (c) 2019-2020 VMware, Inc.
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

extern "C" {
#include "vmaccel_defs.h"
#include "vmcl_defs.h"
#if VMACCEL_LOCAL
#include "vmcl_local.h"
#include "vmcl_ops.h"
#else
#include "vmcl_rpc.h"
#endif
#include "vmaccel_utils.h"
}

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace vmaccel {


/**
 * VMCLContext structure.
 *
 * Bare minimum information for transferring surface contents and interfacing
 * with VMCL.
 */
class clcontext : public context {

public:
   /**
    * Constructor.
    */
   clcontext(std::shared_ptr<accelerator> &a, unsigned int megaFlops,
             unsigned int selectionMask, unsigned int numSubDevices,
             unsigned int requiredCaps)
      : vmaccel::context(a, VMACCEL_COMPUTE_ACCELERATOR_MASK,
                         a->get_max_ref_objects()) {
      LOG_ENTRY(("clcontext::Constructor(a=%p) {\n", a.get()));
      accelId = VMACCEL_INVALID_ID;
      contextId = VMACCEL_INVALID_ID;

      VMAccelStatusCodeEnum ret = (VMAccelStatusCodeEnum)alloc(
         megaFlops, selectionMask, numSubDevices, requiredCaps);

      if (ret != VMACCEL_SUCCESS) {
         throw exception(ret, "Unable to allocate context...\n");
      }

      LOG_EXIT(("} clcontext::Constructor\n"));
   }

   /**
    * Destructor.
    */
   ~clcontext() {
      LOG_ENTRY(("clcontext::Destructor {\n"));
      destroy();
      LOG_EXIT(("} clcontext::Destructor\n"));
   }

   /**
    * Copy constructor.
    */
   clcontext(const clcontext &obj)
      : vmaccel::context(obj.get_accel(), obj.get_typeMask(),
                         obj.get_accel()->get_max_ref_objects()) {
      LOG_ENTRY(("clcontext::CopyConstructor(obj.clnt=%p, "
                 "obj.accelId=%d, obj.contextId=%d, "
                 "obj.get_typeMask()=%d) {\n",
                 obj.clnt, obj.accelId, obj.contextId, obj.get_typeMask()));
      clnt = obj.clnt;
      accelId = obj.accelId;
      contextId = obj.contextId;
      LOG_EXIT(("} clcontext::CopyConstructor\n"));
   }

   /**
    * alloc_surface
    *
    * Makes a surface resident for the context.
    */
   bool alloc_surface(ref_object<surface> surf) {
      VMAccelSurfaceAllocateReturnStatus *result_1;
      VMCLSurfaceAllocateDesc vmcl_surfacealloc_1_arg;

      if (is_resident(surf->get_id())) {
         return true;
      }

#if DEBUG_PERSISTENT_SURFACES
      VMACCEL_LOG("%s: Allocating surface %d\n", __FUNCTION__, id);
#endif

      vmcl_surfacealloc_1_arg.client.cid = get_contextId();
      vmcl_surfacealloc_1_arg.client.accel.id = surf->get_id();
      vmcl_surfacealloc_1_arg.desc = surf->get_desc();

      assert(vmcl_surfacealloc_1_arg.desc.format == VMACCEL_FORMAT_R8_TYPELESS);

      result_1 = vmcl_surfacealloc_1(&vmcl_surfacealloc_1_arg, get_client());

      if (result_1 == NULL) {
         VMACCEL_WARNING("%s: Unable to allocate surface %d for context %d.\n",
                         __FUNCTION__, surf->get_id(), get_contextId());
         return false;
      }

      vmaccel_xdr_free((xdrproc_t)xdr_VMAccelSurfaceAllocateReturnStatus,
                       (caddr_t)result_1);

      set_residency(surf->get_id(), true);

      return true;
   }

   /**
    * upload_surface
    *
    * Update a resident surface.
    */
   bool upload_surface(ref_object<surface> surf) {
      VMAccelSurfaceMapReturnStatus *result_2;
      VMCLSurfaceMapOp vmcl_surfacemap_1_arg;
      VMAccelReturnStatus *result_3;
      VMCLSurfaceUnmapOp vmcl_surfaceunmap_1_arg;

#if DEBUG_SURFACE_CONSISTENCY
      surf->log_consistency();
#endif

/*
 * Detect if there are any updates from the application for this surface.
 */
#if !DEBUG_FORCE_SURFACE_CONSISTENCY
      if (surf->is_consistent(get_contextId())) {
         return true;
      }
#else
      VMACCEL_LOG("%s: Forcing update of surface %d\n", __FUNCTION__,
                  surf->get_id());
#endif

      memset(&vmcl_surfacemap_1_arg, 0, sizeof(vmcl_surfacemap_1_arg));
      vmcl_surfacemap_1_arg.queue.cid = get_contextId();
      vmcl_surfacemap_1_arg.queue.id = surf->get_queue_id();
      vmcl_surfacemap_1_arg.op.surf.id = surf->get_id();
      vmcl_surfacemap_1_arg.op.size.x = surf->get_desc().width;
      vmcl_surfacemap_1_arg.op.size.y = surf->get_desc().height;
      vmcl_surfacemap_1_arg.op.mapFlags =
         VMACCEL_MAP_READ_FLAG | VMACCEL_MAP_WRITE_FLAG;

      result_2 = vmcl_surfacemap_1(&vmcl_surfacemap_1_arg, get_client());

      if (result_2 != NULL &&
          result_2->VMAccelSurfaceMapReturnStatus_u.ret->status ==
             VMACCEL_SUCCESS) {
         void *ptr = result_2->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_val;

         assert(result_2->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_len ==
                vmcl_surfacemap_1_arg.op.size.x);

         /*
          * Memory copy the contents into the value
          */
         memcpy(ptr, surf->get_backing().get(),
                vmcl_surfacemap_1_arg.op.size.x);

#if DEBUG_COMPUTE_OPERATION
         for (int i = 0;
              i < vmcl_surfacemap_1_arg.op.size.x / sizeof(unsigned int); i++) {
            VMACCEL_LOG("%s: in_uint32[%d]=%d\n", __FUNCTION__, i,
                        ((unsigned int *)ptr)[i]);
         }
#endif

         memset(&vmcl_surfaceunmap_1_arg, 0, sizeof(vmcl_surfaceunmap_1_arg));
         vmcl_surfaceunmap_1_arg.queue.cid = get_contextId();
         vmcl_surfaceunmap_1_arg.queue.id = surf->get_queue_id();
         vmcl_surfaceunmap_1_arg.op.surf.id = surf->get_id();
         vmcl_surfaceunmap_1_arg.op.ptr.ptr_len =
            vmcl_surfacemap_1_arg.op.size.x;
         vmcl_surfaceunmap_1_arg.op.ptr.ptr_val = (char *)ptr;

         result_3 = vmcl_surfaceunmap_1(&vmcl_surfaceunmap_1_arg, get_client());

         vmaccel_xdr_free((xdrproc_t)xdr_VMAccelSurfaceMapReturnStatus,
                          (caddr_t)result_2);

         if (result_3 == NULL) {
            return false;
         }

         vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus,
                          (caddr_t)result_3);

         surf->set_consistency(get_contextId(), true);
      }

      return true;
   }

   /**
    * download_surface
    *
    * Evict a surface from the context.
    */
   bool download_surface(ref_object<surface> surf) {
      VMAccelSurfaceMapReturnStatus *result_1;
      VMCLSurfaceMapOp vmcl_surfacemap_1_arg;
      VMAccelReturnStatus *result_2;
      VMCLSurfaceUnmapOp vmcl_surfaceunmap_1_arg;

#if DEBUG_SURFACE_CONSISTENCY
      surf->log_consistency();
#endif

      if (surf->get_desc().usage != VMACCEL_SURFACE_USAGE_READONLY) {
         memset(&vmcl_surfacemap_1_arg, 0, sizeof(vmcl_surfacemap_1_arg));
         vmcl_surfacemap_1_arg.queue.cid = get_contextId();
         vmcl_surfacemap_1_arg.queue.id = surf->get_queue_id();
         vmcl_surfacemap_1_arg.op.surf.id = surf->get_id();
         vmcl_surfacemap_1_arg.op.size.x = surf->get_desc().width;
         vmcl_surfacemap_1_arg.op.size.y = surf->get_desc().height;
         vmcl_surfacemap_1_arg.op.mapFlags = VMACCEL_MAP_READ_FLAG;

         assert(surf->get_desc().format == VMACCEL_FORMAT_R8_TYPELESS);

         result_1 = vmcl_surfacemap_1(&vmcl_surfacemap_1_arg, get_client());

         if (result_1 != NULL &&
             result_1->VMAccelSurfaceMapReturnStatus_u.ret->status ==
                VMACCEL_SUCCESS) {
            unsigned int *ptr =
               (unsigned int *)
                  result_1->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_val;

            assert(result_1->VMAccelSurfaceMapReturnStatus_u.ret->ptr.ptr_len ==
                   vmcl_surfacemap_1_arg.op.size.x);

#if DEBUG_COMPUTE_OPERATION
            for (int i = 0;
                 i < vmcl_surfacemap_1_arg.op.size.x / sizeof(unsigned int);
                 i++) {
               VMACCEL_LOG("%s: out_uint32[%d]=%d\n", __FUNCTION__, i,
                           ((unsigned int *)ptr)[i]);
            }
#endif

            memcpy(surf->get_backing().get(), ptr,
                   vmcl_surfacemap_1_arg.op.size.x);

            memset(&vmcl_surfaceunmap_1_arg, 0,
                   sizeof(vmcl_surfaceunmap_1_arg));
            vmcl_surfaceunmap_1_arg.queue.cid = get_contextId();
            vmcl_surfaceunmap_1_arg.queue.id = surf->get_queue_id();
            vmcl_surfaceunmap_1_arg.op.surf.id = surf->get_id();
            vmcl_surfaceunmap_1_arg.op.ptr.ptr_len =
               vmcl_surfacemap_1_arg.op.size.x;
            vmcl_surfaceunmap_1_arg.op.ptr.ptr_val = (char *)ptr;

            result_2 =
               vmcl_surfaceunmap_1(&vmcl_surfaceunmap_1_arg, get_client());

            vmaccel_xdr_free((xdrproc_t)xdr_VMAccelSurfaceMapReturnStatus,
                             (caddr_t)result_1);

            if (result_2 == NULL) {
               return false;
            }

            vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus,
                             (caddr_t)result_2);
         }
      }
#if DEBUG_COMPUTE_OPERATION
      else {
         VMACCEL_LOG("%s: Skipping readback due to read-only flag.\n",
                     __FUNCTION__);
      }
#endif
      return true;
   }

   /**
    * copy_surface
    *
    * Copies the contents of the source surface to the destination surface.
    */
   void copy_surface(VMAccelId qid, ref_object<surface> srcSurf,
                     VMAccelSurfaceRegion srcRegion,
                     ref_object<surface> dstSurf,
                     VMAccelSurfaceRegion dstRegion) {
      VMAccelReturnStatus *result_1;
      VMCLSurfaceCopyOp vmcl_surfacecopy_1_arg;
      VMAccelReturnStatus *result_2;
      VMCLQueueId vmcl_queueflush_1_arg;
      VMAccelId srcId = srcSurf->get_id();
      VMAccelId dstId = dstSurf->get_id();

      if (!is_resident(srcId) || !is_resident(dstId)) {
         return;
      }

      vmcl_surfacecopy_1_arg.queue.cid = get_contextId();
      vmcl_surfacecopy_1_arg.queue.id = qid;
      vmcl_surfacecopy_1_arg.dst.cid = get_contextId();
      vmcl_surfacecopy_1_arg.dst.accel.id = dstId;
      vmcl_surfacecopy_1_arg.src.cid = get_contextId();
      vmcl_surfacecopy_1_arg.src.accel.id = srcId;
      vmcl_surfacecopy_1_arg.op.dstRegion = dstRegion;
      vmcl_surfacecopy_1_arg.op.srcRegion = srcRegion;

      result_1 = vmcl_surfacecopy_1(&vmcl_surfacecopy_1_arg, get_client());

      if (result_1 == NULL) {
         VMACCEL_WARNING("%s: Unable to copy surface %d->%d using context %d\n",
                         __FUNCTION__, srcId, dstId, get_contextId());
      } else {
         if (result_1->VMAccelReturnStatus_u.ret->status != VMACCEL_SUCCESS) {
            VMACCEL_WARNING(
               "%s: Unable to copy surface %d->%d using context %d\n",
               __FUNCTION__, srcId, dstId, get_contextId());
         }
         vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus,
                          (caddr_t)result_1);
      }

      vmcl_queueflush_1_arg.cid = get_contextId();
      vmcl_queueflush_1_arg.id = qid;

      result_2 = vmcl_queueflush_1(&vmcl_queueflush_1_arg, get_client());

      if (result_2 == NULL) {
         VMACCEL_WARNING("%s: Unable to flush context %d\n", __FUNCTION__,
                         get_contextId());
      }

      if (dstSurf->is_consistent(get_contextId())) {
         dstSurf->set_consistency(get_contextId(), false);
      }
   }

   /**
    * Virtual function overrides.
    */
   void destroy_surface(VMAccelId id) {
      VMAccelReturnStatus *result_1;
      VMCLSurfaceId vmcl_surfacedestroy_1_arg;

      if (!is_resident(id)) {
         return;
      }

      memset(&vmcl_surfacedestroy_1_arg, 0, sizeof(vmcl_surfacedestroy_1_arg));
      vmcl_surfacedestroy_1_arg.cid = get_contextId();
      vmcl_surfacedestroy_1_arg.accel.id = id;

#if DEBUG_PERSISTENT_SURFACES
      VMACCEL_LOG("%s: Destroying surface %d\n", __FUNCTION__, id);
#endif

      result_1 =
         vmcl_surfacedestroy_1(&vmcl_surfacedestroy_1_arg, get_client());

      if (result_1 == NULL) {
         VMACCEL_WARNING("%s: Unable to destroy surface %d using context %d\n",
                         __FUNCTION__, id, get_contextId());
      } else {
         vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus,
                          (caddr_t)result_1);
      }

      set_residency(id, false);
   }

/**
 * Accessors.
 */
#if VMACCEL_LOCAL
   struct svc_req *get_client() {
      return NULL;
   }
#else
   CLIENT *get_client() { return clnt; }
#endif

   VMAccelId get_accelId() { return accelId; }

   VMAccelId get_contextId() { return contextId; }

private:
   /**
    * Reservation of a context and an associated queue.
    */
   int alloc(unsigned int megaFlops, unsigned int selectionMask,
             unsigned int numSubDevices, unsigned int requiredCaps) {
      VMAccelAllocateReturnStatus *result_1;
      VMAccelDesc vmaccelmgr_alloc_1_arg;
      VMCLContextAllocateReturnStatus *result_2;
      VMCLContextAllocateDesc vmcl_contextalloc_1_arg;
      VMAccelQueueReturnStatus *result_3;
      VMCLQueueAllocateDesc vmcl_queuealloc_1_arg;
      char host[4 * VMACCEL_MAX_LOCATION_SIZE];
      unsigned int i = 0;

      // Allocate at least one queue
      numSubDevices = MAX(1, numSubDevices);

      strcpy(&host[0], "127.0.0.1");
      accelId = VMACCEL_INVALID_ID;

      if (accel->get_manager() != NULL) {
         memset(&vmaccelmgr_alloc_1_arg, 0, sizeof(vmaccelmgr_alloc_1_arg));
         vmaccelmgr_alloc_1_arg.typeMask = VMACCEL_COMPUTE_ACCELERATOR_MASK;
         vmaccelmgr_alloc_1_arg.capacity.megaFlops = megaFlops;

         result_1 =
            vmaccelmgr_alloc_1(&vmaccelmgr_alloc_1_arg, accel->get_manager());

         if (result_1 != NULL) {
            if (!VMAccel_AddressOpaqueAddrToString(
                   &result_1->VMAccelAllocateReturnStatus_u.ret->desc
                       .parentAddr,
                   host, sizeof(host))) {
               VMAccelId vmaccelmgr_free_1_arg;
               VMACCEL_WARNING(
                  "%s: Unable to resolve Compute Accelerator host\n",
                  __FUNCTION__);
               vmaccelmgr_free_1_arg =
                  result_1->VMAccelAllocateReturnStatus_u.ret->id;
               vmaccelmgr_free_1(&vmaccelmgr_free_1_arg, accel->get_manager());
               vmaccel_xdr_free((xdrproc_t)xdr_VMAccelAllocateReturnStatus,
                                (caddr_t)result_1);
               return VMACCEL_FAIL;
            }
            accelId = result_1->VMAccelAllocateReturnStatus_u.ret->id;
            vmaccel_xdr_free((xdrproc_t)xdr_VMAccelAllocateReturnStatus,
                             (caddr_t)result_1);
         } else {
            VMACCEL_WARNING("%s: Unable to connect to Accelerator manager %p, "
                            "attempting direct connect to VMCL localhost\n",
                            __FUNCTION__, accel->get_manager());
         }
      }

      if (!VMAccel_IsLocal()) {
         clnt = clnt_create(host, VMCL, VMCL_VERSION, "tcp");
         if (clnt == NULL) {
            VMACCEL_WARNING("%s: Unable to instantiate VMCL for host = %s\n",
                            __FUNCTION__, host);
            return VMACCEL_FAIL;
         }

         /*
          * Set a one minute timeout for the protocol. This must be sufficient
          * for any bulk data transfer.
          */
         struct timeval tv;
         tv.tv_sec = 60;
         tv.tv_usec = 0;

         VMACCEL_LOG(
            "vmaccel_compute: Setting protocol timeout to %ld seconds\n",
            tv.tv_sec);

         if (!clnt_control(clnt, CLSET_TIMEOUT, (char *)&tv)) {
            VMACCEL_WARNING("vmaccel_compute: Unable to set timeout..\n");
         }
      }

      /*
       * Allocate a context from the Compute Accelerator.
       */
      memset(&vmcl_contextalloc_1_arg, 0, sizeof(vmcl_contextalloc_1_arg));
      vmcl_contextalloc_1_arg.accelId = 0;
      vmcl_contextalloc_1_arg.clientId = accel->alloc_id();
      vmcl_contextalloc_1_arg.selectionMask = selectionMask;
      vmcl_contextalloc_1_arg.numSubDevices = numSubDevices;
      vmcl_contextalloc_1_arg.requiredCaps = requiredCaps;

      result_2 = vmcl_contextalloc_1(&vmcl_contextalloc_1_arg, get_client());

      if (result_2 == NULL) {
         VMACCEL_WARNING("%s: Unable to create a VMCL context\n", __FUNCTION__);
         accel->release_id(vmcl_contextalloc_1_arg.clientId);
         destroy();
         return VMACCEL_FAIL;
      }

      vmaccel_xdr_free((xdrproc_t)xdr_VMCLContextAllocateReturnStatus,
                       (caddr_t)result_2);

      contextId = vmcl_contextalloc_1_arg.clientId;

      /*
       * Allocate a queue from the Compute Accelerator.
       */
      for (i = 0; i < numSubDevices; i++) {
         memset(&vmcl_queuealloc_1_arg, 0, sizeof(vmcl_queuealloc_1_arg));
         vmcl_queuealloc_1_arg.client.cid = contextId;
         vmcl_queuealloc_1_arg.client.id = i;
         vmcl_queuealloc_1_arg.subDevice = i;
         vmcl_queuealloc_1_arg.desc.flags = VMACCEL_QUEUE_ON_DEVICE_FLAG;
         vmcl_queuealloc_1_arg.desc.size = -1; /* Unbounded? */

         result_3 = vmcl_queuealloc_1(&vmcl_queuealloc_1_arg, get_client());

         if (result_3 == NULL) {
            VMACCEL_WARNING("%s: Unable to create a VMCL queue\n",
                            __FUNCTION__);
            accel->release_id(vmcl_queuealloc_1_arg.client.id);
            destroy();
            return VMACCEL_FAIL;
         }
      }

      this->numSubDevices = numSubDevices;

      vmaccel_xdr_free((xdrproc_t)xdr_VMAccelQueueReturnStatus,
                       (caddr_t)result_3);

      return VMACCEL_SUCCESS;
   }

   /**
    * Forced destroy.
    */
   void destroy() {
      VMAccelReturnStatus *result_1;
      VMCLSurfaceId vmcl_surfacedestroy_1_arg;
      VMAccelReturnStatus *result_2;
      VMCLQueueId vmcl_queuedestroy_1_arg;
      VMAccelReturnStatus *result_3;
      VMCLContextId vmcl_contextdestroy_1_arg;

      LOG_ENTRY(("clcontext::destroy() {\n"));

      if (!VMAccel_IsLocal()) {
         if (clnt == NULL) {
            LOG_EXIT(("} clcontext::destroy\n"));
            return;
         }
      }

      std::map<VMAccelId, ref_object<surface>> surfaces =
         get_accel()->get_surface_database();

      for (auto it = surfaces.begin(); it != surfaces.end(); it++) {
         VMAccelId surfId = it->second->get_id();
         if (is_resident(surfId)) {
            destroy_surface(surfId);
         }
      }

      if (contextId != VMACCEL_INVALID_ID) {
         unsigned int i;
         for (i = 0; i < numSubDevices; i++) {
            vmcl_queuedestroy_1_arg.cid = contextId;
            vmcl_queuedestroy_1_arg.id = i;
            result_2 =
               vmcl_queuedestroy_1(&vmcl_queuedestroy_1_arg, get_client());
            if (result_2 == NULL) {
               VMACCEL_WARNING("%s: Unable to destroy queue id = %u\n",
                               __FUNCTION__, vmcl_queuedestroy_1_arg.id);
            }
         }
         numSubDevices = 0;
      }

      if (contextId != VMACCEL_INVALID_ID) {
         vmcl_contextdestroy_1_arg = contextId;
         result_3 =
            vmcl_contextdestroy_1(&vmcl_contextdestroy_1_arg, get_client());
         if (result_3 == NULL) {
            VMACCEL_WARNING("%s: Unable to destroy context id = %u\n",
                            __FUNCTION__, vmcl_contextdestroy_1_arg);
         }
         contextId = VMACCEL_INVALID_ID;
      }

      if (!VMAccel_IsLocal()) {
         clnt_destroy(clnt);
         clnt = NULL;
      }

      if (accelId != VMACCEL_INVALID_ID) {
         VMAccelId vmaccelmgr_free_1_arg;
         vmaccelmgr_free_1_arg = accelId;
         vmaccelmgr_free_1(&vmaccelmgr_free_1_arg, accel->get_manager());
         accelId = VMACCEL_INVALID_ID;
      }

      LOG_EXIT(("} clcontext::destroy\n"));
   }

   CLIENT *clnt;
   VMAccelId accelId;
   VMAccelId contextId;
   unsigned int numSubDevices;
};

typedef unsigned int VMCLKernelArchitecture;

namespace compute {

/**
 * prepareComputeArgs
 *
 * Recursively prepare Compute Accelerator arguments for consumption by the
 * Compute Kernel. Arguments must be declared in the order as they are
 * declared in the Compute Kernel.
 */
template <typename T>
bool prepareComputeArgs(ref_object<clcontext> &clctx,
                        VMCLKernelArgDesc *kernelArgs, unsigned int *surfaceIds,
                        unsigned int argIndex, T arg) {
   VMAccelSurfaceAllocateReturnStatus *result_1;
   VMCLSurfaceAllocateDesc vmcl_surfacealloc_1_arg;
   VMAccelSurfaceMapReturnStatus *result_2;
   VMCLSurfaceMapOp vmcl_surfacemap_1_arg;
   VMAccelReturnStatus *result_3;
   VMCLSurfaceUnmapOp vmcl_surfaceunmap_1_arg;

#if DEBUG_TEMPLATE_TYPES
   VMACCEL_LOG("%s: argIndex=%u, type=%s, q=%d\n", __FUNCTION__, argIndex,
               typeid(T).name(), arg.get_queue_id());
#endif

   kernelArgs[argIndex].surf.id = VMACCEL_INVALID_ID;
   kernelArgs[argIndex].index = -1;

   memset(&vmcl_surfacealloc_1_arg, 0, sizeof(vmcl_surfacealloc_1_arg));
   vmcl_surfacealloc_1_arg.client.cid = clctx->get_contextId();
   vmcl_surfacealloc_1_arg.client.accel.id = argIndex;
   vmcl_surfacealloc_1_arg.desc.type = VMACCEL_SURFACE_BUFFER;
   vmcl_surfacealloc_1_arg.desc.width = arg.get_size();
   vmcl_surfacealloc_1_arg.desc.format = VMACCEL_FORMAT_R8_TYPELESS;
   vmcl_surfacealloc_1_arg.desc.usage = arg.get_usage();
   vmcl_surfacealloc_1_arg.desc.bindFlags = VMACCEL_BIND_UNORDERED_ACCESS_FLAG;

   result_1 =
      vmcl_surfacealloc_1(&vmcl_surfacealloc_1_arg, clctx->get_client());

   if (result_1 == NULL) {
      VMACCEL_WARNING("%s: Unable to allocate surface %d for context %d\n",
                      __FUNCTION__, argIndex, clctx->get_contextId());
      return false;
   }

   vmaccel_xdr_free((xdrproc_t)xdr_VMAccelSurfaceAllocateReturnStatus,
                    (caddr_t)result_1);

   kernelArgs[argIndex].surf.id = vmcl_surfacealloc_1_arg.client.accel.id;

   memset(&vmcl_surfacemap_1_arg, 0, sizeof(vmcl_surfacemap_1_arg));
   vmcl_surfacemap_1_arg.queue.cid = clctx->get_contextId();
   vmcl_surfacemap_1_arg.queue.id = arg.get_queue_id();
   vmcl_surfacemap_1_arg.op.surf.id = (VMAccelId)argIndex;
   vmcl_surfacemap_1_arg.op.size.x = arg.get_size();
   vmcl_surfacemap_1_arg.op.mapFlags =
      VMACCEL_MAP_READ_FLAG | VMACCEL_MAP_WRITE_FLAG;

   result_2 = vmcl_surfacemap_1(&vmcl_surfacemap_1_arg, clctx->get_client());

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
         VMACCEL_LOG("%s: in_uint32[%d]=%d\n", __FUNCTION__, i,
                     ((unsigned int *)ptr)[i]);
      }
#endif

      memset(&vmcl_surfaceunmap_1_arg, 0, sizeof(vmcl_surfaceunmap_1_arg));
      vmcl_surfaceunmap_1_arg.queue.cid = clctx->get_contextId();
      vmcl_surfaceunmap_1_arg.queue.id = arg.get_queue_id();
      vmcl_surfaceunmap_1_arg.op.surf.id = argIndex;
      vmcl_surfaceunmap_1_arg.op.ptr.ptr_len = vmcl_surfacemap_1_arg.op.size.x;
      vmcl_surfaceunmap_1_arg.op.ptr.ptr_val = (char *)ptr;

      result_3 =
         vmcl_surfaceunmap_1(&vmcl_surfaceunmap_1_arg, clctx->get_client());

      vmaccel_xdr_free((xdrproc_t)xdr_VMAccelSurfaceMapReturnStatus,
                       (caddr_t)result_2);

      if (result_3 == NULL) {
         return false;
      }

      vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus, (caddr_t)result_3);

      kernelArgs[argIndex].index = argIndex;
      kernelArgs[argIndex].type = VMCL_ARG_SURFACE;
      kernelArgs[argIndex].surf.id = vmcl_surfacealloc_1_arg.client.accel.id;

      return true;
   }

   VMACCEL_WARNING("%s: Unable to map surface %d for context %d queue %d\n",
                   __FUNCTION__, argIndex, clctx->get_contextId(),
                   arg.get_queue_id());
   VMACCEL_WARNING("%s:   size=%d usage=%d\n", arg.get_size(), arg.get_usage());

   return false;
}

template <typename T, typename... R>
bool prepareComputeArgs(ref_object<clcontext> &clctx,
                        VMCLKernelArgDesc *kernelArgs, unsigned int *surfaceIds,
                        unsigned int argIndex, T arg, R... args) {
   if (!prepareComputeArgs(clctx, kernelArgs, surfaceIds, argIndex, arg)) {
      return false;
   }
   return prepareComputeArgs(clctx, kernelArgs, surfaceIds, ++argIndex,
                             args...);
}

/**
 * prepareComputeSurfaceArgs
 *
 * Recursively prepare Compute Accelerator arguments for consumption by the
 * Compute Kernel. Arguments must be declared in the order as they are
 * declared in the Compute Kernel.
 */
template <typename... R>
bool prepareComputeSurfaceArgs(ref_object<clcontext> &clctx,
                               VMCLKernelArgDesc *kernelArgs,
                               unsigned int *surfaceIds, unsigned int argIndex,
                               ref_object<surface> arg) {
#if DEBUG_COMPUTE_OPERATION
   VMACCEL_LOG(
      "%s: argIndex=%u, type=ref_object<surface>, id=%d, contextId=%d, "
      "queueId=%d\n",
      __FUNCTION__, argIndex, arg->get_id(), clctx->get_contextId(),
      arg->get_queue_id());
#endif

   kernelArgs[argIndex].surf.id = VMACCEL_INVALID_ID;
   kernelArgs[argIndex].index = -1;

   if (!clctx->alloc_surface(arg)) {
      return false;
   }

   if (clctx->upload_surface(arg)) {
      kernelArgs[argIndex].index = argIndex;
      kernelArgs[argIndex].type = VMCL_ARG_SURFACE;
      kernelArgs[argIndex].surf.id = arg->get_id();
      return true;
   }

   VMACCEL_WARNING("%s: Unable to map surface %d for context %d queue %d\n",
                   __FUNCTION__, arg->get_id(), clctx->get_contextId(),
                   arg->get_queue_id());
   VMACCEL_WARNING("%s:   width=%d height=%d\n", __FUNCTION__,
                   arg->get_desc().width, arg->get_desc().height);

   return false;
}

template <typename... R>
bool prepareComputeSurfaceArgs(ref_object<clcontext> &clctx,
                               VMCLKernelArgDesc *kernelArgs,
                               unsigned int *surfaceIds, unsigned int argIndex,
                               ref_object<surface> arg, R... args) {
   if (!prepareComputeSurfaceArgs(clctx, kernelArgs, surfaceIds, argIndex,
                                  arg)) {
      return false;
   }
   return prepareComputeSurfaceArgs(clctx, kernelArgs, surfaceIds, ++argIndex,
                                    args...);
}

/**
 * quiesceComputeArgs
 *
 * Recursively retrieve Compute Accelerator arguments for consumption by the
 * application. Arguments must be declared in the order as they are declared
 * in the Compute Kernel.
 */
template <typename T>
bool quiesceComputeArgs(ref_object<clcontext> &clctx,
                        VMCLKernelArgDesc *kernelArgs, unsigned int *surfaceIds,
                        unsigned int argIndex, T arg) {
   VMAccelSurfaceMapReturnStatus *result_1;
   VMCLSurfaceMapOp vmcl_surfacemap_1_arg;
   VMAccelReturnStatus *result_2;
   VMCLSurfaceUnmapOp vmcl_surfaceunmap_1_arg;
   VMAccelReturnStatus *result_3;
   VMCLSurfaceId vmcl_surfacedestroy_1_arg;

#if DEBUG_TEMPLATE_TYPES
   VMACCEL_LOG("%s: argIndex = %u, type = %s\n", __FUNCTION__, argIndex,
               typeid(T).name());
#endif

   if (kernelArgs[argIndex].surf.id == VMACCEL_INVALID_ID) {
      return true;
   }

   if (arg.get_usage() != VMACCEL_SURFACE_USAGE_READONLY) {
      memset(&vmcl_surfacemap_1_arg, 0, sizeof(vmcl_surfacemap_1_arg));
      vmcl_surfacemap_1_arg.queue.cid = clctx->get_contextId();
      vmcl_surfacemap_1_arg.queue.id = kernelArgs[argIndex].queue.id;
      vmcl_surfacemap_1_arg.op.surf.id =
         (VMAccelId)kernelArgs[argIndex].surf.id;
      vmcl_surfacemap_1_arg.op.size.x = arg.get_size();
      vmcl_surfacemap_1_arg.op.mapFlags = VMACCEL_MAP_READ_FLAG;

      result_1 = vmcl_surfacemap_1(&vmcl_surfacemap_1_arg, clctx->get_client());

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
            VMACCEL_LOG("%s: out_uint32[%d]=%d\n", __FUNCTION__, i,
                        ((unsigned int *)ptr)[i]);
         }
#endif

         memcpy(arg.get_ptr(), ptr, vmcl_surfacemap_1_arg.op.size.x);

         memset(&vmcl_surfaceunmap_1_arg, 0, sizeof(vmcl_surfaceunmap_1_arg));
         vmcl_surfaceunmap_1_arg.queue.cid = clctx->get_contextId();
         vmcl_surfaceunmap_1_arg.queue.id = kernelArgs[argIndex].queue.id;
         vmcl_surfaceunmap_1_arg.op.surf.id = kernelArgs[argIndex].surf.id;
         vmcl_surfaceunmap_1_arg.op.ptr.ptr_len =
            vmcl_surfacemap_1_arg.op.size.x;
         vmcl_surfaceunmap_1_arg.op.ptr.ptr_val = (char *)ptr;

         result_2 =
            vmcl_surfaceunmap_1(&vmcl_surfaceunmap_1_arg, clctx->get_client());

         vmaccel_xdr_free((xdrproc_t)xdr_VMAccelSurfaceMapReturnStatus,
                          (caddr_t)result_1);

         if (result_2 == NULL) {
            return false;
         }

         vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus,
                          (caddr_t)result_2);
      }
   }

   memset(&vmcl_surfacedestroy_1_arg, 0, sizeof(vmcl_surfacedestroy_1_arg));
   vmcl_surfacedestroy_1_arg.cid = clctx->get_contextId();
   vmcl_surfacedestroy_1_arg.accel.id = kernelArgs[argIndex].surf.id;

   result_3 =
      vmcl_surfacedestroy_1(&vmcl_surfacedestroy_1_arg, clctx->get_client());

   if (result_3 == NULL) {
      return false;
   }

   vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus, (caddr_t)result_3);

   return true;
}

template <typename T, typename... R>
bool quiesceComputeArgs(ref_object<clcontext> &clctx,
                        VMCLKernelArgDesc *kernelArgs, unsigned int *surfaceIds,
                        unsigned int argIndex, T arg, R... args) {
   if (!quiesceComputeArgs(clctx, kernelArgs, surfaceIds, argIndex, arg)) {
      return false;
   }
   return quiesceComputeArgs(clctx, kernelArgs, surfaceIds, ++argIndex,
                             args...);
}

/**
 * quiesceComputeSurfaceArgs
 *
 * Recursively retrieve Compute Accelerator arguments for consumption by the
 * application. Arguments must be declared in the order as they are declared
 * in the Compute Kernel.
 */
template <typename T>
bool quiesceComputeSurfaceArgs(ref_object<clcontext> &clctx,
                               VMCLKernelArgDesc *kernelArgs,
                               unsigned int *surfaceIds, unsigned int argIndex,
                               ref_object<surface> arg) {
#if DEBUG_COMPUTE_OPERATION
   VMACCEL_LOG("%s: argIndex=%u, type=ref_object<surface>, id=%d\n",
               __FUNCTION__, argIndex, kernelArgs[argIndex].surf.id);
#endif

   if (kernelArgs[argIndex].surf.id == VMACCEL_INVALID_ID) {
#if DEBUG_COMPUTE_OPERATION
      VMACCEL_WARNING("%s: Kernel argument to nowhere...\n", __FUNCTION__);
#endif
      return true;
   }

   clctx->download_surface(arg);

#if DEBUG_PERSISTENT_SURFACES
   VMACCEL_LOG("%s: Detroying server side persistent surface\n", __FUNCTION__);

   clctx->destroy_surface(kernelArgs[argIndex].surf.id);
#endif

   return true;
}

template <typename T, typename... R>
bool quiesceComputeSurfaceArgs(ref_object<clcontext> &clctx,
                               VMCLKernelArgDesc *kernelArgs,
                               unsigned int *surfaceIds, unsigned int argIndex,
                               T arg, R... args) {
   VMACCEL_LOG("%s: Quiescing surface arg index %d\n", __FUNCTION__, argIndex);
   if (!quiesceComputeSurfaceArgs(clctx, kernelArgs, surfaceIds, argIndex,
                                  arg)) {
      return false;
   }
   return quiesceComputeSurfaceArgs(clctx, kernelArgs, surfaceIds, ++argIndex,
                                    args...);
}

/**
 * VMAccel compute binding encapsulating class.
 *
 * This class is used to encapsulate the binding object for the compute
 * accelerator.
 */

class binding {

public:
   /**
    * Default constructor.
    */
   binding() {}

   /**
    * Constructor.
    *
    * @param bindFlags Bind flags for the object.
    * @param usage Usage flags for the object.
    * @param s Surface object for the binding.
    */
   binding(unsigned int bindFlags, unsigned int usage, ref_object<surface> &s) {
      assert(usage == s->get_desc().usage);
      clbinding = ref_object<vmaccel::binding>(new vmaccel::binding(
         VMACCEL_COMPUTE_ACCELERATOR_MASK, bindFlags, usage, s));
   }

   /**
    * Accessors.
    */
   vmaccel::binding *operator->() { return clbinding.get().get(); }

   operator ref_object<vmaccel::binding> &() { return clbinding; }

private:
   ref_object<vmaccel::binding> clbinding;
};

/**
 * VMAccel compute kernel encapsulating class.
 *
 * This class is used to encapsulate the kernel object for the compute
 * accelerator.
 */

class kernel {

public:
   /**
    * Default constructor.
    */
   kernel() {}

   /**
    * Constructor.
    *
    * @param type Type of kernel handed to the accelerator.
    * @param kernelSource Source byte array for the kernel.
    */
   kernel(unsigned int type, const char *kernelSource) {
      char *kernelStr = new char[strlen(kernelSource) + 1];
      strcpy(kernelStr, kernelSource);
      kernels[VMCL_IR_NATIVE] = ref_object<char>(
         kernelStr, strlen(kernelStr) + 1, VMACCEL_SURFACE_USAGE_READONLY);
   }

   /**
    * Accessors.
    */
   operator std::map<unsigned int, ref_object<char>> &() { return kernels; }

private:
   std::map<unsigned int, ref_object<char>> kernels;
};

/**
 * VMAccel compute_operation object class.
 *
 * Operation Objects represent the lifetime of an operation, from submission
 * to completion/notification. An operation is associated to an Accelerator
 * Context and will keep a context alive until the operation completion has
 * been acknowledged. This allows for transparent abstraction of a queue extent
 * when handling asynchronous operations and their content across multiple
 * Accelerator resources. Referenced Objects (e.g. Surfaces) will also be kept
 * alive for the lifetime of the Operation Object, thus avoiding faults due to
 * freed objects that are in-flight.
 */
class operation : private vmaccel::operation {

public:
   /**
    * Default constructor.
    */
   operation() : vmaccel::operation() {
      LOG_ENTRY(("compute::operation::Constructor() {\n"));
      prepared = false;
      dispatched = false;
      quiesced = false;

      kernelArgs = NULL;
      surfaceIds = NULL;
      LOG_EXIT(("} compute::operation::Constructor\n"));
   }

   /**
    * Destructor.
    */
   ~operation() {
      LOG_ENTRY(("compute::operation::Destructor() prepared=%d, dispatched=%d, "
                 "quiesced=%d {\n",
                 prepared, dispatched, quiesced));
      finish();

      if (kernelArgs) {
         free(kernelArgs);
      }
      if (surfaceIds) {
         free(surfaceIds);
      }
      LOG_EXIT(("} prepared=%d, dispatched=%d, quiesced=%d "
                "compute::operation::Destructor\n",
                prepared, dispatched, quiesced));
   }

   /**
    * prepare
    *
    * Prepares the operation's state.
    */
   template <class... B>
   void
   prepare(ref_object<clcontext> &c, const unsigned int subDev,
           const VMCLKernelLanguageType type,
           const std::map<VMCLKernelArchitecture, vmaccel::ref_object<char>> &k,
           const std::string &func, const vmaccel::work_topology &topology,
           B &... args) {
      if (prepared) {
         VMACCEL_WARNING("%s: Operation already prepared...\n", __FUNCTION__);
         return;
      }
      std::string tag("COMPUTE");
      vmaccel::operation::prepare<B...>(VMACCEL_COMPUTE_ACCELERATOR_MASK, tag,
                                        args...);
      clctx = c;
      subDevice = subDev;
      kernelType = type;
      kernel = k;
      kernelFunction = func;
      computeTopology = topology;
      prepared = true;
   }

   /**
    * dispatch
    *
    * Dispatch function.
    */
   int dispatch() {
      VMCLKernelAllocateReturnStatus *result_1;
      VMCLKernelAllocateDesc vmcl_kernelalloc_1_arg;
      VMAccelReturnStatus *result_2;
      VMCLDispatchOp vmcl_dispatch_1_arg;
      VMAccelReturnStatus *result_3;
      VMCLQueueId vmcl_queueflush_1_arg;
      VMAccelReturnStatus *result_4;
      VMCLKernelId vmcl_kerneldestroy_1_arg;
      unsigned int numArguments = bindings.size();
      unsigned int contextId = clctx->get_contextId();
      unsigned int queueId = subDevice;
      unsigned int kernelId = 0;
      unsigned int i;

      if (!prepared) {
         return VMACCEL_FAIL;
      }

      if (dispatched) {
         return VMACCEL_SUCCESS;
      }

#if DEBUG_COMPUTE_OPERATION
      /*
       * Setup Compute Kernel.
       */
      for (auto const &k : kernel) {
         VMACCEL_LOG("%s: Kernel Architecture - %u\n", __FUNCTION__, k.first);
         VMACCEL_LOG("%s:\n%s\n", __FUNCTION__, (char *)k.second.get_ptr());
      }

      /*
       * Setup Compute Kernel arguments up-front, since the memory for surfaces
       * will be the scarce resource.
       */
      VMACCEL_LOG("%s: Function Name = %s\n", __FUNCTION__,
                  kernelFunction.c_str());
      VMACCEL_LOG("%s: Number of Arguments = %u\n", __FUNCTION__, numArguments);
#endif

      kernelArgs =
         (VMCLKernelArgDesc *)malloc(sizeof(VMCLKernelArgDesc) * numArguments);
      surfaceIds = (unsigned int *)malloc(sizeof(unsigned int) * numArguments);

      if (kernelArgs == NULL || surfaceIds == NULL) {
         VMACCEL_WARNING(
            "%s: Unable to create a kernel arguments and surface ids\n",
            __FUNCTION__);
         return VMACCEL_FAIL;
      }

      /*
       * Zero out the arguments to ensure variable sized members are not
       * encoded by the RPC stack.
       */
      memset(&kernelArgs[0], 0, sizeof(VMCLKernelArgDesc) * numArguments);

      for (i = 0; i < numArguments; i++) {
         surfaceIds[i] = VMACCEL_INVALID_ID;
         if (!prepareComputeSurfaceArgs<ref_object<surface>>(
                clctx, kernelArgs, surfaceIds, i, bindings[i]->get_surf())) {
            VMACCEL_WARNING("%s: Unable to prepare compute argument %d\n",
                            __FUNCTION__, i);
         }
      }

      /*
       * Allocate a Compute Kernel given the source/binaries provided.
       */
      memset(&vmcl_kernelalloc_1_arg, 0, sizeof(vmcl_kernelalloc_1_arg));
      vmcl_kernelalloc_1_arg.client.cid = contextId;
      vmcl_kernelalloc_1_arg.client.id = kernelId;
      vmcl_kernelalloc_1_arg.subDevice = subDevice;
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

      result_1 =
         vmcl_kernelalloc_1(&vmcl_kernelalloc_1_arg, clctx->get_client());

      if (result_1 != NULL) {
         /*
          * Execute the compute kernel
          */
         memset(&vmcl_dispatch_1_arg, 0, sizeof(vmcl_dispatch_1_arg));
         vmcl_dispatch_1_arg.queue.cid = contextId;
         vmcl_dispatch_1_arg.queue.id = subDevice;
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

         vmaccel_xdr_free((xdrproc_t)xdr_VMCLKernelAllocateReturnStatus,
                          (caddr_t)result_1);

         result_2 =
            vmcl_dispatch_1(&vmcl_dispatch_1_arg, clctx.get()->get_client());

         if (result_2 != NULL) {
            vmcl_queueflush_1_arg.cid = contextId;
            vmcl_queueflush_1_arg.id = subDevice;

            vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus,
                             (caddr_t)result_2);

            result_3 =
               vmcl_queueflush_1(&vmcl_queueflush_1_arg, clctx->get_client());

            if (result_3 == NULL) {
               VMACCEL_WARNING("%s: Unable to flush queue...\n", __FUNCTION__);
            } else {
               vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus,
                                (caddr_t)result_3);
            }
         }
      }

      /*
       * Destroy the Compute resources.
       */
      vmcl_kerneldestroy_1_arg = vmcl_kernelalloc_1_arg.client;

      result_4 =
         vmcl_kerneldestroy_1(&vmcl_kerneldestroy_1_arg, clctx->get_client());

      if (result_4 == NULL) {
         VMACCEL_WARNING("%s: Unable to destroy kernel id = %u\n", __FUNCTION__,
                         vmcl_kerneldestroy_1_arg.id);
      } else {
         vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus,
                          (caddr_t)result_4);
      }
      dispatched = true;

      return VMACCEL_SUCCESS;
   }

   /**
    * quiesce
    *
    * Quiescing function.
    */
   int quiesce() {
      int i;

      if (quiesced) {
         return VMACCEL_SUCCESS;
      }

      if (!prepared || !dispatched) {
         return VMACCEL_FAIL;
      }

      if (!VMAccel_IsLocal()) {
         if (clctx.get()->get_client() == NULL) {
            VMACCEL_WARNING("%s: Context not initialized...\n", __FUNCTION__);
            return VMACCEL_RESOURCE_UNAVAILABLE;
         }
      }

      /*
       * Quiesce the Compute Kernel arguments and retrieve the data.
       */
      for (i = 0; i < bindings.size(); i++) {
         if (!quiesceComputeSurfaceArgs<ref_object<surface>>(
                clctx, kernelArgs, surfaceIds, i, bindings[i]->get_surf())) {
            VMACCEL_WARNING("%s: Unable to prepare compute argument %d\n",
                            __FUNCTION__, i);
            return VMACCEL_FAIL;
         }
      }

      quiesced = true;

      return VMACCEL_SUCCESS;
   }

   /**
    * finish
    *
    * Finish the operation.
    */
   int finish() {
      if (prepared) {
         if (!dispatched) {
            dispatch();
         }
         if (dispatched && !quiesced) {
            quiesce();
         }
      }

      return VMACCEL_SUCCESS;
   }

private:
   bool prepared;
   bool dispatched;
   bool quiesced;

   ref_object<clcontext> clctx;
   unsigned int subDevice;
   VMCLKernelLanguageType kernelType;
   std::map<VMCLKernelArchitecture, vmaccel::ref_object<char>> kernel;
   std::string kernelFunction;
   vmaccel::work_topology computeTopology;
   VMCLKernelArgDesc *kernelArgs;
   unsigned int *surfaceIds;
};

/**
 * Compute context encapsulating class.
 *
 * This class is used to encapsulate the clcontext, to unify the management
 * of adding a context object to the vmaccel::accelerator. A reference object
 * must be added to vmaccel::accelerator for dereference active contexts
 * during surface destruction.
 */

class context {

public:
   /**
    * Default constructor.
    */
   context() {}

   /**
    * Constructor.
    *
    * @param a Accelerator class used to instantiate the compute operation.
    * @param megaFlops The estimated mega-flops needed for the workload.
    * @param selectionMask The selection mask for the accelerators requested.
    * @param numSubDevices The number of sub-devices to explicitly submit work
    *to.
    * @param requiredCaps The required capabilites for this compute context.
    */
   context(std::shared_ptr<vmaccel::accelerator> &a, unsigned int megaFlops,
           unsigned int selectionMask, unsigned int numSubDevices,
           unsigned int requiredCaps) {
      std::shared_ptr<vmaccel::clcontext> clctxPtr;
      clctxPtr = std::shared_ptr<clcontext>(new clcontext(
         a, megaFlops, selectionMask, numSubDevices, requiredCaps));
      /*
       * Downcast the clcontext for tracking in the accelerator. Tracking in the
       * accelerator class is required for a surface to destroy its remote
       * object upon class destruction.
       */
      std::shared_ptr<vmaccel::context> ctxPtr =
         std::static_pointer_cast<vmaccel::context, clcontext>(clctxPtr);
      ref_object<vmaccel::context> cb(ctxPtr, sizeof(vmaccel::clcontext), 0, 0);
      /*
       * Track the context so vmaccel::surface::destroy can dereference and
       * instantiate destruction of the server side surface.
       */
      a->add_context(clctxPtr->get_contextId(), cb);
      clctx = ref_object<vmaccel::clcontext>(clctxPtr,
                                             sizeof(vmaccel::clcontext), 0, 0);
   }

   /**
    * Destructor.
    */
   ~context() {
      /*
       * Remove the tracked context.
       */
      clctx->get_accel()->remove_context(clctx->get_contextId());
   }

   /**
    * Accessors.
    */
   vmaccel::clcontext *operator->() { return clctx.get().get(); }

   operator ref_object<vmaccel::clcontext> &() { return clctx; }

private:
   ref_object<vmaccel::clcontext> clctx;
};

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
int execute(
   std::shared_ptr<vmaccel::accelerator> &accel, const unsigned int subDevice,
   const VMCLKernelLanguageType kernelType,
   const std::map<VMCLKernelArchitecture, vmaccel::ref_object<char>> &kernel,
   const std::string &kernelFunction,
   const vmaccel::work_topology &computeTopology, ARGTYPES... args) {
   compute::context c;
   VMCLKernelAllocateReturnStatus *result_1;
   VMCLKernelAllocateDesc vmcl_kernelalloc_1_arg;
   VMAccelReturnStatus *result_2;
   VMCLDispatchOp vmcl_dispatch_1_arg;
   VMAccelReturnStatus *result_3;
   VMCLQueueId vmcl_queueflush_1_arg;
   VMAccelReturnStatus *result_4;
   VMCLKernelId vmcl_kerneldestroy_1_arg;
   VMCLKernelArgDesc *kernelArgs = NULL;
   unsigned int *surfaceIds = NULL;
   unsigned int numArguments = sizeof...(args);
   unsigned int kernelId = 0;
   unsigned int i;

   /*
    * Query an accelerator manager for the Compute Resource.
    */
   try {
      c = compute::context(accel, 1, VMACCEL_CPU_MASK | VMACCEL_GPU_MASK,
                           subDevice + 1, 0);
   } catch (const exception &) {
      VMACCEL_WARNING("%s: Unable to instantiate VMCL\n", __FUNCTION__);
      return VMACCEL_FAIL;
   }

#if DEBUG_COMPUTE_OPERATION
   /*
    * Setup Compute Kernel.
    */
   for (auto const &k : kernel) {
      VMACCEL_LOG("%s: Kernel Architecture - %u\n", __FUNCTION__, k.first);
      VMACCEL_LOG("%s:\n%s\n", __FUNCTION__, (char *)k.second.get_ptr());
   }

   /*
    * Setup Compute Kernel arguments up-front, since the memory for surfaces
    * will be the scarce resource.
    */
   VMACCEL_LOG("%s: Function Name = %s\n", __FUNCTION__,
               kernelFunction.c_str());
   VMACCEL_LOG("%s: Number of Arguments = %u\n", __FUNCTION__, numArguments);
#endif

   kernelArgs =
      (VMCLKernelArgDesc *)malloc(sizeof(VMCLKernelArgDesc) * numArguments);
   surfaceIds = (unsigned int *)malloc(sizeof(unsigned int) * numArguments);

   if (kernelArgs == NULL || surfaceIds == NULL) {
      VMACCEL_WARNING(
         "%s: Unable to create a kernel arguments and surface ids\n",
         __FUNCTION__);
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

   if (prepareComputeArgs(c, kernelArgs, surfaceIds, 0, args...)) {
      /*
       * Allocate a Compute Kernel given the source/binaries provided.
       */
      memset(&vmcl_kernelalloc_1_arg, 0, sizeof(vmcl_kernelalloc_1_arg));
      vmcl_kernelalloc_1_arg.client.cid = c->get_contextId();
      vmcl_kernelalloc_1_arg.client.id = kernelId;
      vmcl_kernelalloc_1_arg.subDevice = subDevice;
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

      result_1 = vmcl_kernelalloc_1(&vmcl_kernelalloc_1_arg, c->get_client());

      if (result_1 != NULL) {
         /*
          * Execute the compute kernel
          */
         memset(&vmcl_dispatch_1_arg, 0, sizeof(vmcl_dispatch_1_arg));
         vmcl_dispatch_1_arg.queue.cid = c->get_contextId();
         vmcl_dispatch_1_arg.queue.id = subDevice;
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

         vmaccel_xdr_free((xdrproc_t)xdr_VMCLKernelAllocateReturnStatus,
                          (caddr_t)result_1);

         result_2 = vmcl_dispatch_1(&vmcl_dispatch_1_arg, c->get_client());

         if (result_2 != NULL) {
            vmcl_queueflush_1_arg.cid = c->get_contextId();
            vmcl_queueflush_1_arg.id = subDevice;

            vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus,
                             (caddr_t)result_2);

            result_3 =
               vmcl_queueflush_1(&vmcl_queueflush_1_arg, c->get_client());

            if (result_3 != NULL) {
               /*
                * Quiesce the Compute Kernel arguments and retrieve the data.
                */
               quiesceComputeArgs(c, kernelArgs, surfaceIds, 0, args...);
               vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus,
                                (caddr_t)result_3);
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

   result_4 = vmcl_kerneldestroy_1(&vmcl_kerneldestroy_1_arg, c->get_client());
   if (result_4 == NULL) {
      VMACCEL_WARNING("%s: Unable to destroy kernel id = %u\n", __FUNCTION__,
                      vmcl_kerneldestroy_1_arg.id);
   } else {
      vmaccel_xdr_free((xdrproc_t)xdr_VMAccelReturnStatus, (caddr_t)result_4);
   }

   return VMACCEL_SUCCESS;
}

/**
 * Asynchronous compute operation for compute kernels.
 *
 * Create an Operation Object that will prepare a compute kernel for an
 * available Compute Accelerator. The specified kernel function will be
 * instantiated per-thread, allowing for multi-dimensional partitioning
 * of the working set. The number of concurrent threads is defined as
 * follows:
 *
 *   n = numDimensions
 *   # of threads = n * SUM(globalWorkSizes[0] ... globalWorkSizes[n]) *
 *                  SUM(localWorkSizes[0] ... localWorkSizes[n])
 *
 * The working set comprises of Input and Output to the function, and
 * is specified in the args parameter.
 *
 * @param ctx Context class used to instantiate the compute operation.
 * @param opobj Operation Object class used to store the operation
 *              metadata.
 * @param kernelType Kernel type as defined in vmcl_defs.h, e.g.
 *                   VMCL_SPIRV_1_0.
 * @param kernel A vector of per-architecture kernels.
 * @param kernelFunction Name of the function in the kernel to instantiate.
 * @param computeTopology Topology for the Accelerator's threading model.
 * @param args Packed list of arguments to pass to the kernel function, will be
 *             evaluated in declared order for both input and output.
 * @return Initialized ref_object for the Operation Object.
 */

template <class... ARGTYPES>
int dispatch(
   compute::context &clctx, const unsigned int subDevice,
   ref_object<compute::operation> &opobj,
   const VMCLKernelLanguageType kernelType,
   const std::map<VMCLKernelArchitecture, vmaccel::ref_object<char>> &kernel,
   const std::string &kernelFunction,
   const vmaccel::work_topology &computeTopology, ARGTYPES... args) {
   std::shared_ptr<compute::operation> op(new compute::operation());

   op->prepare<ref_object<vmaccel::binding>>(clctx, subDevice, kernelType,
                                             kernel, kernelFunction,
                                             computeTopology, args...);

   if (opobj.get().get() != NULL) {
#if DEBUG_COMPUTE_OPERATION
      VMACCEL_LOG("%s: dispatching existing\n", __FUNCTION__);
#endif
      opobj->dispatch();
#if DEBUG_COMPUTE_OPERATION
      VMACCEL_LOG("%s: quiescing\n", __FUNCTION__);
#endif
      opobj->quiesce();
   } else {
#if DEBUG_COMPUTE_OPERATION
      VMACCEL_LOG("%s: creating new operation\n", __FUNCTION__);
#endif
   }

   opobj = ref_object<compute::operation>(op, sizeof(compute::operation), 0,
                                          subDevice);

   return VMACCEL_SUCCESS;
}
};
};

#endif /* defined _VMACCEL_COMPUTE_HPP_ */
