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
 * vmxc_spirv_base.hpp
 *
 * Baseline SPIR-V infrastructure for using the spvBinaryParse interface
 */
#include <algorithm>
#include <cassert>
#include <vector>

#include "binary.h"
#include "diagnostic.h"
#include "instruction.h"
#include "opcode.h"
#include "val/instruction.h"

using namespace spvtools;

// Returns the vector of words representing the concatenation
// of all input vectors.
inline std::vector<uint32_t>
SPIRVConcatenate(const std::vector<std::vector<uint32_t>> &instructions) {
   std::vector<uint32_t> result;
   for (const auto &instruction : instructions) {
      result.insert(result.end(), instruction.begin(), instruction.end());
   }
   return result;
}

// Returns the vector of words representing the concatenation
// of all input vectors.
inline void
SPIRVConcatenate(std::vector<uint32_t> &stream,
                 const std::vector<std::vector<uint32_t>> &instructions) {
   for (const auto &instruction : instructions) {
      std::copy(instruction.begin(), instruction.end(),
                std::back_inserter(stream));
   }
}

// Returns a vector of words representing a single instruction with the
// given opcode and operand words as a vector.
inline std::vector<uint32_t>
SPIRVMakeInstruction(SpvOp opcode, const std::vector<uint32_t> &args) {
   std::vector<uint32_t> result{
      spvOpcodeMake(uint16_t(args.size() + 1), opcode)};
   result.insert(result.end(), args.begin(), args.end());
   return result;
}

// Encodes a string as a sequence of words, using the SPIR-V encoding.
inline std::vector<uint32_t> SPIRVMakeVector(std::string input) {
   std::vector<uint32_t> result;
   uint32_t word = 0;
   size_t num_bytes = input.size();

   // SPIR-V strings are null-terminated.  The byte_index == num_bytes
   // case is used to push the terminating null byte.
   for (size_t byte_index = 0; byte_index <= num_bytes; byte_index++) {
      const auto new_byte =
         (byte_index < num_bytes ? uint8_t(input[byte_index]) : uint8_t(0));
      word |= (new_byte << (8 * (byte_index % sizeof(uint32_t))));
      if (3 == (byte_index % sizeof(uint32_t))) {
         result.push_back(word);
         word = 0;
      }
   }

   // Emit a trailing partial word.
   if ((num_bytes + 1) % sizeof(uint32_t)) {
      result.push_back(word);
   }
   return result;
}

class VMXCSPIRVBase {

public:
   /*
    * Base constructor functionality.
    */
   VMXCSPIRVBase() = delete;
   virtual ~VMXCSPIRVBase() {
      spvValidatorOptionsDestroy(validator_options_);
      spvContextDestroy(context_);
   }

   explicit VMXCSPIRVBase(spv_target_env targetEnv) {
      context_ = spvContextCreate(targetEnv);
      validator_options_ = spvValidatorOptionsCreate();
   }

   /*
    * Base instruction processing functionality.
    */
   virtual spv_result_t
   ProcessHeader(spv_endianness_t /* endian */, uint32_t /* magic */,
                 uint32_t /* version */, uint32_t /* generator */,
                 uint32_t /* id_bound */, uint32_t /* schema */) {
      return SPV_SUCCESS;
   }

   virtual spv_result_t
   ProcessInstruction(const spv_parsed_instruction_t &inst) {
      inst_ = inst;
      instructions_.emplace_back(new val::Instruction(&inst_));
      return SPV_SUCCESS;
   }

   virtual spv_result_t ProcessEnd() { return SPV_SUCCESS; }

protected:
   /*
    * VMXCSPIRVBase instance state
    */
   spv_validator_options validator_options_ = nullptr;
   spv_context context_ = nullptr;

   // Current instruction, current operand and current operand index.
   spv_parsed_instruction_t inst_;
   spv_parsed_operand_t operand_;
   uint32_t operand_index_;

   // List of instructions in the order they are given in the module.
   std::vector<std::unique_ptr<const val::Instruction>> instructions_;
};
