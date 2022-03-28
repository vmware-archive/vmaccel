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
 * vmaccel_codec.hpp
 *
 * Accelerator C++11 external interface. The interface class will connect to a
 * given Accelerator Manager Server and negotiate Accelerators based upon the
 * requested operation.
 *
 * The classes below use safe types, such as std::string vs. char *, to avoid
 * security issues with unbounded access. To retain an object over the lifetime
 * of an Accelerator operation, std::shared_ptr is required for all arguments.
 */

#ifndef _VMACCEL_CODEC_HPP_
#define _VMACCEL_CODEC_HPP_ 1

#include "vmaccel.hpp"
#include "vmaccel_defs.h"
#include "vmcodec_defs.h"
#include "vmcodec_rpc.h"
#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace vmaccel {

/**
 * VMCODECContext structure.
 *
 * Bare minimum information for transferring surface contents.
 */
class codeccontext : public context {

public:
   /**
    * Constructor.
    */
   codeccontext(std::shared_ptr<accelerator> &a, unsigned int megaFlops,
                unsigned int selectionMask, unsigned int requiredCaps)
      : context(a, VMACCEL_COMPUTE_ACCELERATOR_MASK, a->get_max_ref_objects()) {
      LOG_ENTRY(("codeccontext::Constructor(a=%p) {\n", a.get()));
      accelId = VMACCEL_INVALID_ID;
      contextId = VMACCEL_INVALID_ID;

      VMAccelStatusCodeEnum ret =
         (VMAccelStatusCodeEnum)alloc(megaFlops, selectionMask, requiredCaps);

      if (ret != VMACCEL_SUCCESS) {
         throw exception(ret, "Unable to allocate context...\n");
      }

      LOG_EXIT(("} codeccontext::Constructor\n"));
   }

   /**
    * Destructor.
    */
   ~codeccontext() {
      LOG_ENTRY(("codeccontext::Destructor {\n"));
      destroy();
      LOG_EXIT(("} codeccontext::Destructor\n"));
   }

   /**
    * Copy constructor.
    */
   codeccontext(const clcontext &obj)
      : context(obj.get_accel(), obj.get_typeMask(),
                obj.get_accel()->get_max_ref_objects()) {
      LOG_ENTRY(("codeccontext::CopyConstructor(obj.clnt=%p, obj.accelId=%d, "
                 "obj.contextId=%d, obj.get_typeMask()=%d) {\n",
                 obj.clnt, obj.accelId, obj.contextId, obj.get_typeMask()));
      clnt = obj.clnt;
      accelId = obj.accelId;
      contextId = obj.contextId;
      LOG_EXIT(("} codeccontext::CopyConstructor\n"));
   }

   /**
    * Virtual function overrides.
    */

   /**
    * Accessors.
    */
   CLIENT *get_client() { return clnt; }

   VMAccelId get_accelId() { return accelId; }

   VMAccelId get_contextId() { return contextId; }

private:
   /**
    * Reservation of a context and an associated queue.
    */
   int alloc(unsigned int megaFlops, unsigned int selectionMask,
             unsigned int requiredCaps) {
      return VMACCEL_SUCCESS;
   }

   /**
    * Forced destroy.
    */
   void destroy() {}

   CLIENT *clnt;
   VMAccelId accelId;
   VMAccelId contextId;
};
}; // namespace vmaccel

#endif /* defined _VMACCEL_CODEC_HPP_ */
