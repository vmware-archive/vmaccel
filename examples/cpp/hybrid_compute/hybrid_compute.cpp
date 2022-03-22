/******************************************************************************

Copyright (c) 2022 VMware, Inc.
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

const char *matrixAdd2DKernel =
   "__kernel void MatrixAdd2D(__global int *a, __global int *b,\n"
   "                          __global int *semaphores,\n"
   "                          __global int *dims)\n"
   "{\n"
   "   int i = get_global_id(0);\n"
   "   int j = get_global_id(1);\n"
   "   int m = dims[0];\n"
   "   int n = dims[1];\n"
   "   int k, l;\n"
   "   int chunkSize = dims[2];\n"
   "   int numPasses = dims[3];\n"
   "   for (l = 0; l < numPasses; l++) {\n"
   "      for (k = 0; k < chunkSize; k++) {\n"
   "         b[n * chunkSize * i + chunkSize * j + k] +=\n"
   "            a[n * chunkSize * i + chunkSize * j + k];\n"
   "      }\n"
   "   }\n"
   "}\n";

typedef struct FunctionTableEntry {
   const char *source;
   const char *function;
} FunctionTableEntry;

enum {
   MATRIX_ADD_2D = 0,
   MATRIX_FUNCTION_MAX
} FunctionEnum;

#define MEMPOOL(_LOC) ((_LOC)&0xff)
#define MEMDEVICE(_LOC) (((_LOC) >> 16) & 0xffff)
#define MEMQUEUE(_LOC) (((_LOC) >> 8) & 0xff)

FunctionTableEntry functionTable[MATRIX_FUNCTION_MAX] = {
   {matrixAdd2DKernel, "MatrixAdd2D"},
};

/**
 * @brief Parses the command line arguments for a C program.
 *
 * @param argc Number of arguments.
 * @param argv Argument array.
 * ...
 *
 * @return 0 if successful, 1 otherwise
 */
