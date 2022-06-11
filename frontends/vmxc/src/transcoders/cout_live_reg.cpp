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
 * cout_live_reg.cpp
 *
 * Output SPIR-V live registers to the console using cout. This
 * demonstrates analyzing live registers within the SPIRV-Tools-opt
 * infrastructure.
 */
#include <iostream>

#include "cout_live_reg.h"
#include "opt/ir_context.h"
#include "opt/register_pressure.h"
#include "reg_analysis_pass.h"

using namespace spvtools;
using namespace spvtools::opt;
using namespace std;

namespace vmxc {
namespace xcode {

bool CoutLiveReg::ProcessFn(Function *func) {
   cout << "==== Begin Function ====" << endl;
   RegisterLiveness liveness(context(), func);
   std::map<uint32_t, uint32_t> resultIDs;
   std::map<uint32_t, uint32_t> uniqueIDs;
   std::map<uint32_t, uint32_t> variableIDs;
   for (auto &block : *func) {
      RegisterLiveness::RegionRegisterLiveness *blockLiveness;
      RegAnalysisPass regAnalysis;

      blockLiveness = liveness.Get(block.id());
      regAnalysis.GetIDUsage(block.begin(), block.end(), resultIDs, uniqueIDs,
                             variableIDs);

      cout << "Block[" << block.id() << "] number of live registers used: "
           << blockLiveness->used_registers_ << endl;
      cout << "  Input registers" << endl;
      for (auto inst : blockLiveness->live_in_) {
         cout << "    " << *inst << endl;
      }
      cout << "  Output registers" << endl;
      for (auto inst : blockLiveness->live_out_) {
         cout << "    " << *inst << endl;
      }
   }
   if (!resultIDs.empty()) {
      cout << "  Result IDs = [";
      for (auto &id : resultIDs) {
         cout << id.first << ",";
      }
      cout << "]" << endl;
   }
   if (!uniqueIDs.empty()) {
      cout << "  Unique IDs = [";
      for (auto &id : uniqueIDs) {
         cout << id.first << ",";
      }
      cout << "]" << endl;
   }
   if (!variableIDs.empty()) {
      cout << "  Variable IDs = [";
      for (auto &id : variableIDs) {
         cout << id.first << ",";
      }
      cout << "]" << endl;
   }
   cout << "==== End Function ====" << endl;
   return true;
}

Pass::Status CoutLiveReg::Process() {
   cout << "=== Begin CoutLiveReg ===" << endl;
   std::map<uint32_t, uint32_t> resultIDs;
   std::map<uint32_t, uint32_t> uniqueIDs;
   std::map<uint32_t, uint32_t> variableIDs;
   RegAnalysisPass regAnalysis;
   bool modified = false;

   // Global pass to process forward declarations and block instructions
   context()->module()->ForEachInst(
      [&regAnalysis, &resultIDs, &uniqueIDs, &variableIDs](Instruction *inst) {
         regAnalysis.HandleInstruction(inst, resultIDs, uniqueIDs, variableIDs);
      },
      true);

   if (!resultIDs.empty()) {
      cout << "  Global Result IDs = [";
      for (auto &id : resultIDs) {
         cout << id.first << ",";
      }
      cout << "]" << endl;
   }
   if (!uniqueIDs.empty()) {
      cout << "  Global Unique IDs = [";
      for (auto &id : uniqueIDs) {
         cout << id.first << ",";
      }
      cout << "]" << endl;
   }

   ProcessFunction pfn = [this](Function *fp) { return ProcessFn(fp); };
   modified |= context()->ProcessReachableCallTree(pfn);
   cout << "=== End CoutLiveReg ===" << endl;
   return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

} // namespace xcode
} // namespace vmxc
