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
 * reg_analysis_pass.cpp
 *
 * Analyze SPIR-V live registers and save the results. This
 * demonstrates analyzing registers within the SPIRV-Tools-opt
 * infrastructure.
 */
#include <iostream>

#include "reg_analysis_pass.h"
#include "opt/ir_context.h"
#include "opt/register_pressure.h"


using namespace spvtools;
using namespace spvtools::opt;
using namespace std;

namespace vmxc {
namespace xcode {

void RegAnalysisPass::HandleInstruction(Instruction *inst,
                                        map<uint32_t, uint32_t> &resultMap,
                                        map<uint32_t, uint32_t> &uniqueMap,
                                        map<uint32_t, uint32_t> &variableMap) {
   if (resultMap.find(inst->result_id()) == resultMap.end()) {
      resultMap[inst->result_id()] = 1;
   } else {
      resultMap[inst->result_id()]++;
   }
   if (uniqueMap.find(inst->unique_id()) == uniqueMap.end()) {
      uniqueMap[inst->unique_id()] = 1;
   } else {
      uniqueMap[inst->unique_id()]++;
   }
   for (Operand &op : *inst) {
      if (op.type == SPV_OPERAND_TYPE_RESULT_ID) {
         if (resultMap.find(op.words[0]) == resultMap.end()) {
            resultMap[op.words[0]] = 1;
         } else {
            resultMap[op.words[0]]++;
         }
      }
      if (op.type == SPV_OPERAND_TYPE_ID) {
         if (uniqueMap.find(op.words[0]) == uniqueMap.end()) {
            uniqueMap[op.words[0]] = 1;
         } else {
            uniqueMap[op.words[0]]++;
         }
      }
      /*
       * TODO: Constants?
       */
      if (spvOperandIsVariable(op.type)) {
         if (variableMap.find(op.words[0]) == variableMap.end()) {
            variableMap[op.words[0]] = 1;
         } else {
            variableMap[op.words[0]]++;
         }
      }
   }
}


void RegAnalysisPass::GetIDUsage(iterator begin, iterator end,
                                 map<uint32_t, uint32_t> &resultMap,
                                 map<uint32_t, uint32_t> &uniqueMap,
                                 map<uint32_t, uint32_t> &variableMap) {
   for (auto &it = begin; it != end; ++it) {
      Instruction &inst = *it;
      HandleInstruction(&inst, resultMap, uniqueMap, variableMap);
   }
}


bool RegAnalysisPass::MergeIDUsage(map<uint32_t, uint32_t> &mapA,
                                   map<uint32_t, uint32_t> &mapB,
                                   map<uint32_t, uint32_t> &mapOut) {
   mapOut.clear();
   for (auto &it : mapA) {
      mapOut[it.first] = it.second;
   }
   for (auto &it : mapB) {
      if (mapOut.find(it.first) != mapOut.end()) {
         mapOut[it.first] += it.second;
      } else {
         mapOut[it.first] = it.second;
      }
   }
   return true;
}


bool RegAnalysisPass::ProcessFn(Function *func) {
   RegisterLiveness liveness(context(), func);
   std::map<uint32_t, uint32_t> resultIDs;
   std::map<uint32_t, uint32_t> uniqueIDs;
   std::map<uint32_t, uint32_t> variableIDs;

   for (auto &block : *func) {
      RegisterLiveness::RegionRegisterLiveness *block_liveness;

      block_liveness = liveness.Get(block.id());
      GetIDUsage(block.begin(), block.end(), resultIDs, uniqueIDs, variableIDs);

      for (auto inst : block_liveness->live_in_) {
         if (resultIDs.find(inst->result_id()) == resultIDs.end()) {
            resultIDs[inst->result_id()] = 1;
         } else {
            resultIDs[inst->result_id()]++;
         }
      }
      for (auto inst : block_liveness->live_out_) {
         if (resultIDs.find(inst->result_id()) == resultIDs.end()) {
            resultIDs[inst->result_id()] = 1;
         } else {
            resultIDs[inst->result_id()]++;
         }
      }
   }

   return true;
}


Pass::Status RegAnalysisPass::Process() {
   std::map<uint32_t, uint32_t> resultIDs;
   std::map<uint32_t, uint32_t> uniqueIDs;
   std::map<uint32_t, uint32_t> variableIDs;
   bool modified = false;

   cout << "=== Begin RegAnalysisPass ===" << endl;

   globalResultIDs.clear();
   globalUniqueIDs.clear();
   globalVariableIDs.clear();

   // Global pass to process forward declarations and block instructions
   context()->module()->ForEachInst(
      [this, &resultIDs, &uniqueIDs, &variableIDs](Instruction *inst) {
         this->HandleInstruction(inst, resultIDs, uniqueIDs, variableIDs);
      },
      true);

   globalResultIDs = resultIDs;
   globalUniqueIDs = uniqueIDs;
   globalVariableIDs = variableIDs;

   ProcessFunction pfn = [this](Function *fp) { return ProcessFn(fp); };

   modified |= ProcessReachableCallTree(pfn, context());
   cout << "=== End RegAnalysisPass ===" << endl;

   return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

} // namespace xcode
} // namespace vmxc
