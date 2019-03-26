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
#include <memory>
#include <string>
#include <vector>

namespace vmaccel {

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

private:
   std::shared_ptr<T> data;
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
   accelerator(const address &mgr) {
      char host[256];
      if (VMAccelAddressOpaqueAddrToString(mgr.get_accel_addr(), host,
                                           sizeof(host))) {
         Log("%s: Connecting to Accelerator manager %s\n", __FUNCTION__, host);
         mgrClnt = clnt_create(host, VMACCELMGR, VMACCELMGR_VERSION, "udp");
         if (mgrClnt == NULL) {
            clnt_pcreateerror(host);
            exit(1);
         }
         Log("%s: mgrClient = %p\n", __FUNCTION__, mgrClnt);
      }
   }

   /**
    * Destructor.
    *
    * Closes the connection to the Accelerator Management server.
    */
   ~accelerator() { clnt_destroy(mgrClnt); }

   /**
    * get_manager
    *
    * @return A pointer to the manager client.
    */
   CLIENT *get_manager() { return mgrClnt; }

private:
   CLIENT *mgrClnt;
};
};

#endif /* defined _VMACCEL_HPP_ */
