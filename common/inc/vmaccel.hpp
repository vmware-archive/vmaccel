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
 * vmaccel.hpp
 *
 * Accelerator C++11 external interface. The interface class will connect to a
 * given Accelerator Manager Server and negotiate Accelerators based upon the
 * requested operation.
 *
 * The classes below use safe types, such as std::string vs. char *, to avoid
 * security issues with unbounded access. To retain an object over the lifetime
 * of an Accelerator operation, std::shared_ptr is required for all arguments.
 */

#ifndef _VMACCEL_HPP_
#define _VMACCEL_HPP_ 1

#include "vmaccel_defs.h"
#include "vmaccel_mgr.h"
#include "vmaccel_rpc.h"
#include "vmaccel_utils.h"
#include "vmaccel_types_address.h"
#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace vmaccel {

class accelerator;
class operation;
template <class T>
class ref_object;
extern vmaccel::ref_object<class vmaccel::operation> noop;

/**
 * VMAccel exception class.
 */
class exception {

public:
   /**
    * Constructors.
    */
   exception() throw();
   exception(const VMAccelStatusCodeEnum code, std::string msg) {
      status = code;
      message = msg;
   }

   /**
    * Copy Constructors.
    */
   exception(const exception &) throw();
   exception &operator=(const exception &) throw();

   /**
    * Destructor.
    */
   virtual ~exception() {}

   /**
    * Describes the exception.
    */
   virtual const char *what() const { return message.c_str(); }

private:
   VMAccelStatusCodeEnum status;
   std::string message;
};

/**
 * VMAccel address encapsulation class.
 */
class address {

public:
   /**
    * Constructor.
    *
    * @param host Host address in a null-terminated string format.
    */
   address(const std::string host) {
      addr.addr.addr_val =
         (char *)malloc(VMACCEL_MAX_LOCATION_SIZE * sizeof(char));
      addr.addr.addr_len = VMACCEL_MAX_LOCATION_SIZE;
      if (!VMAccelAddressStringToOpaqueAddr(host.c_str(), addr.addr.addr_val,
                                            addr.addr.addr_len)) {
         throw exception(VMACCEL_FAIL, "Failed to convert address");
      }
   }

   /**
    * Destructor.
    */
   ~address() { free(addr.addr.addr_val); }

   /**
    * get_accel_addr
    *
    * @return A pointer to the VMAccelAddress.
    */
   const VMAccelAddress *get_accel_addr() const { return &addr; }

   /**
    * get_opaque_addr
    *
    * @return A pointer to the opaque address.
    */
   const char *get_opaque_addr() const { return addr.addr.addr_val; }

   /**
    * get_opaque_addr_size
    *
    * @return A size of the opaque address array.
    */
   size_t get_opaque_addr_size() { return addr.addr.addr_len; }

private:
   VMAccelAddress addr;
};

/**
 * VMAccel object reference encapsulation class.
 */
class object {

public:
   /**
    * Default constructor.
    */
   object() {}

   /**
    * Constructor.
    *
    * Given a pointer and a size of the backing memory, create an Accelerator
    * object.
    */
   object(void *p, size_t s, unsigned int u) {
      ptr = p;
      size = s;
      usage = u;
   }

   /**
    * get_ptr
    *
    * @return A pointer to the backing memory of the object.
    */
   virtual void *get_ptr() { return ptr; }

   /**
    * get_ptr
    *
    * @return A pointer to the backing memory of the object.
    */
   virtual const void *get_ptr() const { return ptr; }

   /**
    * get_size
    *
    * @return The size of the backing memory in bytes.
    */
   virtual const size_t get_size() const { return size; }

   /**
    * get_usage
    *
    * @return The usage of the backing memory, defined by
    *         VMAccelSurfaceUsageEnum.
    */
   virtual const unsigned int get_usage() const { return usage; }

   /**
    * get_fence_id
    *
    * @return The last assigned fence identifier, in VM Accelerator Manager
    *         space.
    */
   virtual const VMAccelId get_fence_id() const { return fenceId; }

