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

#include "vmcl_callback_rpc.h"

void vmcl_callback_1(char *host) {
   CLIENT *clnt;
   VMAccelReturnStatus *result_1;
   VMCLCallbackOp vmcl_callbackop_1_arg;

#ifndef DEBUG
   clnt = clnt_create(host, VMCL_CALLBACK, VMCL_CALLBACK_VERSION, "udp");
   if (clnt == NULL) {
      clnt_pcreateerror(host);
      exit(1);
   }
#endif /* DEBUG */

   result_1 = vmcl_callbackop_1(&vmcl_callbackop_1_arg, clnt);
   if (result_1 == (VMAccelReturnStatus *)NULL) {
      clnt_perror(clnt, "call failed");
   }
#ifndef DEBUG
   clnt_destroy(clnt);
#endif /* DEBUG */
}

int main(int argc, char *argv[]) {
   char *host;

   if (argc < 2) {
      printf("usage: %s server_host\n", argv[0]);
      exit(1);
   }
   host = argv[1];
   vmcl_callback_1(host);
   exit(0);
}
