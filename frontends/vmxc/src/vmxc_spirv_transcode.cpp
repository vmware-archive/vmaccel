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
 * vmxc_spirv_transcode.cpp
 *
 * Demonstrates transcode functionality for SPIR-V binaries using the
 * SPIRV-Tools-opt infrastructure. Each transcoder is placed in the
 * src/transcoders directory as a transcoding pass used to mutate the given
 * SPIR-V binary.
 */
#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <spirv_validator_options.h>
#include <sstream>
#include <vector>

#include "opt/build_module.h"
#include "opt/pass_manager.h"
#include "opt/passes.h"
#include "spirv-tools/optimizer.hpp"
#include "tools/io.h"
#include "util/make_unique.h"
#include "vmxc_spirv.h"

/*
 * Internal Passes
 */
#include "transcoders/cout_bb_pass.h"
#include "transcoders/cout_cfg_pass.h"
#include "transcoders/cout_live_reg.h"
#include "transcoders/cout_pass.h"

namespace spvtools {

void DiagnosticsMessageHandler(spv_message_level_t level, const char *,
                               const spv_position_t &position,
                               const char *message) {
   switch (level) {
      case SPV_MSG_FATAL:
      case SPV_MSG_INTERNAL_ERROR:
      case SPV_MSG_ERROR:
         std::cerr << "error: " << position.index << ": " << message
                   << std::endl;
         break;
      case SPV_MSG_WARNING:
         std::cout << "warning: " << position.index << ": " << message
                   << std::endl;
         break;
      case SPV_MSG_INFO:
         std::cout << "info: " << position.index << ": " << message
                   << std::endl;
         break;
      default:
         break;
   }
}

struct Optimizer::PassToken::Impl {
   Impl(std::unique_ptr<opt::Pass> p) : pass(std::move(p)) {}

   std::unique_ptr<opt::Pass> pass; // Internal implementation pass.
};

Optimizer::PassToken CreateCoutPass() {
   return MakeUnique<Optimizer::PassToken::Impl>(
      MakeUnique<vmxc::xcode::CoutPass>());
}

Optimizer::PassToken CreateCoutBBPass() {
   return MakeUnique<Optimizer::PassToken::Impl>(
      MakeUnique<vmxc::xcode::CoutBBPass>());
}

Optimizer::PassToken CreateCoutCFGPass() {
   return MakeUnique<Optimizer::PassToken::Impl>(
      MakeUnique<vmxc::xcode::CoutCFGPass>());
}

Optimizer::PassToken CreateCoutLiveReg() {
   return MakeUnique<Optimizer::PassToken::Impl>(
      MakeUnique<vmxc::xcode::CoutLiveReg>());
}

} // namespace spvtools

using namespace spvtools;

spv_result_t vmxc_spirv_xcode(uint32_t *spirv_data, size_t spirv_size,
                              uint32_t transcoder_flags,
                              uint32_t **spirv_out_data,
                              size_t *spirv_out_size) {
   spv_target_env targetEnv = SPV_ENV_UNIVERSAL_1_2;
   spv_validator_options options = spvValidatorOptionsCreate();
   std::vector<uint32_t> binary;

   Optimizer optimizer(targetEnv);
   optimizer.SetMessageConsumer(
      [](spv_message_level_t level, const char *source,
         const spv_position_t &position, const char *message) {
         Log(DiagnosticsMessageHandler, level, source, position, message);
      });

#ifdef _DEBUG
   if ((transcoder_flags == (uint32_t)-1) && (spirv_out_data == NULL) &&
       (spirv_out_size == NULL)) {
      optimizer.RegisterPass(CreateCoutPass());
   }
#endif

   // Validate the stream first
   spv_context context = spvContextCreate(targetEnv);
   spv_diagnostic diagnostic = nullptr;
   spv_const_binary_t binary_struct = {spirv_data, spirv_size};
   spv_result_t error =
      spvValidateWithOptions(context, options, &binary_struct, &diagnostic);
   if (error) {
      spvDiagnosticPrint(diagnostic);
      spvDiagnosticDestroy(diagnostic);
      spvValidatorOptionsDestroy(options);
      spvContextDestroy(context);
      return error;
   }
   spvDiagnosticDestroy(diagnostic);
   spvValidatorOptionsDestroy(options);
   spvContextDestroy(context);

   // By using the same vector as input and output, we save time in the case
   // that there was no change.
   binary.reserve(spirv_size);

   if (!optimizer.Run(spirv_data, spirv_size, &binary)) {
      std::cerr << "Unable to run transcoding pass..." << std::endl;
      return SPV_ERROR_INVALID_BINARY;
   }

   return SPV_SUCCESS;
}
