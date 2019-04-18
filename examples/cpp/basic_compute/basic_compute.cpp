/******************************************************************************

Copyright (c) 2019 VMware, Inc.
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

#include "vmaccel_compute.hpp"

#include "log_level.h"

using namespace std;
using namespace vmaccel;

#define ARRAY_SIZE 32

const char *helloKernel = "__kernel void hello_kernel(__global int *a)\n"
                          "{\n"
                          "    int gid = get_global_id(0);\n"
                          "    a[gid] = a[gid] + a[gid];\n"
                          "}\n";

int main(int argc, char **argv) {
   std::string host;

   if (argc > 1) {
      host = argv[1];
   } else {
      host = "127.0.0.1";
   }

   address mgrAddr(host);
   work_topology workTopology({0}, {ARRAY_SIZE}, {1});
   std::shared_ptr<accelerator> accel(new accelerator(mgrAddr));

   /*
    * Initialize the Compute Kernel.
    */
   char *kernelSource = new char[strlen(helloKernel) + 1];

   strcpy(kernelSource, helloKernel);

   shared_ptr<char> kernelPtr(kernelSource);
   ref_object<char> kernel(kernelPtr, strlen(kernelSource) + 1,
                           VMACCEL_SURFACE_USAGE_READONLY);
   map<unsigned int, ref_object<char>> kernels;

   kernels[VMCL_IR_NATIVE] = kernel;

   /*
    * Setup the working set.
    */
   shared_ptr<int> intArray(new int[ARRAY_SIZE]);
   ref_object<int> a(intArray, sizeof(int) * ARRAY_SIZE,
                     VMACCEL_SURFACE_USAGE_READWRITE);

   /*
    * Initialize the array with default values.
    */
   for (int i = 0; i < ARRAY_SIZE; i++) {
      intArray.get()[i] = i;
   }

   /*
    * Execute the compute operation.
    */
   int ret = compute<ref_object<int>>(accel, VMCL_OPENCL_C_1_0, kernels,
                                      "hello_kernel", workTopology, a);

   Log("%s: compute ret = %d\n", __FUNCTION__, ret);

   for (int i = 0; i < ARRAY_SIZE; i++) {
      Log("%s: intArray[%d] = %u\n", __FUNCTION__, i, intArray.get()[i]);
   }

   return 1;
}