   /**
    * set_fence_id
    *
    * @param id The fence identifier in VM Accelerator Manager space.
    */
   virtual void set_fence_id(VMAccelId id) { fenceId = id; }

private:
   void *ptr;
   size_t size;
   unsigned int usage;
   VMAccelId fenceId;
};

/**
 * VMAccel referenced object class.
 *
 * Objects referenced by the Accelerator have longer lifespans than traditional
 * objects within the scope of the CPU. If an Accelerator to access an object,
 * Accelerator stack may lock the object in-place (e.g. for a DMA operation).
 * To avoid an object being freed before an Accelerator has completed pending
 * operations referencing the object, we use a reference count based shared_ptr
 * that will ensure that the backing memory is in place until all references
 * have been destroyed.
 */
template <class T>
class ref_object : public object {

public:
   /**
    * Default constructor.
    */
   ref_object() : object() {}

   /**
    * Constructor.
    *
    * Dereferences a shared_ptr instance to ensure the lifespan of the object
    * within the context of the Accelerator.
    */
   ref_object(T *ptr)
      : object(ptr, sizeof(T), VMACCEL_SURFACE_USAGE_READWRITE) {
      data = std::shared_ptr<T>(ptr);
   }

   ref_object(T *ptr, size_t s)
      : object(ptr, s, VMACCEL_SURFACE_USAGE_READWRITE) {
      data = std::shared_ptr<T>(ptr);
   }

   ref_object(T *ptr, size_t s, unsigned int u) : object(ptr, s, u) {
      data = std::shared_ptr<T>(ptr);
      ;
   }

   ref_object(std::shared_ptr<T> &o, size_t s, unsigned int u)
      : object(o.get(), s, u) {
      data = o;
   }

   /**
    * Copy constructor.
    */
   ref_object(const ref_object &obj)
      : object(obj.data.get(), obj.get_size(), obj.get_usage()) {
      data = obj.data;
   }

   /**
    * Accessor for the encapsulated object.
    */
   T *operator->() const { return data.get(); }

   T &operator[](int index) { return data.get()[index]; }

   std::shared_ptr<T> &get() { return data; }

private:
   std::shared_ptr<T> data;
};

/**
 * VMAccel surface object class.
 *
 * Surfaces are Accelerator objects first and foremost, thus the topology of
 * the contents can be adjusted on the Accelerator for performance reasons.
 * Surfaces identify current content state and topology on the Accelerator,
 * so this class is designed to interact with the surface as if it was an
 * opaque remote object (e.g. Upload and Download). An Upload/Download model
 * was chosen to avoid non-deterministic contention between open mappings
 * and the producers/consumers of the operations, since Surfaces are process
 * global to the client.
 */
class surface : public object {

public:
   /**
    * Default constructor.
    */
   surface() : object() {
      backing = nullptr;
      producer = noop;
   }

   /**
    * Constructor.
    */
   surface(VMAccelSurfaceDesc d) : object() {
      desc = d;
      backing = std::shared_ptr<char>(new char[d.width]);
      producer = noop;
   }

   /**
    * Destructor.
    */
   ~surface() {}

   /**
    * Copy constructor.
    */
   surface(const surface &obj)
      : object(obj.backing.get(), obj.get_size(), obj.get_usage()) {
      backing = obj.backing;
      producer = obj.producer;
   }

   /**
    * Accessors.
    */
   VMAccelSurfaceDesc &get_desc() { return desc; }

   std::shared_ptr<char> &get_backing() { return backing; }

