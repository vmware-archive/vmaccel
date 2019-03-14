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
 * vmxc_spirv_clone.cpp
 *
 * SPIR-V cloning class which uses the spvBinaryParse C-Style callbacks.
 * This functionality demonstrates C-Style callback usage of the SPIRV-Tools
 * infrastructure to parse a binary stream, read information
 * from spv_parsed_instruction, and replicate the SPIR-V binary stream
 */
#include <iostream>

#include "vmxc_spirv.h"
#include "vmxc_spirv_base.hpp"

#include "binary.h"
#include "diagnostic.h"
#include "enum_string_mapping.h"
#include "ext_inst.h"
#include "extensions.h"
#include "id_descriptor.h"
#include "instruction.h"
#include "opcode.h"
#include "operand.h"
#include "spirv-tools/libspirv.h"
#include "spirv_endian.h"
#include "spirv_validator_options.h"
#include "comp/bit_stream.h"
#include "comp/huffman_codec.h"
#include "comp/move_to_front.h"
#include "util/parse_number.h"
#include "val/instruction.h"
#include "val/validate.h"
#include "val/validation_state.h"

using namespace std;

class VMXCSPIRVCloner : public VMXCSPIRVBase {

public:
   static spv_result_t CloneHeader(void *user_data, spv_endianness_t endian,
                                   uint32_t magic, uint32_t version,
                                   uint32_t generator, uint32_t id_bound,
                                   uint32_t schema) {
      VMXCSPIRVCloner *cloner = reinterpret_cast<VMXCSPIRVCloner *>(user_data);
      return cloner->ProcessHeader(endian, magic, version, generator, id_bound,
                                   schema);
   }

   static spv_result_t CloneInstruction(void *user_data,
                                        const spv_parsed_instruction_t *inst) {
      VMXCSPIRVCloner *cloner = reinterpret_cast<VMXCSPIRVCloner *>(user_data);
      return cloner->ProcessInstruction(*inst);
   }

public:
   /*
    * Cloner constructor functionality.
    */
   VMXCSPIRVCloner(spv_target_env targetEnv) : VMXCSPIRVBase(targetEnv) {}

   /*
    * Cloner accessor functionality.
    */
   void GetSPIRV(std::vector<uint32_t> &spirv_out) {
      spirv_out = std::move(spirv_);
   }

   void GetSPIRV(uint32_t *spirv_out, size_t spirv_size) {
      memcpy(spirv_out, spirv_.data(),
             std::min(spirv_.size(), spirv_size) * sizeof(uint32_t));
   }

   /*
    * Cloner instruction processing functionality.
    */
   spv_result_t ProcessHeader(spv_endianness_t /* endian */, uint32_t magic,
                              uint32_t version, uint32_t generator,
                              uint32_t id_bound, uint32_t schema) {
      inst_words_.reserve(25);
      parsed_operands_.reserve(5);
      spirv_.reserve(25);
#ifdef _DEBUG_DISASSEMBLY
      cout << "ProcessHeader: magic = " << magic;
      cout << " version = " << version;
      cout << " generator = " << generator;
      cout << " id_bound = " << id_bound << endl;
#endif
      SPIRVConcatenate(spirv_, {{magic, version, generator, id_bound, schema}});
      return SPV_SUCCESS;
   }

   spv_result_t ProcessInstruction(const spv_parsed_instruction_t &inst) {
      inst_words_.clear();
      parsed_operands_.clear();

      instructions_.emplace_back(new val::Instruction(&inst));

#ifdef _DEBUG_DISASSEMBLY
      cout << "inst: opcode = " << inst.opcode << endl;
      cout << "      ext_inst_type = " << inst.ext_inst_type << endl;
      cout << "      type_id = " << inst.type_id << endl;
      cout << "      result_id = " << inst.result_id << endl;
      cout << "      num_words = " << inst.num_words << endl;
      cout << "      num_operands = " << inst.num_operands << endl;
#endif

      // Clone the instruction opcode and ids.
      inst_.opcode = inst.opcode;
      inst_.ext_inst_type = inst.ext_inst_type;
      inst_.type_id = inst.type_id;
      inst_.result_id = inst.result_id;

      // Setup the number of instruction words and operands.
      inst_.num_words = inst.num_words;
      inst_.num_operands = inst.num_operands;

      if (inst_.num_words == 0) {
         SPIRVConcatenate(spirv_, {{inst.opcode}});
         return SPV_SUCCESS;
      }

      inst_words_.resize(inst_.num_words);
      parsed_operands_.resize(inst_.num_operands);

      // Dereference the temporary instruction and operands storage.
      inst_.operands =
         parsed_operands_.empty() ? nullptr : parsed_operands_.data();

      // Clone the instruction operands to the temporary
      // word storage *WITHOUT* relocation.
      for (int i = 0; i < inst_.num_operands; i++) {
#ifdef _DEBUG_DISASSEMBLY
         cout << "      operands[" << i << "]" << endl;
         cout << "         offset = " << inst.operands[i].offset << endl;
         cout << "         num_words = " << inst.operands[i].num_words << endl;
         cout << "         type = " << inst.operands[i].type << endl;
         cout << "         number_kind = " << inst.operands[i].number_kind
              << endl;
         cout << "         number_bit_width = "
              << inst.operands[i].number_bit_width << endl;
#endif

         parsed_operands_[i].offset = inst.operands[i].offset;
         parsed_operands_[i].num_words = inst.operands[i].num_words;
         parsed_operands_[i].type = inst.operands[i].type;
         parsed_operands_[i].number_kind = inst.operands[i].number_kind;
         parsed_operands_[i].number_bit_width =
            inst.operands[i].number_bit_width;

         assert(inst.operands[i].offset == inst_.operands[i].offset);
         assert(inst.operands[i].num_words == inst_.operands[i].num_words);

         memcpy(&inst_words_[inst_.operands[i].offset],
                &inst.words[inst.operands[i].offset],
                inst_.operands[i].num_words * sizeof(uint32_t));
      }

      // Define the instruction now that the number of words is known
      inst_words_[0] =
         spvOpcodeMake(uint16_t(inst_words_.size()), SpvOp(inst_.opcode));

      // Resizing of inst_words_ is prohibited past this point.
      inst_.words = inst_words_.data();

#ifdef _DEBUG_DISASSEMBLY
      cout << "inst_: opcode = " << inst_.opcode << endl;
      cout << "       ext_inst_type = " << inst_.ext_inst_type << endl;
      cout << "       type_id = " << inst_.type_id << endl;
      cout << "       result_id = " << inst_.result_id << endl;
      cout << "       num_words = " << inst_.num_words << endl;
      cout << "       num_operands = " << inst_.num_operands << endl;

      for (int i = 0; i < inst_.num_operands; i++) {
         cout << "       operands[" << i << "]" << endl;
         cout << "          offset = " << inst_.operands[i].offset << endl;
         cout << "          num_words = " << inst_.operands[i].num_words
              << endl;
         cout << "          type = " << inst_.operands[i].type << endl;
         cout << "          number_kind = " << inst_.operands[i].number_kind
              << endl;
         cout << "          number_bit_width = "
              << inst_.operands[i].number_bit_width << endl;
      }
#endif

      assert(inst_.num_words == inst_words_.size());

      SPIRVConcatenate(spirv_, {inst_words_});

      return SPV_SUCCESS;
   }

