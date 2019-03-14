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

#include "vmxc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void vmxc_1(char *host, char *inFile) {
   CLIENT *clnt;
   VMXCReturnStatus *result_1;
   VMXCodeOp vmxc_xcode_1_arg;
   VMXCReturnStatus *result_2;
   char *kernelBinary = NULL;

#ifndef DEBUG
   clnt = clnt_create(host, VMXC, VMXC_VERSION, "udp");
   if (clnt == NULL) {
      clnt_pcreateerror(host);
      exit(1);
   }
#endif /* DEBUG */

   memset(&vmxc_xcode_1_arg, 0, sizeof(vmxc_xcode_1_arg));

   if (inFile) {
      FILE *fp;
      int kernelSize;

      fp = fopen(inFile, "rb");

      if (!fp) {
         printf("Unable to load file\n");
         exit(1);
      }

      fseek(fp, 0L, SEEK_END);
      kernelSize = ftell(fp);
      rewind(fp);

      kernelBinary = malloc(kernelSize);

      if (!kernelBinary) {
         fclose(fp);
         exit(1);
      }

      fread(kernelBinary, kernelSize, 1, fp);

      fclose(fp);

      vmxc_xcode_1_arg.targetLanguage = VMCL_SPIRV_1_0;
      vmxc_xcode_1_arg.kernel.language = VMCL_SPIRV_1_0;
      vmxc_xcode_1_arg.kernel.source.source_len = kernelSize;
      vmxc_xcode_1_arg.kernel.source.source_val = kernelBinary;
   } else {
      return;
   }

   result_2 = vmxc_validate_1(&vmxc_xcode_1_arg.kernel, clnt);
   if (result_2 == (VMXCReturnStatus *)NULL) {
      clnt_perror(clnt, "call failed");
   }

   vmxc_xcode_1_arg.targetLanguage = -1;
   vmxc_xcode_1_arg.kernel.language = VMCL_SPIRV_1_0;

   result_1 = vmxc_xcode_1(&vmxc_xcode_1_arg, clnt);
   if (result_1 == (VMXCReturnStatus *)NULL) {
      clnt_perror(clnt, "call failed");
   }

   vmxc_xcode_1_arg.targetLanguage = VMCL_SPIRV_1_0;
   vmxc_xcode_1_arg.kernel.language = VMCL_SPIRV_1_0;

   result_1 = vmxc_xcode_1(&vmxc_xcode_1_arg, clnt);
   if (result_1 == (VMXCReturnStatus *)NULL) {
      clnt_perror(clnt, "call failed");
   }

   if (kernelBinary) {
      free(kernelBinary);
   }

   if (inFile) {
      char *outFile;
      FILE *fp;

      outFile = malloc(strlen(inFile) + strlen(".xc") + 1);
      sprintf(outFile, "%s.xc", inFile);

      fp = fopen(outFile, "wb+");

      if (!fp) {
         printf("Unable to open file\n");
         exit(1);
      }

      fwrite(result_1->VMXCReturnStatus_u.ret->kernel.source.source_val,
             result_1->VMXCReturnStatus_u.ret->kernel.source.source_len, 1, fp);

      fclose(fp);

      free(outFile);
   }

#ifndef DEBUG
   clnt_destroy(clnt);
#endif /* DEBUG */
}


int main(int argc, char *argv[]) {
   char *host;
   char *inFile = NULL;

   if (argc < 2) {
      printf("usage: %s server_host\n", argv[0]);
      exit(1);
   }
   host = argv[1];

   if (argc >= 2) {
      inFile = argv[2];
   }

   vmxc_1(host, inFile);
   exit(0);
}