   /**
    * upload
    *
    * Uploads contents of "in" to a given image region within a surface.
    * Uploads to a surface to avoid holding open map operations. Concurrent
    * map operations can lead to contention between two or more threads for
    * the same surface contents.
    *
    * @return VMAccelStatusCodeEnum value.
    */
   template <class E>
   int upload(VMAccelSurfaceRegion imgRegion, ref_object<E> &in) {
      if (desc.format == VMACCEL_FORMAT_R8_TYPELESS && imgRegion.coord.x == 0 &&
          imgRegion.coord.y == 0 && imgRegion.coord.z == 0 &&
          imgRegion.size.x * sizeof(E) == desc.width &&
          imgRegion.size.y == desc.height && imgRegion.size.z == desc.depth) {
         memcpy(backing.get(), in.get_ptr(), MIN(desc.width, in.get_size()));
         return VMACCEL_SUCCESS;
      }
      return VMACCEL_FAIL;
   }

   /**
    * download
    *
    * Downloads from a given image region within a surface to the memory
    * referenced by "out". Downloads from a surface to avoid holding open
    * map operations. Concurrent map operations can lead to contention
    * between two or more threads for the same surface contents.
    *
    * @return VMAccelStatusCodeEnum value.
    */
   template <class E>
   int download(VMAccelSurfaceRegion imgRegion, ref_object<E> &out) {
      if (desc.format == VMACCEL_FORMAT_R8_TYPELESS && imgRegion.coord.x == 0 &&
          imgRegion.coord.y == 0 && imgRegion.coord.z == 0 &&
          imgRegion.size.x * sizeof(E) == desc.width &&
          imgRegion.size.y == desc.height && imgRegion.size.z == desc.depth) {
         memcpy(out.get_ptr(), backing.get(), MIN(desc.width, out.get_size()));
         return VMACCEL_SUCCESS;
      }
      return VMACCEL_FAIL;
   }

   /**
    * set_producer
    *
    * Updates the producer op for this surface.
    */
   void set_producer(ref_object<class operation> &op) {
      /*
       * Depending on the data consistency contract, a swap of the operation
       * may decrement the operation to zero and signal an end of the scope
       * for the variable. At the end of the scope, the application will wait
       * for the operation to complete and update all output bindings with
       * the latest content.
       */
      producer = op;
   }

private:
   VMAccelSurfaceDesc desc;
   std::shared_ptr<char> backing;
   ref_object<class operation> producer;
};

/**
 * VMAccel context object class.
 *
 * Context Objects represent the persistent state for a given group of
 * operations. As long as the Context Object is live, the Accelerator state
 * associated with the context instance is retained. By defining a context's
 * lifetime, through the variable scope for a Context Object, disjoint
 * workloads can be coalesced to optimal placement based upon an
 * application's usage.
 */
class context : public object {

public:
   /**
    * Default constructor.
    */
   context() : object() {}

   context(const std::shared_ptr<accelerator> &a, VMAccelResourceType t)
      : object() {
      accel = a;
      type = t;
   }

   /**
    * Accessor functions.
    */
   const std::shared_ptr<accelerator> &get_accel() const { return accel; }

   const VMAccelResourceType get_type() const { return type; }

protected:
   std::shared_ptr<accelerator> accel;
   VMAccelResourceType type;
};

/**
 * VMAccel binding class.
 *
 * Bindings represent the relationship between a context's persistent state
 * and a surface. Each binding is configured to attach a surface to a point
 * in the Accelerator's pipeline with the desired VMAccelSurfaceUsage.
 */
class binding {

public:
   /**
    * Constructor.
    */
   binding(VMAccelResourceType type, VMAccelSurfaceBindFlags bindFlags,
           VMAccelSurfaceUsage bindUsage, ref_object<surface> &target) {
      accel = type;
      flags = bindFlags;
      usage = bindUsage;
      surf = target;
   }

   /**
    * get_flags
    *
    * Retrieves the bind flags for the binding.
    */
   VMAccelSurfaceBindFlags get_flags() { return flags; }

   /**
    * get_usage
    *
    * Retrieves the usage for the binding.
    */
   VMAccelSurfaceUsage get_usage() { return usage; }

   /**
    * get_surf
    *
    * Retrieves the surf object for the binding.
    */
   ref_object<surface> &get_surf() { return surf; }

private:
   VMAccelResourceType accel;
   VMAccelSurfaceBindFlags flags;
   VMAccelSurfaceUsage usage;
   ref_object<surface> surf;
};


