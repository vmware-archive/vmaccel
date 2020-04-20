/******************************************************************************

Copyright (c) 2020 VMware, Inc.
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

#define DEBUG_COMPUTE_OPERATION
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
   "   int k, l;\n"
   "   int chunkSize = dims[2];\n"
   "   int numPasses = dims[3];\n"
   "   while (semaphores[n * i + j] == 0);\n"
   "   semaphores[n * i + j + 1] = 1;\n"
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
   MEMCPY = -1,
   MATRIX_ADD_2D = 0,
   MATRIX_COPY_2D,
   MATRIX_ADD_TRANSPOSE_2D,
   MATRIX_COPY_TRANSPOSE_2D,
   MATRIX_ADD_2D_SEMAPHORE1U,
   MATRIX_ADD_2D_SEMAPHORENU,
   MATRIX_ADD_2D_EXEC_CHAIN,
   MATRIX_FUNCTION_MAX
} FunctionEnum;

#define MEMPOOL(_LOC) ((_LOC)&0xffff)
#define MEMDEVICE(_LOC) (((_LOC) >> 16) & 0xffff)

FunctionTableEntry functionTable[MATRIX_FUNCTION_MAX] = {
   {matrixAdd2DKernel, "MatrixAdd2D"},
   {matrixCopy2DKernel, "MatrixCopy2D"},
   {matrixAddTranspose2DKernel, "MatrixAddTranspose2D"},
   {matrixCopyTranspose2DKernel, "MatrixCopyTranspose2D"},
   {matrixAdd2DSemaphore1UKernel, "MatrixAddSemaphore1U"},
   {matrixAdd2DSemaphoreNUKernel, "MatrixAddSemaphoreNU"},
   {matrixAdd2DExecChainNUKernel, "MatrixAdd2DExecChain"},
};


/**
 * @brief Takes a difference of two timespec structures per the example
 * shown at:
 *
 * https://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
 */
struct timespec DiffTime(struct timespec *start, struct timespec *end) {
   struct timespec diff;
   int numSeconds;

   if (end->tv_nsec < start->tv_nsec) {
      numSeconds = (start->tv_nsec - end->tv_nsec) / 1000000000 + 1;
      end->tv_nsec -= 1000000000 * numSeconds;
      end->tv_sec += numSeconds;
   }
   if (end->tv_nsec - start->tv_nsec > 1000000000) {
      numSeconds = (end->tv_nsec - start->tv_nsec) / 1000000000;
      start->tv_nsec += 1000000000 * numSeconds;
      start->tv_sec -= numSeconds;
   }

   diff.tv_sec = end->tv_sec - start->tv_sec;
   diff.tv_nsec = end->tv_nsec - start->tv_nsec;
   return diff;
}

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
                          int *pKernelDevice) {
   int i = 1;

   while (i < argc) {
      if (strcmp("-h", argv[i]) == 0) {
         host = argv[i + 1];
         i += 2;
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
      } else {
         VMACCEL_LOG("invalid argument %s\n", argv[i]);
         return 1;
      }
   }

   return 0;
}

