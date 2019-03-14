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
 * vmxc_spirv_validate.cpp
 *
 * Demonstrates how to use the SPIRV-Tools validation functionality for a
 * given SPIR-V binary.
 */
#include <cassert>

#include "vmxc_spirv.h"

spv_result_t vmxc_spirv_validate(uint32_t *spirv_data, size_t spirv_size) {
   spv_context ctx = spvContextCreate(SPV_ENV_UNIVERSAL_1_2);
   spv_validator_options validator_options = spvValidatorOptionsCreate();

   assert(ctx != NULL);
   assert(spirv_data != NULL);

   if (validator_options) {
      spv_const_binary_t spirv_binary = {spirv_data, spirv_size};

      const spv_result_t result =
         spvValidateWithOptions(ctx, validator_options, &spirv_binary, nullptr);

      if (result != SPV_SUCCESS) {
         spvContextDestroy(ctx);
         return result;
      }
   }

   spvContextDestroy(ctx);

   return SPV_SUCCESS;
}
