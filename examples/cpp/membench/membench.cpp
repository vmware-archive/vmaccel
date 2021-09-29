/******************************************************************************

Copyright (c) 2020-2021 VMware, Inc.
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

const char *matrixCopy2DKernel =
   "__kernel void MatrixCopy2D(__global int *a, __global int *b,\n"
   "                           __global int *semaphores,\n"
   "                           __global int *dims)\n"
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
   "         b[n * chunkSize * i + chunkSize * j + k] =\n"
   "            a[n * chunkSize * i + chunkSize * j + k];\n"
   "      }\n"
   "   }\n"
   "}\n";

const char *matrixAddTranspose2DKernel =
   "__kernel void MatrixAddTranspose2D(__global int *a, __global int *b,\n"
   "                                   __global int *semaphores,\n"
   "                                   __global int *dims)\n"
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
   "         b[n * chunkSize * j + chunkSize * i + k] +=\n"
   "            a[n * chunkSize * i + chunkSize * j + k];\n"
   "      }\n"
   "   }\n"
   "}\n";

const char *matrixCopyTranspose2DKernel =
   "__kernel void MatrixCopyTranspose2D(__global int *a, __global int *b,\n"
   "                                    __global int *semaphores,\n"
   "                                    __global int *dims)\n"
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
   "         b[n * chunkSize * j + chunkSize * i + k] =\n"
   "            a[n * chunkSize * i + chunkSize * j + k];\n"
   "      }\n"
   "   }\n"
   "}\n";

const char *matrixAdd2DSemaphore1UKernel =
   "__kernel void MatrixAdd2DSemaphore1U(__global int *a, __global int *b,\n"
   "                                     __global int *semaphores,\n"
   "                                     __global int *dims)\n"
   "{\n"
   "   int i = get_global_id(0);\n"
   "   int j = get_global_id(1);\n"
   "   int m = dims[0];\n"
   "   int n = dims[1];\n"
   "   int k, l;\n"
   "   int chunkSize = dims[2];\n"
   "   int numPasses = dims[3];\n"
   "   while (semaphores[0] == 0);\n"
   "   for (l = 0; l < numPasses; l++) {\n"
   "      for (k = 0; k < chunkSize; k++) {\n"
   "         b[n * chunkSize * i + chunkSize * j + k] +=\n"
   "            a[n * chunkSize * i + chunkSize * j + k];\n"
   "      }\n"
   "   }\n"
   "}\n";

const char *matrixAdd2DSemaphoreNUKernel =
   "__kernel void MatrixAdd2DSemaphoreNU(__global int *a, __global int *b,\n"
   "                                     __global int *semaphores,\n"
   "                                     __global int *dims)\n"
   "{\n"
   "   int i = get_global_id(0);\n"
   "   int j = get_global_id(1);\n"
   "   int m = dims[0];\n"
   "   int n = dims[1];\n"
   "   int k, l;\n"
   "   int chunkSize = dims[2];\n"
   "   int numPasses = dims[3];\n"
   "   while (semaphores[n * i + j] == 0);\n"
   "   for (l = 0; l < numPasses; l++) {\n"
   "      for (k = 0; k < chunkSize; k++) {\n"
   "         b[n * chunkSize * i + chunkSize * j + k] +=\n"
   "            a[n * chunkSize * i + chunkSize * j + k];\n"
   "      }\n"
   "   }\n"
   "}\n";

const char *matrixAdd2DExecChainNUKernel =
   "__kernel void MatrixAdd2DExecChainNU(__global int *a, __global int *b,\n"
   "                                     __global int *semaphores,\n"
   "                                     __global int *dims)\n"
   "{\n"
   "   int i = get_global_id(0);\n"
   "   int j = get_global_id(1);\n"
   "   int m = dims[0];\n"
   "   int n = dims[1];\n"
   "   int k = 0, l = 0;\n"
   "   int chunkSize = dims[2];\n"
   "   int numPasses = dims[3];\n"
   "   while (semaphores[n * i + j] == 0) {\n"
   "      k = k + 1;\n"
   "   }\n"
   "   semaphores[n * i + j + 1] = 1;\n"
   "   for (l = 0; l < numPasses; l++) {\n"
   "      for (k = 0; k < chunkSize; k++) {\n"
   "         b[n * chunkSize * i + chunkSize * j + k] +=\n"
   "            a[n * chunkSize * i + chunkSize * j + k];\n"
   "      }\n"
   "   }\n"
   "}\n";

const char *matrixAdd2DEpsilonJumpNUKernel =
   "__kernel void MatrixAdd2DEpsilonJumpNU(__global int *a, __global int *b,\n"
   "                                       __global int *semaphores,\n"
   "                                       __global int *dims)\n"
   "{\n"
   "   int i = get_global_id(0);\n"
   "   int j = get_global_id(1);\n"
   "   int m = dims[0];\n"
   "   int n = dims[1];\n"
   "   int k, l;\n"
   "   int chunkSize = dims[2];\n"
   "   for (l = 0; ; l++) {\n"
   "      for (k = 0; k < chunkSize; k++) {\n"
   "         if (semaphores[n * i + j] == 1) {\n"
   "            semaphores[n * i + j] = l;\n"
   "            return;\n"
   "         }\n"
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
   MEMSET = -4,
   UPLOAD_A = -3,
   DOWNLOAD_A = -2,
   MEMCPY = -1,
   MATRIX_ADD_2D = 0,
   MATRIX_COPY_2D,
   UPLOAD_MATRIX_COPY,
   DOWNLOAD_MATRIX_COPY,
   MATRIX_ADD_TRANSPOSE_2D,
   MATRIX_COPY_TRANSPOSE_2D,
   MATRIX_ADD_2D_SEMAPHORE1U,
   MATRIX_ADD_2D_SEMAPHORENU,
   MATRIX_ADD_2D_EXEC_CHAIN,
   MATRIX_ADD_2D_EPSILON_JUMPNU,
   MATRIX_FUNCTION_MAX
} FunctionEnum;

#define MEMPOOL(_LOC) ((_LOC)&0xff)
#define MEMDEVICE(_LOC) (((_LOC) >> 16) & 0xffff)
#define MEMQUEUE(_LOC) (((_LOC) >> 8) & 0xff)

FunctionTableEntry functionTable[MATRIX_FUNCTION_MAX] = {
   {matrixAdd2DKernel, "MatrixAdd2D"},
   {matrixCopy2DKernel, "MatrixCopy2D"},
   {matrixCopy2DKernel, "MatrixCopy2D"},
   {matrixCopy2DKernel, "MatrixCopy2D"},
   {matrixAddTranspose2DKernel, "MatrixAddTranspose2D"},
   {matrixCopyTranspose2DKernel, "MatrixCopyTranspose2D"},
   {matrixAdd2DSemaphore1UKernel, "MatrixAdd2DSemaphore1U"},
   {matrixAdd2DSemaphoreNUKernel, "MatrixAdd2DSemaphoreNU"},
   {matrixAdd2DExecChainNUKernel, "MatrixAdd2DExecChainNU"},
   {matrixAdd2DEpsilonJumpNUKernel, "MatrixAdd2DEpsilonJumpNU"},
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
                          int *pEpsilonDelayMS, int *pVerbose) {
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
      } else if (strcmp("--epsilonDelayMS", argv[i]) == 0) {
         if (i + 1 == argc) {
            return 1;
         }
         *pEpsilonDelayMS = atoi(argv[i + 1]);
         i += 2;
      } else if (strcmp("--dirtyPages", argv[i]) == 0) {
         if (i == argc) {
            return 1;
         }
         *pDirtyPages = TRUE;
         i++;
      } else if (strcmp("--help", argv[i]) == 0) {
         printf("Usage: membench <options>\n\n");
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
         printf("  --epsilonDelayMS <x> Delay in milliseconds before semaphore"
                " update\n");
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
   struct timespec quiesceDiffTime, quiesceStartTime, quiesceEndTime;
   float totalRuntimeMS = 0.0f;
   float quiesceTimeMS = 0.0f;
   int epsilonDelayMS = 0;
   std::string host = "127.0.0.1";
   int numRows = 8192;
   int numColumns = 1;
   int chunkSize = 4096;
   int numPasses = 1;
   int numIterations = 100;
   int memoryPoolA = VMACCEL_SURFACE_POOL_AUTO;
   int memoryPoolB = VMACCEL_SURFACE_POOL_AUTO;
   int memoryPoolS = VMACCEL_SURFACE_POOL_AUTO;
   int kernelFunc = MATRIX_ADD_2D;
   int kernelDevice = 0;
   int dirtyPages = FALSE;
   int verbose = FALSE;
   int numSubDevices = 0;
   int numQueues = 0;

   if (ParseCommandArguments(
          argc, argv, host, &numRows, &numColumns, &chunkSize, &numPasses,
          &numIterations, &memoryPoolA, &memoryPoolB, &memoryPoolS, &kernelFunc,
          &kernelDevice, &dirtyPages, &epsilonDelayMS, &verbose)) {
      return 1;
   }

   VMACCEL_LOG("\n");
   VMACCEL_LOG("== VMCL Memory Benchmark ==\n");
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
   VMACCEL_LOG("  Epsilon Semaphore Update Delay:         %d ms\n",
               epsilonDelayMS);
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

   if ((kernelFunc == MATRIX_ADD_2D_SEMAPHORE1U ||
        kernelFunc == MATRIX_ADD_2D_SEMAPHORENU ||
        kernelFunc == MATRIX_ADD_2D_EXEC_CHAIN ||
        kernelFunc == MATRIX_ADD_2D_EPSILON_JUMPNU) &&
       ((numQueues < 2) || (MEMQUEUE(memoryPoolS) == 0))) {
      VMACCEL_LOG("Semaphore device queue must be non-zero to run"
                  " concurrently with kernel\n");
      return 1;
   }

   if ((kernelFunc == MATRIX_ADD_2D_EPSILON_JUMPNU) && (numIterations != 1)) {
      VMACCEL_LOG("Epsilon jump quiesce time measurements require single"
                  " iteration\n");
      return 1;
   }

#if VMACCEL_LOCAL
   vmcl_poweron_svc(NULL);
#elif ENABLE_VMCL_STREAM_SERVER
   vmaccel_stream_poweron();
#endif

   address mgrAddr(host);
   work_topology workTopology({0}, {numRows}, {numColumns});
   ref_object<accelerator> accel(new accelerator(mgrAddr));

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
   /*
    * Create a scope for the Operation Object that forces quiescing before
    * the surface download.
    */
   {
      compute::context c(accel.get(), 1, VMACCEL_CPU_MASK | VMACCEL_GPU_MASK,
                         numSubDevices, numQueues, 0);
      size_t surfBytes = (size_t)numRows * numColumns * chunkSize * sizeof(int);
      size_t uploadBytes = 0;
      size_t downloadBytes = 0;
      size_t refBytes = 0;
      size_t dirtyBytes = 0;
      size_t computeBytes = 0;

      /*
       * Query an accelerator manager for the Compute Resource.
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
      accelerator_surface accelA(
         accel.get(),
         MEMDEVICE(memoryPoolA) * numQueues + MEMQUEUE(memoryPoolA), descA);
      accelerator_surface accelB(
         accel.get(),
         MEMDEVICE(memoryPoolB) * numQueues + MEMQUEUE(memoryPoolB), descB);
      accelerator_surface accelS(
         accel.get(),
         MEMDEVICE(memoryPoolS) * numQueues + MEMQUEUE(memoryPoolS), semDesc);
      accelerator_surface accelDims(accel.get(), kernelDevice, descDims);

      VMAccelSurfaceRegion rgn = {
         0, {0, 0, 0}, {numRows * numColumns * chunkSize, 0, 0}};
      if (accelA->upload<int>(rgn, memA) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload A\n");
         return 1;
      }
      if (accelB->upload<int>(rgn, memB) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload B\n");
         return 1;
      }

      uploadBytes += 2 * surfBytes;

      VMAccelSurfaceRegion rgnSem = {
         0, {0, 0, 0}, {numRows * numColumns, 0, 0}};

      if (accelS->upload<int>(rgnSem, memS) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload semaphores\n");
         return 1;
      }

      VMAccelSurfaceRegion rgnDims = {0, {0, 0, 0}, {4, 0, 0}};
      if (accelDims->upload<int>(rgnDims, memDims) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload dims\n");
         return 1;
      }

      uploadBytes += 2 * numRows * numColumns * sizeof(int);

      compute::binding bindA(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                             VMACCEL_SURFACE_USAGE_READWRITE, accelA);
      compute::binding bindB(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                             VMACCEL_SURFACE_USAGE_READWRITE, accelB);
      compute::binding bindS(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                             VMACCEL_SURFACE_USAGE_READWRITE, accelS);
      compute::binding bindDims(VMACCEL_BIND_UNORDERED_ACCESS_FLAG,
                                VMACCEL_SURFACE_USAGE_READWRITE, accelDims);

      if (!c->alloc_surface(bindA->get_surf()) ||
          !c->alloc_surface(bindB->get_surf()) ||
          !c->alloc_surface(bindS->get_surf()) ||
          !c->alloc_surface(bindDims->get_surf())) {
         VMACCEL_LOG("ERROR: Unable to allocate surfaces\n");
         return 1;
      }

      c->upload_surface(bindA->get_surf());
      c->upload_surface(bindB->get_surf());
      c->upload_surface(bindS->get_surf());
      c->upload_surface(bindDims->get_surf());

      clock_gettime(CLOCK_REALTIME, &e2eStartTime);

      if (kernelFunc < 0) {
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

               if (accelA->upload<int>(rgn, memA) != VMACCEL_SUCCESS) {
                  VMACCEL_LOG("ERROR: Unable to upload A\n");
                  return 1;
               }
               c->upload_surface(bindA->get_surf(), false);
               uploadBytes += surfBytes;
            }

            if (kernelFunc == MEMSET) {
               VMAccelSurfaceRegion fillRgn = {
                  0, {0, 0, 0}, {surfBytes, 0, 0}};
               unsigned int element = iter;
               // Hard code qid==0
               c->fill_surface(0, bindB->get_surf(), fillRgn, &element,
                               VMACCEL_FORMAT_UINT);
               dirtyBytes += surfBytes;
            } else if (kernelFunc == UPLOAD_A && !dirtyPages) {
               c->upload_surface(bindA->get_surf(), true);
               uploadBytes += surfBytes;
            } else if (kernelFunc == DOWNLOAD_A) {
               c->download_surface(bindA->get_surf(), true);
               downloadBytes += surfBytes;
            } else if (kernelFunc == MEMCPY) {
               VMAccelSurfaceRegion copyRgn = {
                  0, {0, 0, 0}, {surfBytes, 0, 0}};
               // Hard code qid==0
               c->copy_surface(0, bindA->get_surf(), copyRgn, bindB->get_surf(),
                               copyRgn);

               refBytes += surfBytes;
               dirtyBytes += surfBytes;
            }
         }
      } else {
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
               if (accelA->upload<int>(rgn, memA) != VMACCEL_SUCCESS) {
                  VMACCEL_LOG("ERROR: Unable to upload A\n");
                  return 1;
               }
               c->upload_surface(bindA->get_surf(), false);
               uploadBytes += surfBytes;
            }

            if (kernelFunc == UPLOAD_MATRIX_COPY) {
               if (!dirtyPages) {
                  c->upload_surface(bindA->get_surf(), true);
                  uploadBytes += surfBytes;
               }
               opobj->dispatch(true);
               refBytes += surfBytes;
               dirtyBytes += surfBytes;
               computeBytes += surfBytes;
            } else if (kernelFunc == DOWNLOAD_MATRIX_COPY) {
               opobj->dispatch(true);
               opobj->quiesce();
               c->download_surface(bindB->get_surf(), true);
               refBytes += surfBytes;
               dirtyBytes += surfBytes;
               downloadBytes += surfBytes;
               computeBytes += surfBytes;
            } else {
               if (kernelFunc == MATRIX_ADD_2D_SEMAPHORE1U ||
                   kernelFunc == MATRIX_ADD_2D_SEMAPHORENU ||
                   kernelFunc == MATRIX_ADD_2D_EXEC_CHAIN ||
                   kernelFunc == MATRIX_ADD_2D_EPSILON_JUMPNU) {
                  // Clear the semaphores
                  for (int i = 0; i < numRows; i++) {
                     for (int j = 0; j < numColumns; j++) {
                        memS[i * numColumns + j] = 0;
                        uploadBytes += 4 * sizeof(int);
                     }
                  }
                  VMACCEL_LOG("Reset semaphores\n");
                  if (accelS->upload<int>(rgnSem, memS) != VMACCEL_SUCCESS) {
                     VMACCEL_LOG("ERROR: Unable to reset semaphores\n");
                     return 1;
                  }
               }

               opobj->dispatch(true);

               if (kernelFunc == MATRIX_ADD_2D_SEMAPHORE1U ||
                   kernelFunc == MATRIX_ADD_2D_SEMAPHORENU ||
                   kernelFunc == MATRIX_ADD_2D_EXEC_CHAIN ||
                   kernelFunc == MATRIX_ADD_2D_EPSILON_JUMPNU) {
                  VMACCEL_LOG("Workload started...\n");
                  usleep(epsilonDelayMS * 1000);
                  VMACCEL_LOG("Update epsilon semaphore\n");
                  for (int i = 0; i < numRows; i++) {
                     for (int j = 0; j < numColumns; j++) {
                        memS[i * numColumns + j] = 1;
                        uploadBytes += 4 * sizeof(int);
                     }
                  }
                  if (kernelFunc != MATRIX_ADD_2D_EPSILON_JUMPNU) {
                     if (accelS->upload<int>(rgnSem, memS) != VMACCEL_SUCCESS) {
                        VMACCEL_LOG("ERROR: Unable to update semaphores\n");
                        return 1;
                     }
                     c->upload_surface(bindS->get_surf(), true, true);
                  } else {
                     clock_gettime(CLOCK_REALTIME, &quiesceStartTime);
                     if (accelS->upload<int>(rgnSem, memS) != VMACCEL_SUCCESS) {
                        VMACCEL_LOG("ERROR: Unable to update semaphores\n");
                        return 1;
                     }
                     c->upload_surface(bindS->get_surf(), true, true);

                     VMACCEL_LOG("Quiescing operation object...\n");
                     opobj->quiesce();

                     downloadBytes +=
                        2 * surfBytes + 2 * numRows * numColumns * sizeof(int);

                     if (accelS->download<int>(rgnSem, memS) !=
                         VMACCEL_SUCCESS) {
                        VMACCEL_LOG("ERROR: Unable to update semaphores\n");
                        return 1;
                     }
                     VMACCEL_LOG("Operation object quiesced...\n");
                     clock_gettime(CLOCK_REALTIME, &quiesceEndTime);
                  }
               }
               if (kernelFunc == MATRIX_COPY_2D ||
                   kernelFunc == MATRIX_COPY_TRANSPOSE_2D) {
                  refBytes += surfBytes * numPasses;
                  dirtyBytes += surfBytes * numPasses;
               } else {
                  refBytes += 2 * surfBytes * numPasses;
                  dirtyBytes += surfBytes * numPasses;
               }
               computeBytes += surfBytes * numPasses;
            }
         }
         if (kernelFunc != MATRIX_ADD_2D_EPSILON_JUMPNU) {
            opobj->quiesce();
            downloadBytes +=
               2 * surfBytes + 2 * numRows * numColumns * sizeof(int);
         }
      }

      if (kernelFunc == MEMSET || kernelFunc == MEMCPY) {
         // Manually download the surface from the context
         c->download_surface(bindB->get_surf());
      }

      if (accelB->download<int>(rgn, memB) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to readback B\n");
         return 1;
      }

      downloadBytes += surfBytes;

      clock_gettime(CLOCK_REALTIME, &e2eEndTime);
      e2eDiffTime = DiffTime(&e2eStartTime, &e2eEndTime);

      totalRuntimeMS =
         e2eDiffTime.tv_sec * 1000.0f +
         ((e2eDiffTime.tv_nsec != 0) ? (double)e2eDiffTime.tv_nsec / 1000000.0f
                                     : 0.0f);

      VMACCEL_LOG("\n");
      VMACCEL_LOG("End-to-end Time = %lf ms\n", totalRuntimeMS);
      if (kernelFunc == MATRIX_ADD_2D_EPSILON_JUMPNU) {
         int minThreadPassCount = 1 << 30;
         int maxThreadPassCount = 0;
         quiesceDiffTime = DiffTime(&quiesceStartTime, &quiesceEndTime);
         quiesceTimeMS = quiesceDiffTime.tv_sec * 1000.0f +
                         ((quiesceDiffTime.tv_nsec != 0)
                             ? (double)quiesceDiffTime.tv_nsec / 1000000.0f
                             : 0.0f);
         VMACCEL_LOG("Quiesce Time = %lf ms\n", quiesceTimeMS);
         for (int i = 0; i < numRows; i++) {
            for (int j = 0; j < numColumns; j++) {
               minThreadPassCount =
                  MIN(minThreadPassCount, memS[i * numColumns + j]);
               maxThreadPassCount =
                  MAX(maxThreadPassCount, memS[i * numColumns + j]);
               if (verbose) {
                  VMACCEL_LOG("Compute Kernel Passes (Thread %d) = %d\n",
                              i * numColumns + j, memS[i * numColumns + j]);
               }
            }
         }
         VMACCEL_LOG("Min compute kernel thread pass count = %d\n",
                     minThreadPassCount);
         VMACCEL_LOG("Max compute kernel thread pass count = %d\n",
                     maxThreadPassCount);
      } else {
         VMACCEL_LOG("Working Set Total = %zu bytes, A = %zu bytes, B = %zu bytes\n",
                     2*matBytes + semBytes + dimBytes, matBytes, matBytes);
         VMACCEL_LOG("Total Referenced = %ld bytes, %lf bytes/ms\n", refBytes,
                     refBytes / totalRuntimeMS);
         VMACCEL_LOG("Total Dirtied = %ld bytes, %lf bytes/ms\n", dirtyBytes,
                     dirtyBytes / totalRuntimeMS);
         VMACCEL_LOG("Compute Throughput = %ld bytes, %lf bytes/ms\n",
                     computeBytes, computeBytes / totalRuntimeMS);
      }
      if (kernelFunc < 0) {
         VMACCEL_LOG("Total Uploaded = %ld bytes, %lf bytes/ms\n", uploadBytes,
                     uploadBytes / totalRuntimeMS);
         VMACCEL_LOG("Total Downloaded = %ld bytes, %lf bytes/ms\n", downloadBytes,
                     downloadBytes / totalRuntimeMS);
      }
      VMACCEL_LOG("\n");

      for (int i = 0; i < numRows; i++) {
         for (int j = 0; j < numColumns; j++) {
            for (int k = 0; k < chunkSize; k++) {
               int val = memB[i * numColumns * chunkSize + j * chunkSize + k];
               int exp;

               if (kernelFunc == UPLOAD_MATRIX_COPY) {
                  exp = memA[i * numColumns * chunkSize + j * chunkSize + k];
               } else if (kernelFunc == MEMSET) {
                  exp = numIterations - 1;
               } else if (kernelFunc == MATRIX_COPY_2D ||
                          kernelFunc == MEMCPY) {
                  exp = (i * numColumns + j);
               } else if (kernelFunc == MATRIX_COPY_TRANSPOSE_2D) {
                  exp = (j * numColumns + i);
               } else if (kernelFunc == MATRIX_ADD_2D) {
                  exp = numIterations * numPasses * (i * numColumns + j);
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

#if VMACCEL_LOCAL
   vmcl_poweroff_svc();
#elif ENABLE_VMCL_STREAM_SERVER
   vmaccel_stream_poweroff();
#endif

   VMACCEL_LOG("Test PASSED...\n");

   return 0;
}
