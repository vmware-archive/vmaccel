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
 * cout_bb_pass.cpp
 *
 * Output an SPIR-V instructions to the console using cout. This
 * demonstrates iterating over BasicBlocks within a reachable call
 * tree within the SPIRV-Tools-opt infrastructure.
 */
#include "cout_bb_pass.h"

#include <list>
#include <stack>


using namespace spvtools;
using namespace spvtools::opt;
using namespace std;

namespace vmxc {
namespace xcode {

bool CoutBBPass::ProcessFn(Function *func) {
   for (auto &block : *func) {
      cout << "==== Begin Block ID=" << block.id() << " ====" << endl;
      for (auto &inst : block) {
         cout << inst.PrettyPrint() << endl;
      }
      cout << "==== End Block ID=" << block.id() << " ====" << endl;
   }
   return true;
}

Pass::Status CoutBBPass::Process() {
   cout << "=== Begin CoutBBPass ===" << endl;
   // Process all entry point functions
   ProcessFunction pfn = [this](Function *fp) { return ProcessFn(fp); };
   bool modified = ProcessReachableCallTree(pfn, context());
   cout << "=== End CoutBBPass ===" << endl;
   return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

} // namespace xcode
} // namespace vmxc
