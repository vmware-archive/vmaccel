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
 * cout_cfg_pass.cpp
 *
 * Output an SPIR-V instructions to the console using cout. This
 * demonstrates iterating over BasicBlocks in a Control Flow Graph
 * within the SPIRV-Tools-opt infrastructure.
 */
#include "cout_cfg_pass.h"

#include <list>
#include <stack>


using namespace spvtools;
using namespace spvtools::opt;
using namespace std;

namespace vmxc {
namespace xcode {

Pass::Status CoutCFGPass::Process() {
   bool modified = false;
   cout << "=== Begin CoutCFGPass ===" << endl;
   for (auto &func : *get_module()) {
      std::list<BasicBlock *> structuredOrder;
      if (!get_module()->context()->get_feature_mgr()->HasCapability(
             SpvCapabilityShader)) {
         cout << "Control Flow Graph unsupported..." << endl;
         cout << "=== End CoutCFGPass ===" << endl;
         return Status::SuccessWithoutChange;
      }

      cfg()->ComputeStructuredOrder(&func, &*func.begin(), &structuredOrder);
      for (BasicBlock *block : structuredOrder) {
         for (Instruction &inst : *block) {
            cout << inst.PrettyPrint() << endl;
         }
      }
   }
   cout << "=== End CoutCFGPass ===" << endl;
   return (modified ? Status::SuccessWithChange : Status::SuccessWithoutChange);
}

} // namespace xcode
} // namespace vmxc