/**
 * VMAccel operation object class.
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
class operation : public object {

public:
   /**
    * Helper function for populating bindings vector.
    */
   template <typename T>
   void prepareArgs(std::vector<ref_object<binding>> &b, T &arg) {
      b.push_back(arg);
   }

   template <typename T, typename... B>
   void prepareArgs(std::vector<ref_object<binding>> &b, T arg, B &... args) {
      prepareArgs<T>(b, arg);
      prepareArgs<B...>(b, args...);
   }

   /**
    * Default constructor.
    */
   operation() : object() {
      ctxType = VMACCEL_NONE;
      tag = "NOP";
   }

   /**
    * prepare
    *
    * Prepares the operation.
    */
   template <class... B>
   void prepare(VMAccelResourceType type, std::string &t, B &... args) {
      ctxType = type;
      tag = t;
      prepareArgs<B...>(bindings, args...);
   }

   /**
    * If an Operation Object's association is reassigned, then complete the
    * operation and update all Referenced Objects.
    */

protected:
   /**
    * Type of context used for the operation.
    */
   VMAccelResourceType ctxType;

   /**
    * Tag for the operation.
    */
   std::string tag;

   /**
    * Hold a binding for each argument to keep the surface object alive as long
    * as the Operation Object is alive.
    */
   std::vector<ref_object<binding>> bindings;
};

/**
 * VMAccel work topology descriptor class.
 *
 * Describes the accelerator topology requested by the working set. For
 * example, a matrix multiply can be parallelized as a multi-threaded workload
 * when each element in the matrix is processed by a single compute kernel
 * thread instance. This is accomplished by creating a topology that reflects
 * the number of dimensions for the matrix, the size of each dimension is size
 * of the matrix for that dimension, and adjusting the compute kernel to index
 * into the the matrices with a thread id.
 */
class work_topology {

public:
   /**
    * Default constructor.
    */
   work_topology() {}

   /**
    * Constructor.
    *
    * Defines a work topology for the accelerator.
    *
    * @param gOffsets Work-global offset for each dimension in the topology.
    * @param gSizes Work-global size of each dimension in the topology.
    * @param lSizes Work-local size of each dimension in the topology.
    */
   work_topology(const std::vector<unsigned int> gOffsets,
                 const std::vector<unsigned int> gSizes,
                 const std::vector<unsigned int> lSizes) {
      if (gOffsets.size() != gSizes.size() ||
          gOffsets.size() != lSizes.size() || gSizes.size() != lSizes.size()) {
         throw exception(VMACCEL_FAIL,
                         "Unequal dimensions supplied for topology");
      }
      globalOffsets = gOffsets;
      globalSizes = gSizes;
      localSizes = lSizes;
   }

   /**
    * get_num_dimensions
    *
    * @return The number of dimensions assuming equally sized vectors.
    */
   unsigned int get_num_dimensions() const { return globalOffsets.size(); }

   /**
    * get_attribute_size
    *
    * @return The size of the arrays visible to the caller, in bytes.
    */
   unsigned int get_attribute_size() const {
      return get_num_dimensions() * sizeof(unsigned int);
   }

   /**
    * get_global_offsets
    *
    * @return A pointer to the array of work-global offsets per-dimension.
    */
   const unsigned int *get_global_offsets() const { return &globalOffsets[0]; }

   /**
    * get_global_work_sizes
    *
    * @return A pointer to the array of work-global sizes per-dimension.
    */
   const unsigned int *get_global_sizes() const { return &globalSizes[0]; }

   /**
    * get_local_work_sizes
    *
    * @return A pointer to the array of work-local sizes per-dimension.
    */
   const unsigned int *get_local_sizes() const { return &localSizes[0]; }

private:
   std::vector<unsigned int> globalOffsets;
   std::vector<unsigned int> globalSizes;
   std::vector<unsigned int> localSizes;
};