   spv_result_t ProcessEnd() { return SPV_SUCCESS; }

   /*
    * Main entry-point for cloning a SPIR-V binary from spirv_data into
    * spirv_out, using the spvBinaryParse functionality.
    */
   spv_result_t Run(uint32_t *spirv_data, size_t spirv_size,
                    uint32_t *spirv_out) {
      spv_diagnostic diagnostic = nullptr;

      assert(context_ != NULL);
      assert(spirv_data != NULL);
      assert(spirv_out != NULL);

      if (validator_options_) {
         spv_const_binary_t spirv_binary = {spirv_data, spirv_size};

         const spv_result_t result = spvValidateWithOptions(
            context_, validator_options_, &spirv_binary, &diagnostic);

         if (result != SPV_SUCCESS) {
            std::cerr << "SPIR-V input validation failed!" << std::endl;
            spvDiagnosticPrint(diagnostic);
            spvDiagnosticDestroy(diagnostic);
            return result;
         }
         spvDiagnosticDestroy(diagnostic);
         diagnostic = nullptr;
      }

      // Initiate the SPIR-V parsing infrastructure with cloning callbacks.
      // The parser will walk the SPIR-V binary and give visibility into the
      // header and instructions encodeded within.
      spv_position_t position = {};

      if (spvBinaryParse(context_, this, spirv_data, spirv_size, CloneHeader,
                         CloneInstruction, &diagnostic) != SPV_SUCCESS) {
         spvDiagnosticPrint(diagnostic);
         spvDiagnosticDestroy(diagnostic);
         return DiagnosticStream(position, context_->consumer, "",
                                 SPV_ERROR_INVALID_BINARY)
                << "Unable to clone SPIR-V";
      }
      spvDiagnosticDestroy(diagnostic);

      // Cloning finished, validation state should have correct id bound.
      ProcessEnd();

      // Output the newly formed SPIR-V binary
      GetSPIRV(spirv_out, spirv_size);

      if (validator_options_) {
         spv_const_binary_t validation_binary = {spirv_out, spirv_size};

         const spv_result_t result = spvValidateWithOptions(
            context_, validator_options_, &validation_binary, &diagnostic);

         if (result != SPV_SUCCESS) {
            std::cerr << "SPIR-V output validation failed!" << std::endl;
            spvDiagnosticPrint(diagnostic);
            spvDiagnosticDestroy(diagnostic);
            return result;
         }
         spvDiagnosticDestroy(diagnostic);
         diagnostic = nullptr;
      }

      return SPV_SUCCESS;
   }

protected:
   /*
    * VMXCSPIRVCloner context state
    */

   // Temporary sink where decoded SPIR-V words are written. Once it contains
   // the
   // entire module, the container is moved and returned.
   std::vector<uint32_t> spirv_;

   // Temporary storage for operands of the currently parsed instruction.
   // Valid until next DecodeInstruction call.
   std::vector<spv_parsed_operand_t> parsed_operands_;

   // Temporary storage for current instruction words.
   // Valid until next DecodeInstruction call.
   std::vector<uint32_t> inst_words_;
};

spv_result_t vmxc_spirv_clone(uint32_t *spirv_data, size_t spirv_size,
                              uint32_t *spirv_out) {
   VMXCSPIRVCloner cloner(SPV_ENV_UNIVERSAL_1_2);

   cloner.Run(spirv_data, spirv_size, spirv_out);

   return SPV_SUCCESS;
}