int ParseCommandArguments(int argc, char **argv, std::string &host,
                          int *pNumRows, int *pNumColumns, int *pChunkSize,
                          int *pNumPasses, int *pNumIterations,
                          int *pMemoryPoolA, int *pMemoryPoolB,
                          int *pMemoryPoolS, int *pKernelFunc,
                          int *pKernelDevice, int *pDirtyPages,
                          int *pEarlyInit, int *pFastResume,
                          int *pCopyToUserMemory, int *pVerbose) {
   int i = 1;

   while (i < argc) {
      if (strcmp("-h", argv[i]) == 0) {
         host = argv[i + 1];
         i += 2;
      } else if (strcmp("-v", argv[i]) == 0) {
         *pVerbose = TRUE;
         i++;
      } else if (strcmp("-m", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pNumRows = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("-n", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pNumColumns = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("-k", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pChunkSize = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("-l", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pNumPasses = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("-i", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pNumIterations = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("--memoryPoolA", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pMemoryPoolA = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("--memoryPoolB", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pMemoryPoolB = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("--memoryPoolS", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pMemoryPoolS = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("--kernelFunc", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pKernelFunc = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("--kernelDevice", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pKernelDevice = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("--earlyInit", argv[i]) == 0) {
         *pEarlyInit = TRUE;
         i++;
      } else if (strcmp("--fastResume", argv[i]) == 0) {
         *pFastResume = TRUE;
         i++;
      } else if (strcmp("--copyToUserMemory", argv[i]) == 0) {
         *pCopyToUserMemory = TRUE;
         i++;
      } else if (strcmp("--dirtyPages", argv[i]) == 0) {
         if (i == argc) {
            return 1;
         }
         *pDirtyPages = TRUE;
         i++;
      } else if (strcmp("--help", argv[i]) == 0) {
         printf("Usage: hybrid_compute <options>\n\n");
         printf("  --help               Help and usage information\n");
         printf("  -v                   Verbose output\n");
         printf("  -h <IP>              Host to execute workload on\n");
         printf("  -i <num iterations>  Number of iterations\n");
         printf("  -l <num passes>      Number of passes within each thread\n");
         printf("  -m <number of rows>  Number of rows in the matrix,\n");
         printf("                       i.e. 1024x1024 matrix \'-m 1024\'\n");
         printf("  -n <number of cols>  Number of columns in the matrix,\n");
         printf("                       i.e. 1024x1024 matrix \'-n 1024\'\n");
         printf("  -k <chunk size>      Chunk size for each thread to iterate "
                "over\n");
         printf("  --kernelFunc <fn>    Xfer function:\n");
         printf("                         MEMCPY=-1, MATRIX ADD=0, MATRIX "
                "COPY=1, ...\n");
         printf("  --kernelDevice <#>   Device to run the compute kernel\n");
         printf("  --memoryPoolA <x>    Input pool where we will allocate from "
                "to store\n");
         printf("                       this portion of the working set. x is "
                "as follows:\n\n");
         printf("                          pool={ AUTO=0, ACCELERATOR=1, "
                "SYSTEM=2 }\n\n");
         printf("                          if device index == 0\n");
         printf("                            x=pool\n");
         printf("                          else\n");
         printf(
            "                            x=65536 * device index + pool\n\n");
         printf("  --memoryPoolB <x>    Output memory pool\n");
         printf("  --memoryPoolS <x>    Semaphore memory pool\n");
         printf("  --earlyInit          Initialize the remote accelerator early\n");
         printf("  --fastResume         Fast resume of remote accelerator workload\n");
         printf("  --copyToUserMemory   Copy accelerator state to user memory\n");
         printf("  --dirtyPages         Dirty pages each iteration\n");
         printf("\n");
         exit(1);
      } else {
         VMACCEL_LOG("invalid argument %s\n", argv[i]);
         return 1;
      }
   }

   return VMACCEL_SUCCESS;
}

int main(int argc, char **argv) {
   struct timespec e2eDiffTime, e2eStartTime, e2eEndTime;
   struct timespec localDiffTime, localStartTime, localEndTime;
   struct timespec localSetupDiffTime, localSetupStartTime, localSetupEndTime;
   struct timespec remoteDiffTime, remoteStartTime, remoteEndTime;
   struct timespec remoteSetupDiffTime, remoteSetupStartTime, remoteSetupEndTime;
   struct timespec saveDiffTime, saveStartTime, saveEndTime;
   struct timespec restoreDiffTime, restoreStartTime, restoreEndTime;
   struct timespec stunDiffTime, stunStartTime, stunEndTime;
   float totalRuntimeMS = 0.0f;
   float totalLocalRuntimeMS = 0.0f;
   float totalLocalSetupRuntimeMS = 0.0f;
   float totalRemoteRuntimeMS = 0.0f;
   float totalRemoteSetupRuntimeMS = 0.0f;
   float totalSaveTimeMS = 0.0f;
   float totalRestoreTimeMS = 0.0f;
   float totalStunTimeMS = 0.0f;
   std::string host = "127.0.0.1";
   std::string localHost = "";
   int numRows = 8192;
   int numColumns = 1;
   int chunkSize = 4096;
   int numPasses = 1;
   int numIterations = 1;
   int memoryPoolA = VMACCEL_SURFACE_POOL_ACCELERATOR;
   int memoryPoolB = VMACCEL_SURFACE_POOL_ACCELERATOR;
   int memoryPoolS = VMACCEL_SURFACE_POOL_ACCELERATOR;
   int kernelFunc = MATRIX_ADD_2D;
   int kernelDevice = 0;
   int dirtyPages = FALSE;
   int earlyInit = FALSE;
   int fastResume = FALSE;
   int copyToUserMemory = FALSE;
   int verbose = FALSE;
   int numSubDevices = 0;
   int numQueues = 0;

   if (ParseCommandArguments(
          argc, argv, host, &numRows, &numColumns, &chunkSize, &numPasses,
          &numIterations, &memoryPoolA, &memoryPoolB, &memoryPoolS, &kernelFunc,
          &kernelDevice, &dirtyPages, &earlyInit, &fastResume,
          &copyToUserMemory, &verbose)) {
      return 1;
   }

   VMACCEL_LOG("\n");
   VMACCEL_LOG("== VMCL Hybrid Compute ==\n");
   VMACCEL_LOG("  Kernel Function:                        %d\n", kernelFunc);
   VMACCEL_LOG("  Kernel Device:                          %d\n", kernelDevice);
   VMACCEL_LOG("  Rows, Columns, Chunk Size:              %d, %d, %d\n",
               numRows, numColumns, chunkSize);
   VMACCEL_LOG("  Number of Kernel Passes:                %d\n", numPasses);
   VMACCEL_LOG("  Source Memory (Device,Queue,Pool):      %d,%d,%d\n",
               MEMDEVICE(memoryPoolA), MEMQUEUE(memoryPoolA),
               MEMPOOL(memoryPoolA));
   VMACCEL_LOG("  Destination Memory (Device,Queue,Pool): %d,%d,%d\n",
               MEMDEVICE(memoryPoolB), MEMQUEUE(memoryPoolB),
               MEMPOOL(memoryPoolB));
   VMACCEL_LOG("  Semaphore Memory (Device,Queue,Pool):   %d,%d,%d\n",
               MEMDEVICE(memoryPoolS), MEMQUEUE(memoryPoolS),
               MEMPOOL(memoryPoolS));
   VMACCEL_LOG("  Early Initialization:                   %d\n", earlyInit);
   VMACCEL_LOG("  Fast Resume:                            %d\n", fastResume);
   VMACCEL_LOG("  Copy To User Memory:                    %d\n", copyToUserMemory);
   VMACCEL_LOG("  Number of Iterations:                   %d\n", numIterations);
   VMACCEL_LOG("\n");

   numSubDevices = MAX(kernelDevice + 1, MEMDEVICE(memoryPoolA) + 1);
   numSubDevices = MAX(numSubDevices, MEMDEVICE(memoryPoolB) + 1);
   numSubDevices = MAX(numSubDevices, MEMDEVICE(memoryPoolS) + 1);

   numQueues = MAX(1, MEMQUEUE(memoryPoolA) + 1);
   numQueues = MAX(numQueues, MEMQUEUE(memoryPoolB) + 1);
   numQueues = MAX(numQueues, MEMQUEUE(memoryPoolS) + 1);

   VMACCEL_LOG("  Number of Sub-devices:                  %d\n", numSubDevices);
   VMACCEL_LOG("  Number of Queues:                       %d\n", numQueues);

   address localAddr(localAddr);
   address mgrAddr(host);
   work_topology workTopology({0}, {numRows}, {numColumns});
   ref_object<accelerator> localAccel(new accelerator(mgrAddr, VMACCEL_MAX_REF_OBJECTS,
                                                      true, ENABLE_DATA_STREAMING));
   ref_object<accelerator> remoteAccel(new accelerator(mgrAddr, VMACCEL_MAX_REF_OBJECTS,
                                                       false, ENABLE_DATA_STREAMING));


   /*
    * Initialize the Compute Kernel.
    */
   compute::kernel k(VMCL_OPENCL_C_1_0, functionTable[kernelFunc].source);

   /*
    * Setup the working set.
    */
   size_t matBytes = sizeof(int) * numRows *numColumns *chunkSize;
   size_t semBytes = sizeof(int) * (numRows * numColumns + 1);
   size_t dimBytes = sizeof(int) * 4;

   ref_object<int> memA(new int[numRows * numColumns * chunkSize],
                        matBytes, VMACCEL_SURFACE_USAGE_READWRITE);
   ref_object<int> memB(new int[numRows * numColumns * chunkSize],
                        matBytes, VMACCEL_SURFACE_USAGE_READWRITE);
   ref_object<int> memS(new int[numRows * numColumns],
                        semBytes, VMACCEL_SURFACE_USAGE_READWRITE);
   ref_object<int> memDims(new int[4], dimBytes,
                           VMACCEL_SURFACE_USAGE_READWRITE);

   memDims[0] = numRows;
   memDims[1] = numColumns;
   memDims[2] = chunkSize;
   memDims[3] = numPasses;

   VMACCEL_LOG("Allocating Working Set: A=%zu bytes, B=%zu bytes"
               " S=%zu bytes, Dims=%zu bytes\n",
               matBytes, matBytes, semBytes, dimBytes);

   /*
    * Initialize the array with default values.
    */
   for (int i = 0; i < numRows; i++) {
      for (int j = 0; j < numColumns; j++) {
         for (int k = 0; k < chunkSize; k++) {
            memA[i * numColumns * chunkSize + j * chunkSize + k] =
               i * numColumns + j;
            memB[i * numColumns * chunkSize + j * chunkSize + k] = 0;
         }
         memS[i * numColumns + j] = 0;
      }
   }

   /*
    * Execute the compute operation.
    */
   VMAccelSurfaceDesc descA = {
      0,
   };
   VMAccelSurfaceDesc descB = {
      0,
   };
   VMAccelSurfaceDesc descDims = {
      0,
   };
   VMAccelSurfaceDesc semDesc;
   VMAccelSurfaceRegion rgn = {
      0, {0, 0, 0}, {numRows * numColumns * chunkSize, 0, 0}};
   VMAccelSurfaceRegion rgnSem = {
      0, {0, 0, 0}, {numRows * numColumns, 0, 0}};
   VMAccelSurfaceRegion rgnDims = {0, {0, 0, 0}, {4, 0, 0}};
   size_t surfBytes = (size_t)numRows * numColumns * chunkSize * sizeof(int);
   size_t uploadBytes = 0;
   size_t downloadBytes = 0;
   size_t refBytes = 0;
   size_t dirtyBytes = 0;
   size_t computeBytes = 0;

   descA.type = VMACCEL_SURFACE_BUFFER;
   descA.width = sizeof(int) * numRows * numColumns * chunkSize;
   descA.format = VMACCEL_FORMAT_R8_TYPELESS;
   descA.pool = MEMPOOL(memoryPoolA);
   descA.usage = VMACCEL_SURFACE_USAGE_READWRITE;
   descA.bindFlags = VMACCEL_BIND_UNORDERED_ACCESS_FLAG;
   descB = descA;
   descB.pool = MEMPOOL(memoryPoolB);
   descDims = descA;
   descDims.width = sizeof(int) * 4;
   descDims.pool = VMACCEL_SURFACE_POOL_AUTO;
   semDesc = descA;
   semDesc.width = sizeof(int) * numRows * numColumns;
   semDesc.pool = MEMPOOL(memoryPoolS);

   clock_gettime(CLOCK_REALTIME, &e2eStartTime);

   /*
    * Create a scope for the Operation Object that forces quiescing before
    * the surface download.
    */
   {
      compute::context c(localAccel.get(), 1, VMACCEL_CPU_MASK | VMACCEL_GPU_MASK,
                         numSubDevices, numQueues, 0);
      accelerator_surface localAccelA(
         localAccel.get(),
         MEMDEVICE(memoryPoolA) * numQueues + MEMQUEUE(memoryPoolA), descA);
      accelerator_surface localAccelB(
         localAccel.get(),
         MEMDEVICE(memoryPoolB) * numQueues + MEMQUEUE(memoryPoolB), descB);
      accelerator_surface localAccelS(
         localAccel.get(),
         MEMDEVICE(memoryPoolS) * numQueues + MEMQUEUE(memoryPoolS), semDesc);
      accelerator_surface localAccelDims(localAccel.get(), kernelDevice, descDims);

      compute::binding bindA(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                             VMACCEL_SURFACE_USAGE_READWRITE, localAccelA);
      compute::binding bindB(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                             VMACCEL_SURFACE_USAGE_READWRITE, localAccelB);
      compute::binding bindS(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                             VMACCEL_SURFACE_USAGE_READWRITE, localAccelS);
      compute::binding bindDims(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                                VMACCEL_SURFACE_USAGE_READWRITE, localAccelDims);

      /*
       * Phase 1: Upload the working set to the local device
       */
      if (localAccelA->upload<int>(rgn, memA) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload A\n");
         return 1;
      }
      if (localAccelB->upload<int>(rgn, memB) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload B\n");
         return 1;
      }

      uploadBytes += 2 * surfBytes;

      if (localAccelS->upload<int>(rgnSem, memS) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload semaphores\n");
         return 1;
      }

      if (localAccelDims->upload<int>(rgnDims, memDims) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload dims\n");
         return 1;
      }

      uploadBytes += 2 * numRows * numColumns * sizeof(int);

      clock_gettime(CLOCK_REALTIME, &localSetupStartTime);

      if (!c->alloc_surface(bindA->get_surf()) ||
          !c->alloc_surface(bindB->get_surf()) ||
          !c->alloc_surface(bindS->get_surf()) ||
          !c->alloc_surface(bindDims->get_surf())) {
         VMACCEL_LOG("ERROR: Unable to allocate surfaces\n");
         return 1;
      }

      c->upload_surface(bindA->get_surf(), true, true, false);
      c->upload_surface(bindB->get_surf(), true, true, false);
      c->upload_surface(bindS->get_surf(), true, true, false);
      c->upload_surface(bindDims->get_surf(), true, true, false);

      clock_gettime(CLOCK_REALTIME, &localSetupEndTime);
      clock_gettime(CLOCK_REALTIME, &localStartTime);

      /*
       * Phase 2: Dispatch job to the local device.
       */
      if (kernelFunc >= 0) {
         ref_object<compute::operation> opobj;

         compute::dispatch<
            ref_object<vmaccel::binding>, ref_object<vmaccel::binding>,
            ref_object<vmaccel::binding>, ref_object<vmaccel::binding>>(
            c, kernelDevice, opobj, k, VMCL_OPENCL_C_1_0,
            functionTable[kernelFunc].function, workTopology, bindA, bindB,
            bindS, bindDims);

         for (int iter = 0; iter < numIterations; iter++) {
            if (dirtyPages) {
               for (int i = 0; i < numRows; i++) {
                  for (int j = 0; j < numColumns; j++) {
                     for (int k = 0; k < chunkSize; k++) {
                        memA[i * numColumns * chunkSize + j * chunkSize + k] =
                           iter * (i * numColumns + j);
                     }
                  }
               }
               if (localAccelA->upload<int>(rgn, memA) != VMACCEL_SUCCESS) {
                  VMACCEL_LOG("ERROR: Unable to upload A\n");
                  return 1;
               }
               c->upload_surface(bindA->get_surf(), false);
               uploadBytes += surfBytes;
            }

            opobj->dispatch(true);

            refBytes += 2 * surfBytes * numPasses;
            dirtyBytes += surfBytes * numPasses;
            computeBytes += surfBytes * numPasses;
         }

         // Begin migration of the local accelerator working set
         clock_gettime(CLOCK_REALTIME, &stunStartTime);

         clock_gettime(CLOCK_REALTIME, &saveStartTime);
         opobj->quiesce();
         if (copyToUserMemory) {
            if (localAccelA->download<int>(rgn, memA) != VMACCEL_SUCCESS) {
               VMACCEL_LOG("ERROR: Unable to readback A\n");
               return 1;
            }
            if (localAccelB->download<int>(rgn, memB) != VMACCEL_SUCCESS) {
               VMACCEL_LOG("ERROR: Unable to readback B\n");
               return 1;
            }
            if (localAccelS->download<int>(rgnSem, memS) != VMACCEL_SUCCESS) {
               VMACCEL_LOG("ERROR: Unable to readback S\n");
               return 1;
            }
            if (localAccelDims->download<int>(rgnDims, memDims) != VMACCEL_SUCCESS) {
               VMACCEL_LOG("ERROR: Unable to readback Dims\n");
               return 1;
            }
         }
         clock_gettime(CLOCK_REALTIME, &saveEndTime);
         downloadBytes += 2 * surfBytes + 2 * numRows * numColumns * sizeof(int);
      }

      clock_gettime(CLOCK_REALTIME, &localEndTime);

      /*
       * Phase 3: Setup the remote compute accelerator.
       */
      compute::context rc(remoteAccel.get(), 1, VMACCEL_CPU_MASK | VMACCEL_GPU_MASK,
                          numSubDevices, numQueues, 0);
      std::shared_ptr<char> localAccelABacking;
      std::shared_ptr<char> localAccelBBacking;
      std::shared_ptr<char> localAccelSBacking;
      std::shared_ptr<char> localAccelDimsBacking;

      if (copyToUserMemory) {
         localAccelABacking = std::shared_ptr<char>(new char[descA.width]);
         localAccelBBacking = std::shared_ptr<char>(new char[descB.width]);
         localAccelSBacking = std::shared_ptr<char>(new char[semDesc.width]);
         localAccelDimsBacking = std::shared_ptr<char>(new char[descDims.width]);
      } else {
         localAccelABacking = localAccelA->get_backing();
         localAccelBBacking = localAccelB->get_backing();
         localAccelSBacking = localAccelS->get_backing();
         localAccelDimsBacking = localAccelDims->get_backing();
      }

      accelerator_surface remoteAccelA(
         remoteAccel.get(),
         MEMDEVICE(memoryPoolA) * numQueues + MEMQUEUE(memoryPoolA), descA,
         localAccelABacking);
      accelerator_surface remoteAccelB(
         remoteAccel.get(),
         MEMDEVICE(memoryPoolB) * numQueues + MEMQUEUE(memoryPoolB), descB,
         localAccelBBacking);
      accelerator_surface remoteAccelS(
         remoteAccel.get(),
         MEMDEVICE(memoryPoolS) * numQueues + MEMQUEUE(memoryPoolS), semDesc,
         localAccelSBacking);
      accelerator_surface remoteAccelDims(remoteAccel.get(), kernelDevice, descDims,
         localAccelDimsBacking);

      compute::binding rBindA(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                              VMACCEL_SURFACE_USAGE_READWRITE, remoteAccelA);
      compute::binding rBindB(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                              VMACCEL_SURFACE_USAGE_READWRITE, remoteAccelB);
      compute::binding rBindS(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                              VMACCEL_SURFACE_USAGE_READWRITE, remoteAccelS);
      compute::binding rBindDims(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                                 VMACCEL_SURFACE_USAGE_READWRITE, remoteAccelDims);

      if (earlyInit) {
         clock_gettime(CLOCK_REALTIME, &remoteSetupStartTime);

         VMACCEL_LOG("Allocating surfaces on remote accelerator\n");

         if (!rc->alloc_surface(rBindA->get_surf()) ||
             !rc->alloc_surface(rBindB->get_surf()) ||
             !rc->alloc_surface(rBindS->get_surf()) ||
             !rc->alloc_surface(rBindDims->get_surf())) {
            VMACCEL_LOG("ERROR: Unable to allocate surfaces\n");
            return 1;
         }
      }

      if (verbose) {
         for (int i = 0; i < numRows; i++) {
            for (int j = 0; j < numColumns; j++) {
               for (int k = 0; k < chunkSize; k++) {
                  int val = memB[i * numColumns * chunkSize + j * chunkSize + k];
                  int exp;

                  VMACCEL_LOG("memA[%d][%d][%d]=%d memB[%d][%d][%d]=%d\n", i, j,
                              k, memA[i * numColumns * chunkSize + j * chunkSize + k],
                              i, j, k, memB[i * numColumns * chunkSize + j * chunkSize + k]);

                  if (kernelFunc == MATRIX_ADD_2D) {
                     exp = numPasses * (i * numColumns + j);
                  } else {
                     continue;
                  }
                  if (val != exp) {
                     VMACCEL_LOG("ERROR: Mismatch i=%d j=%d k=%d %d != %d\n", i, j,
                                 k, val, exp);
                  }
               }
            }
         }
      }

      /*
       * Phase 4: Upload the working set to the remote device
       */
      if (!earlyInit) {
         clock_gettime(CLOCK_REALTIME, &remoteSetupStartTime);

         VMACCEL_LOG("Allocating surfaces on remote accelerator\n");

         if (!rc->alloc_surface(rBindA->get_surf()) ||
             !rc->alloc_surface(rBindB->get_surf()) ||
             !rc->alloc_surface(rBindS->get_surf()) ||
             !rc->alloc_surface(rBindDims->get_surf())) {
            VMACCEL_LOG("ERROR: Unable to allocate surfaces\n");
            return 1;
         }
      }

      clock_gettime(CLOCK_REALTIME, &restoreStartTime);

      VMACCEL_LOG("Uploading surfaces to remote accelerator\n");

      if (copyToUserMemory) {
         if (remoteAccelA->upload<int>(rgn, memA) != VMACCEL_SUCCESS) {
            VMACCEL_LOG("ERROR: Unable to upload A\n");
            return 1;
         }
         if (remoteAccelB->upload<int>(rgn, memB) != VMACCEL_SUCCESS) {
            VMACCEL_LOG("ERROR: Unable to upload B\n");
            return 1;
         }
         if (remoteAccelS->upload<int>(rgnSem, memS) != VMACCEL_SUCCESS) {
            VMACCEL_LOG("ERROR: Unable to upload S\n");
            return 1;
         }
         if (remoteAccelDims->upload<int>(rgnDims, memDims) != VMACCEL_SUCCESS) {
            VMACCEL_LOG("ERROR: Unable to upload Dims\n");
            return 1;
         }
      }
 
      rc->upload_surface(rBindA->get_surf(), true, true, false);
      rc->upload_surface(rBindB->get_surf(), true, true, false);
      rc->upload_surface(rBindS->get_surf(), true, true, false);
      rc->upload_surface(rBindDims->get_surf(), true, true, false);

      uploadBytes += 2 * surfBytes;
      uploadBytes += 2 * numRows * numColumns * sizeof(int);

      clock_gettime(CLOCK_REALTIME, &restoreEndTime);
      clock_gettime(CLOCK_REALTIME, &remoteSetupEndTime);
      clock_gettime(CLOCK_REALTIME, &remoteStartTime);

      /*
       * Phase 5: Dispatch job to the remote compute accelerator.
       */
      if (kernelFunc >= 0) {
         ref_object<compute::operation> opobj;

         if (fastResume) {
            memDims[3] = 0;
            if (remoteAccelDims->upload<int>(rgnDims, memDims) != VMACCEL_SUCCESS) {
               VMACCEL_LOG("ERROR: Unable to upload dims\n");
               return 1;
            }
         }

         compute::dispatch<
            ref_object<vmaccel::binding>, ref_object<vmaccel::binding>,
            ref_object<vmaccel::binding>, ref_object<vmaccel::binding>>(
            rc, kernelDevice, opobj, k, VMCL_OPENCL_C_1_0,
            functionTable[kernelFunc].function, workTopology, rBindA, rBindB,
            rBindS, rBindDims);

         opobj->dispatch(true);

         refBytes += 2 * surfBytes * numPasses;
         dirtyBytes += surfBytes * numPasses;
         computeBytes += surfBytes * numPasses;

         // Initiate a small surface download
         rc->download_surface(rBindS->get_surf(), true, ENABLE_IMAGE_DOWNLOAD);
         downloadBytes += numRows * numColumns * sizeof(int);

         // Results are visible on the remote compute accelerator
         clock_gettime(CLOCK_REALTIME, &stunEndTime);

         opobj->quiesce();
         downloadBytes +=
            2 * surfBytes + 2 * numRows * numColumns * sizeof(int);
      }

      clock_gettime(CLOCK_REALTIME, &remoteEndTime);

      if (remoteAccelA->download<int>(rgn, memA) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to readback A\n");
         return 1;
      }

      if (remoteAccelB->download<int>(rgn, memB) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to readback B\n");
         return 1;
      }

      downloadBytes += surfBytes;
   }

   clock_gettime(CLOCK_REALTIME, &e2eEndTime);
   e2eDiffTime = DiffTime(&e2eStartTime, &e2eEndTime);
   localDiffTime = DiffTime(&localStartTime, &localEndTime);
   localSetupDiffTime = DiffTime(&localSetupStartTime, &localSetupEndTime);
   remoteDiffTime = DiffTime(&remoteStartTime, &remoteEndTime);
   remoteSetupDiffTime = DiffTime(&remoteSetupStartTime, &remoteSetupEndTime);
   saveDiffTime = DiffTime(&saveStartTime, &saveEndTime);
   restoreDiffTime = DiffTime(&restoreStartTime, &restoreEndTime);
   stunDiffTime = DiffTime(&stunStartTime, &stunEndTime);

   totalRuntimeMS =
      e2eDiffTime.tv_sec * 1000.0f +
      ((e2eDiffTime.tv_nsec != 0) ? (double)e2eDiffTime.tv_nsec / 1000000.0f
                                     : 0.0f);

   totalLocalRuntimeMS =
      localDiffTime.tv_sec * 1000.0f +
      ((localDiffTime.tv_nsec != 0) ? (double)localDiffTime.tv_nsec / 1000000.0f
                                     : 0.0f);

   totalLocalSetupRuntimeMS =
      localSetupDiffTime.tv_sec * 1000.0f +
      ((localSetupDiffTime.tv_nsec != 0) ? (double)localSetupDiffTime.tv_nsec / 1000000.0f
                                     : 0.0f);

   totalRemoteRuntimeMS =
      remoteDiffTime.tv_sec * 1000.0f +
      ((remoteDiffTime.tv_nsec != 0) ? (double)remoteDiffTime.tv_nsec / 1000000.0f
                                     : 0.0f);

   totalRemoteSetupRuntimeMS =
      remoteSetupDiffTime.tv_sec * 1000.0f +
      ((remoteSetupDiffTime.tv_nsec != 0) ? (double)remoteSetupDiffTime.tv_nsec / 1000000.0f
                                     : 0.0f);

   totalSaveTimeMS =
      saveDiffTime.tv_sec * 1000.0f +
      ((saveDiffTime.tv_nsec != 0) ? (double)saveDiffTime.tv_nsec / 1000000.0f
                                     : 0.0f);

   totalRestoreTimeMS =
      restoreDiffTime.tv_sec * 1000.0f +
      ((restoreDiffTime.tv_nsec != 0) ? (double)restoreDiffTime.tv_nsec / 1000000.0f
                                     : 0.0f);

   totalStunTimeMS =
      stunDiffTime.tv_sec * 1000.0f +
      ((stunDiffTime.tv_nsec != 0) ? (double)stunDiffTime.tv_nsec / 1000000.0f
                                     : 0.0f);

   VMACCEL_LOG("\n");
   VMACCEL_LOG("End-to-end Time = %lf ms\n", totalRuntimeMS);
   VMACCEL_LOG("Local Run Time = %lf ms\n", totalLocalRuntimeMS);
   VMACCEL_LOG("Local Setup Run Time = %lf ms\n", totalLocalSetupRuntimeMS);
   VMACCEL_LOG("Remote Run Time = %lf ms\n", totalRemoteRuntimeMS);
   VMACCEL_LOG("Remote Setup Run Time = %lf ms\n", totalRemoteSetupRuntimeMS);
   VMACCEL_LOG("Save Time = %lf ms\n", totalSaveTimeMS);
   VMACCEL_LOG("Restore Time = %lf ms\n", totalRestoreTimeMS);
   VMACCEL_LOG("Stun Time = %lf ms\n", totalStunTimeMS);
   VMACCEL_LOG("Working Set Total = %zu bytes, A = %zu bytes, B = %zu bytes\n",
               2*matBytes + semBytes + dimBytes, matBytes, matBytes);
   VMACCEL_LOG("Total Referenced = %ld bytes, %lf bytes/ms\n", refBytes,
               refBytes / totalRuntimeMS);
   VMACCEL_LOG("Total Dirtied = %ld bytes, %lf bytes/ms\n", dirtyBytes,
               dirtyBytes / totalRuntimeMS);
   VMACCEL_LOG("Compute Throughput = %ld bytes, %lf bytes/ms\n",
               computeBytes, computeBytes / totalRuntimeMS);
   VMACCEL_LOG("\n");

   if (fastResume) {
      return 1;
   }

   VMACCEL_LOG("Validating results...\n");

   bool pass = true;

   for (int i = 0; i < numRows; i++) {
      for (int j = 0; j < numColumns; j++) {
         for (int k = 0; k < chunkSize; k++) {
            int val = memB[i * numColumns * chunkSize + j * chunkSize + k];
            int exp;

            if (verbose) {
               VMACCEL_LOG("memA[%d][%d][%d]=%d memB[%d][%d][%d]=%d\n", i, j,
                           k, memA[i * numColumns * chunkSize + j * chunkSize + k],
                           i, j, k, memB[i * numColumns * chunkSize + j * chunkSize + k]);
            }
            if (kernelFunc == MATRIX_ADD_2D) {
               exp = 2 * numPasses * (i * numColumns + j);
            } else {
               continue;
            }
            if (val != exp) {
               VMACCEL_LOG("ERROR: Mismatch i=%d j=%d k=%d %d != %d\n", i, j,
                           k, val, exp);
               pass = false;
            }
         }
      }
   }

   if (pass) {
      VMACCEL_LOG("Test PASSED...\n");
   } else {
      VMACCEL_LOG("Test FAILED...\n");
   }

   return 0;
}