/**
 * VMAccel Accelerator interface class.
 */
class accelerator {

public:
   /**
    * Constructor.
    *
    * Opens a connection to the Accelerator Management server.
    *
    * @param addr Specifies the address of the Accelerator Managment server,
    *             in VMAccelAddress format.
    */
   accelerator(const address &mgr, int maxSurfaces = VMACCEL_MAX_SURFACES) {
      char host[256];
      if (VMAccelAddressOpaqueAddrToString(mgr.get_accel_addr(), host,
                                           sizeof(host))) {
         Log("%s: Connecting to Accelerator manager %s\n", __FUNCTION__, host);
         mgrClnt = clnt_create(host, VMACCELMGR, VMACCELMGR_VERSION, "udp");
         if (mgrClnt == NULL) {
            Warning("Failed to create VMAccelMgr client.");
         }
         Log("%s: mgrClient = %p\n", __FUNCTION__, mgrClnt);
      }
      surfaceIds = IdentifierDB_Alloc(maxSurfaces);
      if (surfaceIds == NULL) {
         clnt_destroy(mgrClnt);
         throw exception(VMACCEL_FAIL,
                         "Failed to create context identifier database.");
      }
   }

   /**
    * Destructor.
    *
    * Closes the connection to the Accelerator Management server.
    */
   ~accelerator() {
      if (surfaceIds != NULL) {
         IdentifierDB_Free(surfaceIds);
         surfaceIds = NULL;
      }

      if (mgrClnt != NULL) {
         clnt_destroy(mgrClnt);
         mgrClnt = NULL;
      }
   }

   /**
    * get_manager
    *
    * @return A pointer to the manager client.
    */
   CLIENT *get_manager() { return mgrClnt; }

   /**
    * alloc_surface
    *
    * Allocates a surface given surface descriptor.
    *
    * @return A valid accelerator id for this process if available,
    *         INVALID otherwise.
    */
   VMAccelId alloc_surface(VMAccelSurfaceDesc desc) {
      VMAccelId ret = VMACCEL_INVALID_ID;
      if (IdentifierDB_AllocId(surfaceIds, &ret)) {
         try {
            std::shared_ptr<surface> surf(new surface(desc));
            surfaces[ret] = ref_object<surface>(surf, 0, 0);
         } catch (exception e) {
            IdentifierDB_ReleaseId(surfaceIds, ret);
            throw e;
         }
      }
   }

   /**
    * destroy_surface
    *
    * Destroy a surface for a given surface identifier.
    */
   void destroy_surface(VMAccelId id) {
      auto it = surfaces.find(id);
      if (it != surfaces.end()) {
         surfaces.erase(it);
         IdentifierDB_ReleaseId(surfaceIds, id);
      }
   }

   /**
    * surface_upload
    *
    * Uploads contents of "in" to a given image region within a surface.
    *
    * @return VMAccelStatusCodeEnum value.
    */
   template <class E>
   int surface_upload(VMAccelId id, VMAccelSurfaceRegion imgRegion,
                      ref_object<E> &in) {
      auto it = surfaces.find(id);
      if (it == surfaces.end()) {
         return VMACCEL_FAIL;
      }
      return it->second->upload<E>(in);
   }

   /**
    * surface_download
    *
    * Downloads from a given image region within a surface to the memory
    * referenced by "out".
    *
    * @return VMAccelStatusCodeEnum value.
    */
   template <class E>
   int surface_download(VMAccelId id, VMAccelSurfaceRegion imgRegion,
                        ref_object<E> &out) {
      auto it = surfaces.find(id);
      if (it == surfaces.end()) {
         return VMACCEL_FAIL;
      }
      return it->second->download<E>(out);
   }

private:
   CLIENT *mgrClnt;
   IdentifierDB *surfaceIds;
   std::map<VMAccelId, ref_object<surface>> surfaces;
};
};

#endif /* defined _VMACCEL_HPP_ */
