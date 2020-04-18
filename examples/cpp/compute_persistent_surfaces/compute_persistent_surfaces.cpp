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
                          "}\n"
                          "__kernel void hello_kernel2(__global int *a)\n"
                          "{\n"
                          "    int gid = get_global_id(0);\n"
                          "    a[gid] = a[gid] + a[gid] + a[gid] + a[gid];\n"
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
   ref_object<accelerator> accel(new accelerator(mgrAddr));

   /*
    * Initialize the Compute Kernel.
    */
   compute::kernel k(VMCL_IR_NATIVE, helloKernel);

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
   /*
    * Create a scope for the Operation Object that forces quiescing before
    * the surface download.
    */
   {
      /*
       * Query an accelerator manager for the Compute Resource.
       */
      compute::context c(accel.get(), 1, VMACCEL_CPU_MASK | VMACCEL_GPU_MASK, 0,
                         0);

      VMAccelSurfaceDesc desc = {
         0,
      };
      desc.type = VMACCEL_SURFACE_BUFFER;
      desc.width = sizeof(int) * ARRAY_SIZE;
      desc.format = VMACCEL_FORMAT_R8_TYPELESS;
      desc.usage = VMACCEL_SURFACE_USAGE_READWRITE;
      desc.bindFlags = VMACCEL_BIND_UNORDERED_ACCESS_FLAG;
      accelerator_surface s(accel.get(), 0, desc);

      VMAccelSurfaceRegion rgn = {0, {0, 0, 0}, {ARRAY_SIZE, 0, 0}};
      if (s->upload<int>(rgn, a) != VMACCEL_SUCCESS) {
         return VMACCEL_FAIL;
      }

      compute::binding b(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                         VMACCEL_SURFACE_USAGE_READWRITE, s);

      {
         ref_object<compute::operation> opobj;

         compute::dispatch<ref_object<vmaccel::binding>>(
            c, 0, opobj, VMCL_OPENCL_C_1_0, k, "hello_kernel", workTopology, b);

         compute::dispatch<ref_object<vmaccel::binding>>(
            c, 0, opobj, VMCL_OPENCL_C_1_0, k, "hello_kernel", workTopology, b);
      }

      if (s->download<int>(rgn, a) != VMACCEL_SUCCESS) {
         return VMACCEL_FAIL;
      }

      for (int i = 0; i < ARRAY_SIZE; i++) {
         VMACCEL_LOG("%s: 4*i -> a[%d] = %u\n", __FUNCTION__, i, a[i]);
      }
   }

   {
      VMAccelSurfaceDesc desc = {
         0,
      };
      desc.type = VMACCEL_SURFACE_BUFFER;
      desc.width = sizeof(int) * ARRAY_SIZE;
      desc.format = VMACCEL_FORMAT_R8_TYPELESS;
      desc.usage = VMACCEL_SURFACE_USAGE_READWRITE;
      desc.bindFlags = VMACCEL_BIND_UNORDERED_ACCESS_FLAG;
      accelerator_surface s(accel.get(), 0, desc);

      VMAccelSurfaceRegion rgn = {0, {0, 0, 0}, {ARRAY_SIZE, 0, 0}};
      if (s->upload<int>(rgn, a) != VMACCEL_SUCCESS) {
         return VMACCEL_FAIL;
      }

      compute::binding b(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                         VMACCEL_SURFACE_USAGE_READWRITE, s);

      {
         compute::context c(accel.get(), 1, VMACCEL_CPU_MASK | VMACCEL_GPU_MASK,
                            0, 0);

         /*
          * Discard contents of the first compute operation and re-upload
          * the previous 4*i contents. The below is forcing the Operation
          * Object to finish before re-uploading the old contents. While
          * supplying the previous Operation Object to the next compute
          * operation will dispatch and quiesce, it is later than the
          * intended time before the surface upload.
          *
          * To avoid complexity of interdependent dispatching from the
          * surface class, the surface class does not affect the execution
          * order of pending Operation Objects.
          */
         {
            ref_object<compute::operation> opobj;

            compute::dispatch<ref_object<vmaccel::binding>>(
               c, 0, opobj, VMCL_OPENCL_C_1_0, k, "hello_kernel", workTopology,
               b);
         }

         if (s->upload<int>(rgn, a) != VMACCEL_SUCCESS) {
            return VMACCEL_FAIL;
         }

         {
            ref_object<compute::operation> opobj;

            compute::dispatch<ref_object<vmaccel::binding>>(
               c, 0, opobj, VMCL_OPENCL_C_1_0, k, "hello_kernel", workTopology,
               b);
         }
      }

      if (s->download<int>(rgn, a) != VMACCEL_SUCCESS) {
         return VMACCEL_FAIL;
      }

      for (int i = 0; i < ARRAY_SIZE; i++) {
         VMACCEL_LOG("%s: 8*i -> a[%d] = %u\n", __FUNCTION__, i, a[i]);
      }

      {
         compute::context c(accel.get(), 1, VMACCEL_CPU_MASK | VMACCEL_GPU_MASK,
                            0, 0);

         /*
          * Discard contents of the first compute operation and re-upload
          * the previous 8*i contents Version 2.0. The below is forcing
          * the Operation Object to finish before re-uploading the old
          * contents. While supplying the previous Operation Object to the
          * next compute operation will dispatch and quiesce, it is later
          * than the intended time before the surface upload.
          *
          * To avoid complexity of interdependent dispatching from the
          * surface class, the surface class does not affect the execution
          * order of pending Operation Objects.
          */
         {
            ref_object<compute::operation> opobj;
            ref_object<compute::operation> opobj2;

            compute::dispatch<ref_object<vmaccel::binding>>(
               c, 0, opobj, VMCL_OPENCL_C_1_0, k, "hello_kernel", workTopology,
               b);

            compute::dispatch<ref_object<vmaccel::binding>>(
               c, 0, opobj2, VMCL_OPENCL_C_1_0, k, "hello_kernel2",
               workTopology, b);

            // Overwrite the previous operation.
            opobj = opobj2;

            if (s->upload<int>(rgn, a) != VMACCEL_SUCCESS) {
               return VMACCEL_FAIL;
            }
         }
      }

      if (s->download<int>(rgn, a) != VMACCEL_SUCCESS) {
         return VMACCEL_FAIL;
      }

      for (int i = 0; i < ARRAY_SIZE; i++) {
         VMACCEL_LOG("%s: 32*i -> a[%d] = %u\n", __FUNCTION__, i, a[i]);
      }

      {
         compute::context c(accel.get(), 1, VMACCEL_CPU_MASK | VMACCEL_GPU_MASK,
                            0, 0);

         /*
          * Discard contents of the first compute operation and re-upload
          * the previous 8*i contents Version 2.0. The below is forcing
          * the Operation Object to finish before re-uploading the old
          * contents. While supplying the previous Operation Object to the
          * next compute operation will dispatch and quiesce, it is later
          * than the intended time before the surface upload.
          *
          * To avoid complexity of interdependent dispatching from the
          * surface class, the surface class does not affect the execution
          * order of pending Operation Objects.
          */
         {
            ref_object<compute::operation> opobj;
            ref_object<compute::operation> opobj2;

            compute::dispatch<ref_object<vmaccel::binding>>(
               c, 0, opobj, VMCL_OPENCL_C_1_0, k, "hello_kernel", workTopology,
               b);

            compute::dispatch<ref_object<vmaccel::binding>>(
               c, 0, opobj2, VMCL_OPENCL_C_1_0, k, "hello_kernel2",
               workTopology, b);

            // Force a finish of the operation.
            opobj->finish();

            if (s->download<int>(rgn, a) != VMACCEL_SUCCESS) {
               return VMACCEL_FAIL;
            }
         }
      }

      for (int i = 0; i < ARRAY_SIZE; i++) {
         VMACCEL_LOG("%s: 64*i -> a[%d] = %u\n", __FUNCTION__, i, a[i]);
      }
   }

   return 1;
}
