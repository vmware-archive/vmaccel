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
   unsigned int contextId = 0;
   unsigned int queueId = 0;

   if (argc > 1) {
      host = argv[1];
   } else {
      host = "127.0.0.1";
   }

   address mgrAddr(host);
   work_topology workTopology({0}, {ARRAY_SIZE}, {1});
   ref_object<accelerator> accel(new accelerator(mgrAddr));

   /*
    * Query an accelerator manager for the Compute Resource.
    */
   std::shared_ptr<vmaccel::clcontext> clctx;

   try {
      clctx = std::shared_ptr<clcontext>(new clcontext(
         accel.get(), 1, contextId, VMACCEL_CPU_MASK | VMACCEL_GPU_MASK,
         0, // (spirv != NULL) ? VMCL_SPIRV_1_0_CAP : 0;
         queueId));
   } catch (vmaccel::exception &e) {
      Warning("%s: Unable to instantiate VMCL\n", __FUNCTION__);
      return VMACCEL_FAIL;
   }

   ref_object<vmaccel::clcontext> c(clctx, sizeof(vmaccel::clcontext), 0);

   /*
    * Initialize the Compute Kernel.
    */
   char *kernelSource = new char[strlen(helloKernel) + 1];

   strcpy(kernelSource, helloKernel);

   ref_object<char> kernel(kernelSource, strlen(kernelSource) + 1,
                           VMACCEL_SURFACE_USAGE_READONLY);
   map<unsigned int, ref_object<char>> kernels;

   kernels[VMCL_IR_NATIVE] = kernel;

   /*
    * Setup the working set.
    */
   ref_object<int> a(new int[ARRAY_SIZE], sizeof(int) * ARRAY_SIZE,
                     VMACCEL_SURFACE_USAGE_READWRITE);

   /*
    * Initialize the array with default values.
    */
   for (int i = 0; i < ARRAY_SIZE; i++) {
      a[i] = i;
   }

   /*
    * Execute the compute operation.
    */
   VMAccelSurfaceDesc desc = {
      0,
   };
   desc.type = VMACCEL_SURFACE_BUFFER;
   desc.width = sizeof(int) * ARRAY_SIZE;
   desc.format = VMACCEL_FORMAT_R8_TYPELESS;
   desc.usage = VMACCEL_SURFACE_USAGE_READWRITE;
   desc.bindFlags = VMACCEL_BIND_UNORDERED_ACCESS_FLAG;
   ref_object<surface> s(new surface(desc));

   VMAccelSurfaceRegion rgn = {0, {0, 0, 0}, {ARRAY_SIZE, 0, 0}};
   if (s->upload<int>(rgn, a) != VMACCEL_SUCCESS) {
      return VMACCEL_FAIL;
   }

   ref_object<binding> b(new binding(VMACCEL_COMPUTE_ACCELERATOR,
                                     VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                                     VMACCEL_SURFACE_USAGE_READWRITE, s));

   /*
    * Create a scope for the Operation Object that forces quiescing before
    * the surface download.
    */
   {
      ref_object<compute_operation> opobj;

      compute<ref_object<binding>>(c, opobj, VMCL_OPENCL_C_1_0, kernels,
                                   "hello_kernel", workTopology, b);
   }

   if (s->download<int>(rgn, a) != VMACCEL_SUCCESS) {
      return VMACCEL_FAIL;
   }

   for (int i = 0; i < ARRAY_SIZE; i++) {
      Log("%s: a[%d] = %u\n", __FUNCTION__, i, a[i]);
   }

   return 1;
}
