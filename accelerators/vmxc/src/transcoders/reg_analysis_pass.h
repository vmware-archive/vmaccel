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

/*
 * reg_analysis_pass.h
 *
 * Header file for the SPIRV-Tools-opt COUT Register Liveness pass transcoder.
 */
#ifndef _REG_ANALYSIS_PASS_H_
#define _REG_ANALYSIS_PASS_H_

#include "opt/ir_context.h"
#include "opt/module.h"
#include "opt/pass.h"


using namespace spvtools;
using namespace spvtools::opt;
using namespace std;

namespace vmxc {
namespace xcode {

// See optimizer.hpp for documentation.
class RegAnalysisPass : public Pass {
public:
   using iterator = InstructionList::iterator;
   using const_iterator = InstructionList::const_iterator;

   const char *name() const override { return "reg_analysis_pass"; }
   Status Process() override;
   bool ProcessFn(Function *func);

   // Accessor functions for the class instance
   map<uint32_t, uint32_t> &GetResultIDUsage() { return globalResultIDs; }
   map<uint32_t, uint32_t> &GetUniqueIDUsage() { return globalUniqueIDs; }
   map<uint32_t, uint32_t> &GetVariableIDUsage() { return globalVariableIDs; }

   // Utility functions that do not take a class instance
   static void GetIDUsage(iterator begin, iterator end,
                          map<uint32_t, uint32_t> &resultMap,
                          map<uint32_t, uint32_t> &uniqueMap,
                          map<uint32_t, uint32_t> &variableMap);
   static void HandleInstruction(Instruction *inst,
                                 map<uint32_t, uint32_t> &resultMap,
                                 map<uint32_t, uint32_t> &uniqueMap,
                                 map<uint32_t, uint32_t> &variableMap);
   static bool MergeIDUsage(map<uint32_t, uint32_t> &mapA,
                            map<uint32_t, uint32_t> &mapB,
                            map<uint32_t, uint32_t> &mapOut);

private:
   map<uint32_t, uint32_t> globalResultIDs;
   map<uint32_t, uint32_t> globalUniqueIDs;
   map<uint32_t, uint32_t> globalVariableIDs;
};

} // namespace xcode
} // namespace vmxc

#endif // _REG_ANALYSIS_PASS_H_
