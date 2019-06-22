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
 * vmai_core.hpp
 *
 * Accelerator C++11 core interface for AI acceleration.
 *
 * The classes below use safe types, such as std::string vs. char *, to avoid
 * security issues with unbounded access. To retain an object over the lifetime
 * of an Accelerator operation, std::shared_ptr is required for all arguments.
 */

#ifndef _VMAI_CORE_HPP_
#define _VMAI_CORE_HPP_ 1

#include "vmaccel.hpp"
#include "vmaccel_defs.h"
#include "vmaccel_compute.hpp"
#include "vmai_defs.h"
#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace vmaccel {
namespace ai {

/**
 * VMAccel AI binding encapsulating class.
 *
 * This class is used to encapsulate the binding object for the AI/ML
 * accelerators.
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
      obj = ref_object<vmaccel::binding>(
         new vmaccel::binding(
		VMACCEL_AI_ACCELERATOR_MASK | VMACCEL_ML_ACCELERATOR_MASK,
                bindFlags, usage, s));
   }

   /**
    * Accessors.
    */
   vmaccel::binding *operator->() { return obj.get().get(); }

   operator ref_object<vmaccel::binding> &() { return obj; }

private:
   ref_object<vmaccel::binding> obj;
};

/**
 * VMware Machine Learning Classifier.
 *
 * Base class for all classifiers used for machine learning.
 */
class classifier {

public:
   /**
    * Constructor.
    */
   classifier(unsigned int classifierType) {
      LOG_ENTRY(("classifier::Constructor(a=%d) {\n", classifierType));
      type = classifierType;
      LOG_EXIT(("} classifier::Constructor\n"));
   }

   /**
    * Destructor.
    */
   ~classifier() {
      LOG_ENTRY(("classifier::Destructor {\n"));
      LOG_EXIT(("} classifier::Destructor\n"));
   }

   /**
    * Copy constructor.
    */
   classifier(const classifier &obj) {
      LOG_ENTRY(("classfier::CopyConstructor(...)\n"));
      type = obj.type;
      LOG_EXIT(("} classifier::CopyConstructor\n"));
   }

   /**
    * Stateless functionality for the classifier.
    */
   virtual int
   train(ref_object<vmaccel::ai::binding> model,
         ref_object<vmaccel::ai::binding> parameters,
         ref_object<vmaccel::ai::binding> input,
         classifier &result);
   virtual int
   test(ref_object<vmaccel::ai::binding> reference,
        ref_object<vmaccel::ai::binding> parameters,
        ref_object<vmaccel::ai::binding> input,
        ref_object<vmaccel::ai::binding> accuracy,
        ref_object<vmaccel::ai::binding> variance);
   virtual int
   validate(ref_object<vmaccel::ai::binding> reference,
            ref_object<vmaccel::ai::binding> parameters,
            ref_object<vmaccel::ai::binding> input,
            ref_object<vmaccel::ai::binding> accuracy,
            ref_object<vmaccel::ai::binding> variance);
   virtual int
   predict(ref_object<vmaccel::ai::binding> model,
           ref_object<vmaccel::ai::binding> parameters,
           ref_object<vmaccel::ai::binding> input,
           std::vector<ref_object<vmaccel::ai::binding>> &outputs,
           std::vector<ref_object<vmaccel::ai::binding>> &confidenceIntervals);

private:
   unsigned int type;
};
};
};

#endif /* defined _VMAI_CORE_HPP_ */