int main(int argc, char **argv) {
   struct timespec e2eDiffTime, e2eStartTime, e2eEndTime;
   float totalRuntimeMS = 0.0f;
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
   int numSubDevices = 0;

   if (ParseCommandArguments(argc, argv, host, &numRows, &numColumns,
                             &chunkSize, &numPasses, &numIterations,
                             &memoryPoolA, &memoryPoolB, &memoryPoolS,
                             &kernelFunc, &kernelDevice)) {
      return 1;
   }

   VMACCEL_LOG("\n");
   VMACCEL_LOG("== VMCL Memory Benchmark ==\n");
   VMACCEL_LOG("  Kernel Function:             %d\n", kernelFunc);
   VMACCEL_LOG("  Kernel Device:               %d\n", kernelDevice);
   VMACCEL_LOG("  Rows, Columns, Chunk Size:   %d, %d, %d\n", numRows,
               numColumns, chunkSize);
   VMACCEL_LOG("  Number of Kernel Passes:     %d\n", numPasses);
   VMACCEL_LOG("  Source Memory Pool:          %d\n", memoryPoolA);
   VMACCEL_LOG("  Destination Memory Pool:     %d\n", memoryPoolB);
   VMACCEL_LOG("  Semaphore Memory Pool:       %d\n", memoryPoolS);
   VMACCEL_LOG("  Number of Iterations:        %d\n", numIterations);
   VMACCEL_LOG("\n");

   numSubDevices = MAX(kernelDevice + 1, MEMDEVICE(memoryPoolA) + 1);
   numSubDevices = MAX(numSubDevices, MEMDEVICE(memoryPoolB) + 1);
   numSubDevices = MAX(numSubDevices, MEMDEVICE(memoryPoolS) + 1);

   address mgrAddr(host);
   work_topology workTopology({0}, {numRows}, {numColumns});
   ref_object<accelerator> accel(new accelerator(mgrAddr));

   /*
    * Initialize the Compute Kernel.
    */
   compute::kernel k(VMCL_IR_NATIVE, functionTable[kernelFunc].source);

   /*
    * Setup the working set.
    */
   ref_object<int> memA(new int[numRows * numColumns * chunkSize],
                        sizeof(int) * numRows *numColumns *chunkSize,
                        VMACCEL_SURFACE_USAGE_READWRITE);
   ref_object<int> memB(new int[numRows * numColumns * chunkSize],
                        sizeof(int) * numRows *numColumns *chunkSize,
                        VMACCEL_SURFACE_USAGE_READWRITE);
   ref_object<int> memS(new int[numRows * numColumns],
                        sizeof(int) * (numRows * numColumns + 1),
                        VMACCEL_SURFACE_USAGE_READWRITE);
   ref_object<int> memDims(new int[4], sizeof(int) * 4,
                           VMACCEL_SURFACE_USAGE_READWRITE);

   memDims[0] = numRows;
   memDims[1] = numColumns;
   memDims[2] = chunkSize;
   memDims[3] = numPasses;

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
                         numSubDevices, 0);

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
      accelerator_surface accelA(accel.get(), MEMDEVICE(memoryPoolA), descA);
      accelerator_surface accelB(accel.get(), MEMDEVICE(memoryPoolB), descB);
      accelerator_surface accelS(accel.get(), MEMDEVICE(memoryPoolS), semDesc);
      accelerator_surface accelDims(accel.get(), kernelDevice, descDims);

      VMAccelSurfaceRegion rgn = {
         0, {0, 0, 0}, {numRows * numColumns * chunkSize, 0, 0}};
      if (accelA->upload<int>(rgn, memA) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload A\n");
         return VMACCEL_FAIL;
      }
      if (accelB->upload<int>(rgn, memB) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload B\n");
         return VMACCEL_FAIL;
      }

      VMAccelSurfaceRegion rgnSem = {
         0, {0, 0, 0}, {numRows * numColumns, 0, 0}};

      if (accelS->upload<int>(rgnSem, memS) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload semaphores\n");
         return VMACCEL_FAIL;
      }

      VMAccelSurfaceRegion rgnDims = {0, {0, 0, 0}, {4, 0, 0}};
      if (accelDims->upload<int>(rgnDims, memDims) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to upload dims\n");
         return VMACCEL_FAIL;
      }

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
         return VMACCEL_FAIL;
      }

      c->upload_surface(bindA->get_surf());
      c->upload_surface(bindB->get_surf());
      c->upload_surface(bindS->get_surf());
      c->upload_surface(bindDims->get_surf());

      clock_gettime(CLOCK_REALTIME, &e2eStartTime);

      for (int iter = 0; iter < numIterations; iter++) {
         if (kernelFunc == MEMCPY) {
            VMAccelSurfaceRegion copyRgn = {
               0, {0, 0, 0}, {numRows * numColumns * chunkSize * sizeof(int), 0, 0}};
            c->copy_surface(0, bindA->get_surf(), copyRgn, bindB->get_surf(), copyRgn);
         } else {
            ref_object<compute::operation> opobj;

            if (kernelFunc == MATRIX_ADD_2D_SEMAPHORE1U ||
                kernelFunc == MATRIX_ADD_2D_SEMAPHORENU ||
                kernelFunc == MATRIX_ADD_2D_EXEC_CHAIN) {
               // Clear the semaphores
               if (accelS->upload<int>(rgnSem, memS) != VMACCEL_SUCCESS) {
                  VMACCEL_LOG("ERROR: Unable to reset semaphores\n");
                  return VMACCEL_FAIL;
               }
            }

            compute::dispatch<
               ref_object<vmaccel::binding>, ref_object<vmaccel::binding>,
               ref_object<vmaccel::binding>, ref_object<vmaccel::binding>>(
               c, kernelDevice, opobj, VMCL_OPENCL_C_1_0, k,
               functionTable[kernelFunc].function, workTopology, bindA, bindB,
               bindS, bindDims);
         }
      }

      if (kernelFunc == MEMCPY) {
         // Manually download the surface from the context
         c->download_surface(bindB->get_surf());
      }

      if (accelB->download<int>(rgn, memB) != VMACCEL_SUCCESS) {
         VMACCEL_LOG("ERROR: Unable to readback B\n");
         return VMACCEL_FAIL;
      }

      clock_gettime(CLOCK_REALTIME, &e2eEndTime);
      e2eDiffTime = DiffTime(&e2eStartTime, &e2eEndTime);
      totalRuntimeMS =
         e2eDiffTime.tv_sec * 1000.0f +
         ((e2eDiffTime.tv_nsec != 0) ? (double)e2eDiffTime.tv_nsec / 1000000.0f
                                     : 0.0f);

      size_t passBytes = (size_t)numRows * numColumns * chunkSize * sizeof(int);
      size_t iterationBytes = passBytes * numPasses;
      size_t totalComputeBytes = iterationBytes * numIterations;
      size_t totalTransferredBytes =
         passBytes * 3 + numRows * numColumns * sizeof(int);
      VMACCEL_LOG("\n");
      VMACCEL_LOG("End-to-end Time = %lf ms\n", totalRuntimeMS);
      VMACCEL_LOG("Total Referenced = %ld bytes\n", totalComputeBytes);
      VMACCEL_LOG("Total Dirtied = %ld bytes\n", totalComputeBytes);
      VMACCEL_LOG("Total Uploaded = %ld bytes\n",
                  passBytes * 2 + numRows * numColumns * sizeof(int));
      VMACCEL_LOG("Total Downloaded = %ld bytes\n", passBytes);
      VMACCEL_LOG("Compute Throughput = %lf bytes/ms\n",
                  totalComputeBytes / totalRuntimeMS);
      VMACCEL_LOG("\n");

      for (int i = 0; i < numRows; i++) {
         for (int j = 0; j < numColumns; j++) {
            for (int k = 0; k < chunkSize; k++) {
               int val = memB[i * numColumns * chunkSize + j * chunkSize + k];
               int exp;

               if (kernelFunc == MATRIX_COPY_2D || kernelFunc == MEMCPY) {
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

   return 1;
}
