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

extern "C" {
#include "vmaccel_defs.h"
#include "vmaccel_mgr.h"
#if ENABLE_VMACCEL_LOCAL
#include "vmaccel_local.h"
#include "vmaccel_ops.h"
#else
#include "vmaccel_rpc.h"
#endif
#include "vmaccel_utils.h"
#include "vmaccel_types_address.h"
#include "vmaccel_types_address.hpp"
}

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace vmaccel {

class context;
class operation;
template <class T>
class ref_object;
class surface;
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
      if (!VMAccel_AddressStringToOpaqueAddr(host.c_str(), addr.addr.addr_val,
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
   const VMAccelAddress &ref_accel_addr() const { return addr; }

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
    * Given a queue id, create an Accelerator object.
    */
   object(VMAccelId q) { queueId = q; }

   /**
    * Constructor.
    *
    * Given a pointer and a size of the backing memory, create an Accelerator
    * object.
    */
   object(void *p, size_t s, unsigned int u, VMAccelId q) {
      ptr = p;
      size = s;
      usage = u;
      queueId = q;
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
    * get_queue_id
    *
    * @return The last assigned queue identifier, in VM Accelerator Manager
    *         space.
    */
   virtual const VMAccelId get_queue_id() const { return queueId; }

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
   VMAccelId queueId;
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
      : object(ptr, sizeof(T), VMACCEL_SURFACE_USAGE_READWRITE, 0) {
      data = std::shared_ptr<T>(ptr);
   }

   ref_object(T *ptr, size_t s)
      : object(ptr, s, VMACCEL_SURFACE_USAGE_READWRITE, 0) {
      data = std::shared_ptr<T>(ptr);
   }

   ref_object(T *ptr, size_t s, unsigned int u) : object(ptr, s, u, 0) {
      data = std::shared_ptr<T>(ptr);
   }

   ref_object(std::shared_ptr<T> &o, size_t s, unsigned int u, VMAccelId q)
      : object(o.get(), s, u, q) {
      data = o;
   }

   /**
    * Copy constructor.
    */
   ref_object(const ref_object &obj)
      : object(obj.data.get(), obj.get_size(), obj.get_usage(),
               obj.get_queue_id()) {
      LOG_ENTRY(("ref_object::Copy Constructor {\n"));
      data = obj.data;
      LOG_EXIT(("} ref_object::Copy Constructor\n"));
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
 * VMAccel Accelerator interface class.
 */
class accelerator {

public:
   /**
    * Constructor.
    *
    * Opens a connection to the Accelerator Management server.
    *
    * @param mgr Specifies the address of the Accelerator Managment server,
    *            in VMAccelAddress format.
    */
   accelerator(const address &mgr,
               int accelMaxRefObjects = VMACCEL_MAX_REF_OBJECTS) {
      char host[256];
      if (!VMAccel_IsLocal()) {
         DeepCopy(mgrAddr, mgr.ref_accel_addr());
      }
      if (VMAccel_AddressOpaqueAddrToString(mgr.get_accel_addr(), host,
                                            sizeof(host))) {
         VMACCEL_LOG("vmaccel: Connecting to Accelerator manager %s\n", host);
         mgrClnt = clnt_create(host, VMACCELMGR, VMACCELMGR_VERSION, "tcp");
         if (mgrClnt != NULL) {
            /*
             * Set a one minute timeout. This must be sufficient for any bulkd
             * data transfer.
             */
            struct timeval tv;
            tv.tv_sec = 60;
            tv.tv_usec = 0;

            VMACCEL_LOG("vmaccel: Setting protocol timeout to %ld seconds\n",
                        tv.tv_sec);

            if (!clnt_control(mgrClnt, CLSET_TIMEOUT, (char *)&tv)) {
               VMACCEL_WARNING("vmaccel: Unable to set timeout..\n");
            }
         }
      }
      refObjectDB = IdentifierDB_Alloc(accelMaxRefObjects);
      if (refObjectDB == NULL) {
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
      if (refObjectDB != NULL) {
         IdentifierDB_Free(refObjectDB);
         refObjectDB = NULL;
      }

      if (mgrClnt != NULL) {
         clnt_destroy(mgrClnt);
         mgrClnt = NULL;
      }

      if (!VMAccel_IsLocal()) {
         Destructor(mgrAddr);
      }
   }

   /**
    * get_manager_addr
    *
    * @return A pointer to the address structure for the manager.
    */
   VMAccelAddress *get_manager_addr() { return &mgrAddr; }

   /**
    * get_manager
    *
    * @return A pointer to the manager client.
    */
   CLIENT *get_manager() { return mgrClnt; }

   /**
    * alloc_id
    *
    * @return A valid accelerator id if available.
    */
   VMAccelId alloc_id() {
      VMAccelId ret = VMACCEL_INVALID_ID;
      IdentifierDB_AllocId(refObjectDB, &ret);
      return ret;
   }

   /**
    * release_id
    *
    * Releases an accelerator id.
    */
   void release_id(VMAccelId id) { IdentifierDB_ReleaseId(refObjectDB, id); }

   /**
    * get_max_ref_objects
    *
    * @return A maximum number of ref objects.
    */
   int get_max_ref_objects() { return IdentifierDB_Size(refObjectDB); }

   /**
    * add_context
    */
   void add_context(VMAccelId id, ref_object<context> &c) { contexts[id] = c; }

   /**
    * remove_context
    */
   void remove_context(VMAccelId id) {
      auto it = contexts.find(id);
      if (it != contexts.end()) {
         contexts.erase(it);
      }
#if DEBUG_OBJECT_LIFETIME
      else {
         VMACCEL_WARNING("%s: Context %d not tracked by accelerator\n",
                         __FUNCTION__, id);
      }
#endif
   }

   /**
    * get_context_database
    *
    * @return A reference to the context database.
    */
   std::map<VMAccelId, ref_object<context>> &get_context_database() {
      return contexts;
   }

   /**
    * add_surface
    */
   void add_surface(VMAccelId id, ref_object<surface> &s) { surfaces[id] = s; }

   /**
    * remove_surface
    */
   void remove_surface(VMAccelId id) {
      auto it = surfaces.find(id);
      if (it != surfaces.end()) {
         surfaces.erase(it);
      }
#if DEBUG_OBJECT_LIFETIME
      else {
         VMACCEL_WARNING("%s: Surface %d not tracked by accelerator.\n",
                         __FUNCTION__, id);
      }
#endif
   }

   /**
    * get_surface_database
    *
    * @return A reference to the surface database.
    */
   std::map<VMAccelId, ref_object<surface>> &get_surface_database() {
      return surfaces;
   }

private:
   VMAccelAddress mgrAddr;
   CLIENT *mgrClnt;

   /*
    * Maximum ref_objects allowed for the accelerator.
    */
   int maxRefObjects;

   /*
    * ref_object ID database.
    */
   IdentifierDB *refObjectDB;

   /*
    * Mapping from ref_object ID to the Context Objects allocated.
    */
   std::map<VMAccelId, ref_object<context>> contexts;

   /*
    * Mapping from ref_object ID to the Surface Objects allocated.
    */
   std::map<VMAccelId, ref_object<surface>> surfaces;
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
   surface() : object() { backing = nullptr; }

   /**
    * Constructor.
    */
   surface(const std::shared_ptr<accelerator> &a, VMAccelId q,
           VMAccelSurfaceDesc d)
      : object(q) {
      LOG_ENTRY(("surface::Constructor {\n"));
      accel = a;
      desc = d;
      generation = 0;
      id = a->alloc_id();
      backing = std::shared_ptr<char>(new char[d.width]);
      consistencyDB = IdentifierDB_Alloc(a->get_max_ref_objects());
      LOG_EXIT(("} surface::Constructor\n"));
   }

   /**
    * Destructor.
    */
   ~surface() {
      LOG_ENTRY(("surface::Destructor {\n"));
      destroy();
      accel->release_id(id);
      IdentifierDB_Free(consistencyDB);
      consistencyDB = NULL;
      LOG_EXIT(("} surface::Destructor\n"));
   }

   /**
    * Copy constructor.
    */
   surface(const surface &obj)
      : object(obj.backing.get(), obj.get_size(), obj.get_usage(),
               obj.get_queue_id()) {
      LOG_ENTRY(("surface::Copy Constructor {\n"));
      accel = obj.accel;
      desc = obj.desc;
      generation = 0;
      id = accel->alloc_id();
      backing = obj.backing;
      consistencyDB = IdentifierDB_Alloc(accel->get_max_ref_objects());
      LOG_EXIT(("} surface::Copy Constructor\n"));
   }

   /**
    * Accessors.
    */
   const std::shared_ptr<accelerator> &get_accel() const { return accel; }

   VMAccelId &get_id() { return id; }

   VMAccelSurfaceDesc &get_desc() { return desc; }

   unsigned int &get_generation() { return generation; }

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
#if DEBUG_SURFACE_CONSISTENCY
      VMACCEL_LOG("%s: surface id=%d\n", __FUNCTION__, id);
#endif
      if (desc.format == VMACCEL_FORMAT_R8_TYPELESS && imgRegion.coord.x == 0 &&
          imgRegion.coord.y == 0 && imgRegion.coord.z == 0 &&
          imgRegion.size.x * sizeof(E) == desc.width &&
          imgRegion.size.y == desc.height && imgRegion.size.z == desc.depth) {
         memcpy(backing.get(), in.get_ptr(), MIN(desc.width, in.get_size()));
         set_consistency_range(0, accel->get_max_ref_objects() - 1, false);
         generation++;
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
#if DEBUG_SURFACE_CONSISTENCY
      VMACCEL_LOG("%s: surface id=%d\n", __FUNCTION__, id);
#endif
      if (desc.format == VMACCEL_FORMAT_R8_TYPELESS && imgRegion.coord.x == 0 &&
          imgRegion.coord.y == 0 && imgRegion.coord.z == 0 &&
          imgRegion.size.x * sizeof(E) == desc.width &&
          imgRegion.size.y == desc.height && imgRegion.size.z == desc.depth) {
#if DEBUG_SURFACE_CONSISTENCY
         VMACCEL_LOG("%s: id=%d ptr=%d, %d, %d, %d\n", __FUNCTION__, id,
                     ((unsigned int *)backing.get())[0],
                     ((unsigned int *)backing.get())[1],
                     ((unsigned int *)backing.get())[2],
                     ((unsigned int *)backing.get())[3]);
#endif
         memcpy(out.get_ptr(), backing.get(), MIN(desc.width, out.get_size()));
         return VMACCEL_SUCCESS;
      }
      return VMACCEL_FAIL;
   }

   /**
    * is_consistent
    *
    * Gets the consistency state for an Accelerator ID.
    */
   bool is_consistent(VMAccelId id) {
      return IdentifierDB_ActiveId(consistencyDB, id);
   }

   /**
    * log_consistency
    */
   void log_consistency() {
      char str[256];
      snprintf(str, sizeof(str), "surface[%d].consistency", id);
      IdentifierDB_Log(consistencyDB, str);
   }

   /**
    * set_consistency
    *
    * Sets the consistency state for an Accelerator ID.
    */
   void set_consistency(VMAccelId id, bool state) {
      if (state) {
         IdentifierDB_AcquireId(consistencyDB, id);
      } else {
         IdentifierDB_ReleaseId(consistencyDB, id);
      }
   }

   /**
    * set_consistency_range
    *
    * Sets the consistency state for a range of Accelerator ID(s).
    */
   void set_consistency_range(int start, int end, bool state) {
      if (state) {
         IdentifierDB_AcquireIdRange(consistencyDB, start, end);
      } else {
         IdentifierDB_ReleaseIdRange(consistencyDB, start, end);
      }
   }

   void destroy();

private:
   /*
    * Acclerator de-reference.
    */
   std::shared_ptr<accelerator> accel;

   /*
    * Accelerator Identifier Database ID.
    */
   VMAccelId id;

   /*
    * Surface descriptor structure.
    */
   VMAccelSurfaceDesc desc;

   /*
    * Current generation
    */
   unsigned int generation;

   /*
    * Backing memory for the surface.
    */
   std::shared_ptr<char> backing;

   /*
    * Consistency database, for tracking if an object's consistency with
    * regards to this context.
    */
   IdentifierDB *consistencyDB;
};

/**
 * Accelerator surface encapsulating class.
 *
 * This class is used to encapsulate vmaccel::surface, to unify the management
 * of adding a surface object to the vmaccel::accelerator. A reference object
 * must be added to vmaccel::accelerator for dereference by active contexts
 * during context destruction.
 */

class accelerator_surface {

public:
   /**
    * Default constructor.
    */
   accelerator_surface() {}

   /**
    * Constructor.
    *
    * @param a Accelerator class used to instantiate the compute operation.
    * @param q Queue ID for the accelerator surface.
    * @param d Descriptor for the surface.
    */
   accelerator_surface(const std::shared_ptr<accelerator> &a, VMAccelId q,
                       VMAccelSurfaceDesc d) {
      std::shared_ptr<surface> surfPtr;
      surfPtr = std::shared_ptr<surface>(new surface(a, q, d));
      surf = ref_object<surface>(surfPtr, sizeof(vmaccel::surface), 0, 0);
      /*
       * Add the surface to the accelerator tracking list for context
       * destruction to dereference.
       */
      a->add_surface(surf->get_id(), surf);
   }

   /**
    * Destructor.
    */
   ~accelerator_surface() {
      /*
       * Remove the surface from the tracking list.
       */
      surf->get_accel()->remove_surface(surf->get_id());
   }

   /**
    * Accessors.
    */
   vmaccel::surface *operator->() { return surf.get().get(); }

   operator ref_object<vmaccel::surface> &() { return surf; }

private:
   ref_object<vmaccel::surface> surf;
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
   binding(VMAccelResourceType typeMask, VMAccelSurfaceBindFlags bindFlags,
           VMAccelSurfaceUsage bindUsage, ref_object<surface> &target) {
      accelMask = typeMask;
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
   /*
    * Accelerator resource type for this binding.
    */
   VMAccelResourceType accelMask;

   /*
    * Bind flags for this binding.
    */
   VMAccelSurfaceBindFlags flags;

   /*
    * Usage flags for this binding.
    */
   VMAccelSurfaceUsage usage;

   /*
    * Surface Object referenced by this binding.
    */
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
      ctxTypeMask = VMACCEL_NONE;
      tag = "NOP";
   }

   /**
    * prepare
    *
    * Prepares the operation.
    */
   template <class... B>
   void prepare(VMAccelResourceType typeMask, std::string &t, B &... args) {
      ctxTypeMask = typeMask;
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
   VMAccelResourceType ctxTypeMask;

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

   context(const std::shared_ptr<accelerator> &a, VMAccelResourceType t,
           unsigned int maxRefObjects)
      : object() {
      accel = a;
      typeMask = t;
      residencyDB = IdentifierDB_Alloc(maxRefObjects);
   }

   /**
    * Default destructor.
    */
   ~context() {
      IdentifierDB_Free(residencyDB);
      residencyDB = NULL;
   }

   /**
    * Accessor functions.
    */
   const std::shared_ptr<accelerator> &get_accel() const { return accel; }

   const VMAccelResourceType get_typeMask() const { return typeMask; }

   /**
    * is_resident
    *
    * Gets the residency state for an Accelerator ID.
    */
   bool is_resident(VMAccelId id) {
      return IdentifierDB_ActiveId(residencyDB, id);
   }

   /**
    * set_residency
    *
    * Sets the residency state for an Accelerator ID.
    */
   void set_residency(VMAccelId id, bool state) {
      if (state) {
         IdentifierDB_AcquireId(residencyDB, id);
      } else {
         IdentifierDB_ReleaseId(residencyDB, id);
      }
   }

   /**
    * destroy_surface
    *
    * Destroys a surface within this context.
    */
   virtual void destroy_surface(VMAccelId id) { assert(0); };

protected:
   /*
    * Acclerator de-reference.
    */
   std::shared_ptr<accelerator> accel;

   /*
    * Type of accelerator context.
    */
   VMAccelResourceType typeMask;

   /*
    * Residency database, for tracking if an object's lifetime is active
    * for this context. Assumes all allocated objects are in this process'
    * vmaccel::accelerator ID space.
    */
   IdentifierDB *residencyDB;
};
};

#endif /* defined _VMACCEL_HPP_ */
