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

#include "vmaccel_stream.h"
#include "log_level.h"
#include "vmaccel_rpc.h"
#include "vmaccel_types_address.h"
#include "vmaccel_utils.h"
#include <arpa/inet.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define TCP_RCV_BUFFER_SIZE 1024 * 128
#define TCP_SND_BUFFER_SIZE 1024 * 16

volatile int g_init = 0;
volatile int g_initClntThreads = 0;
volatile int g_initSvrThreads = 0;
volatile int g_exitClntThreads = 0;
volatile int g_exitSvrThreads = 0;
IdentifierDB *g_svrDB[VMACCEL_STREAM_TYPE_MAX];
int g_clntFD[VMACCEL_STREAM_TYPE_MAX][VMACCEL_MAX_STREAMS];
int g_svrFD[VMACCEL_STREAM_TYPE_MAX][VMACCEL_MAX_STREAMS];
pthread_t g_clntThread[VMACCEL_STREAM_TYPE_MAX][VMACCEL_MAX_STREAMS];
pthread_t g_svrThread[VMACCEL_STREAM_TYPE_MAX][VMACCEL_MAX_STREAMS];

DECLARE_TIME_STAT(vmaccel_stream_send_async);
DECLARE_TIME_STAT(StreamTCPServerThread);
DECLARE_TIME_STAT(StreamTCPServerThread_VMACCEL_STREAM_TYPE_VMCL_UPLOAD);
DECLARE_TIME_STAT(StreamTCPClientThread);
DECLARE_TIME_STAT(StreamTCPClientSend);
DECLARE_COUNTER_STAT(RxBytesPerSend);
DECLARE_COUNTER_STAT(RxPassesPerSend);
DECLARE_COUNTER_STAT(TxBytesPerSend);
DECLARE_COUNTER_STAT(TxPassesPerSend);

/**
 * Modeled after
 * http://man7.org/linux/man-pages/man3/pthread_setschedparam.3.html
 */
static void LogThreadScheduleAttr(const char *prefix, int policy,
                                  struct sched_param *param) {
   VMACCEL_LOG("%s: policy=%s, priority=%d\n", prefix,
               (policy == SCHED_FIFO)
                  ? "SCHED_FIFO"
                  : (policy == SCHED_RR)
                       ? "SCHED_RR"
                       : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "?",
               param->sched_priority);
}

int vmaccel_stream_poweron() {
   VMACCEL_LOG("%s: Stream module poweron\n", __FUNCTION__);

   for (int j = 0; j < VMACCEL_STREAM_TYPE_MAX; j++) {
      g_svrDB[j] = IdentifierDB_Alloc(VMACCEL_MAX_STREAMS);
      for (int i = 0; i < VMACCEL_MAX_STREAMS; i++) {
         g_clntFD[j][i] = -1;
         g_svrFD[j][i] = -1;
         g_clntThread[j][i] = 0;
         g_svrThread[j][i] = 0;
      }
   }

   g_init = 1;

   return VMACCEL_SUCCESS;
}


static void ConfigureSocket(int sockFD, int reqRcvBufSize, int reqSndBufSize) {
   int rcvBufSize, sndBufSize;
   socklen_t len = sizeof(int);

   if (getsockopt(sockFD, SOL_SOCKET, SO_RCVBUF, (char *)&rcvBufSize, &len) ==
       0) {
#if DEBUG_STREAMS
      VMACCEL_LOG("TCP Receive Buffer Size: %d\n", rcvBufSize);
#endif
      if (rcvBufSize != reqRcvBufSize) {
         if (setsockopt(sockFD, SOL_SOCKET, SO_RCVBUF, (char *)&reqRcvBufSize,
                        len) == 0) {
            VMACCEL_LOG("TCP Receive Buffer Size: %d -> %d\n", rcvBufSize,
                        reqRcvBufSize);
         }
      }
   } else {
      VMACCEL_WARNING("Unable to query TCP receive buffer size\n");
   }

   if (getsockopt(sockFD, SOL_SOCKET, SO_SNDBUF, (char *)&sndBufSize, &len) ==
       0) {
#if DEBUG_STREAMS
      VMACCEL_LOG("TCP Send Buffer Size: %d\n", sndBufSize);
#endif
      if (sndBufSize != reqSndBufSize) {
         if (setsockopt(sockFD, SOL_SOCKET, SO_SNDBUF, (char *)&reqSndBufSize,
                        len) == 0) {
            VMACCEL_LOG("TCP Send Buffer Size: %d -> %d\n", sndBufSize,
                        reqSndBufSize);
         }
      }
   } else {
      VMACCEL_WARNING("Unable to query TCP send buffer size\n");
   }

#ifdef TCP_NODELAY
   int noDelay;

   if (getsockopt(sockFD, IPPROTO_TCP, TCP_NODELAY, (char *)&noDelay, &len) ==
       0) {
      VMACCEL_LOG("TCP No Delay: %d\n", noDelay);
   } else {
      VMACCEL_WARNING("Unable to query TCP No Delay setting\n");
   }
#endif
}


static void *StreamTCPServerThread(void *args) {
   VMAccelStreamContext *s = (VMAccelStreamContext *)args;
   VMAccelStreamPacket p = {
      0,
   };
   int svrFD, clntFD, rxSize, winSize;
#ifdef TCP_NODELAY
   int noDelay;
#endif
   struct sockaddr_in svr, clnt;
   int port = s->stream.accel.port + s->stream.index;
   socklen_t len = sizeof(int);
   int policy, ret;
   struct sched_param param;
   START_TIME_STAT(StreamTCPServerThread);

   ret = pthread_getschedparam(pthread_self(), &policy, &param);

   if (ret == 0) {
      LogThreadScheduleAttr(__FUNCTION__, policy, &param);
   }

   svrFD = socket(AF_INET, SOCK_STREAM, 0);
   if (svrFD == -1) {
      VMACCEL_WARNING("Unable to create socket for TCP stream server\n");
      VMACCEL_WARNING("  Port: %d\n", port);
      END_TIME_STAT(StreamTCPServerThread);
      return args;
   }

   VMACCEL_LOG("Created TCP stream server type=%d, port=%d...\n",
               s->stream.type, port);

   svr.sin_family = AF_INET;
   svr.sin_addr.s_addr = INADDR_ANY;
   svr.sin_port = htons(port);

   if (bind(svrFD, (struct sockaddr *)&svr, sizeof(svr)) < 0) {
      VMACCEL_WARNING("Unable to bind socket for TCP stream server\n");
      VMACCEL_WARNING("  Port: %d\n", port);
      close(svrFD);
      END_TIME_STAT(StreamTCPServerThread);
      return args;
   }

   VMACCEL_LOG("TCP bind done for stream server type=%d...\n", s->stream.type);

   ConfigureSocket(svrFD, TCP_RCV_BUFFER_SIZE, TCP_SND_BUFFER_SIZE);

   g_svrFD[s->stream.type][s->stream.index] = svrFD;

   g_initSvrThreads++;

   while (g_exitSvrThreads == 0) {
#if DEBUG_STREAMS
      VMACCEL_LOG("Listening for connection %d\n", svrFD);
#endif

      if (listen(svrFD, 3) != 0) {
         VMACCEL_WARNING("Failed to listen\n");
         goto exit_server;
      }

#if DEBUG_STREAMS
      VMACCEL_LOG("Accepting connection %d\n", svrFD);
#endif

      clntFD = accept(g_svrFD[s->stream.type][s->stream.index],
                      (struct sockaddr *)&clnt, &len);

      if (clntFD < 0) {
         VMACCEL_WARNING("Unable to accept client connection\n");
         goto exit_server;
      }

#if DEBUG_STREAMS
      VMACCEL_LOG("Accepted client connection %d\n", clntFD);
#endif

      while (g_exitSvrThreads == 0 &&
             (rxSize = recv(clntFD, &p, sizeof(VMAccelStreamPacket), 0)) > 0) {
         VMCLSurfaceUnmapOp unmapOp;
         VMAccelSurfaceMapStatus *mapStatus;
         size_t rxLen = p.len;
         size_t rxOffset = 0;
         char *memPtr = NULL;

         if (p.type == VMACCEL_STREAM_TYPE_VMCL_UPLOAD) {
            VMCLSurfaceUnmapOp unmapOp;
            unsigned int numPasses = 0;
            START_TIME_STAT(
               StreamTCPServerThread_VMACCEL_STREAM_TYPE_VMCL_UPLOAD);

            memset(&unmapOp, 0, sizeof(unmapOp));

            mapStatus = s->cb.clSurfacemap_1(&p.desc.cl);

            unmapOp.queue = p.desc.cl.queue;
            unmapOp.op.mapFlags = p.desc.cl.op.mapFlags;
            unmapOp.op.mapFlags |= VMACCEL_MAP_NO_FREE_PTR_FLAG;
            unmapOp.op.surf = p.desc.cl.op.surf;
            unmapOp.op.ptr.ptr_len = mapStatus->ptr.ptr_len;
            unmapOp.op.ptr.ptr_val = mapStatus->ptr.ptr_val;

#if DEBUG_STREAMS
            VMACCEL_LOG("Stream[%d][%d]: Type=%d Rx %d bytes\n", s->stream.type,
                        s->stream.index, p.type, p.len);
            VMACCEL_LOG(
               "Stream[%d][%d]: Type=%d Mapped sid=%d, generation=%d -> %p len=%d\n",
               s->stream.type, s->stream.index, p.type, p.desc.cl.op.surf.id,
               p.desc.cl.op.surf.generation,
               mapStatus->ptr.ptr_val, mapStatus->ptr.ptr_len);
#endif

            if (rxLen > mapStatus->ptr.ptr_len) {
               VMACCEL_WARNING("Stream[%d][%d]: Overflow detected\n", s->stream.type,
                               s->stream.index);
               rxLen = -1;
               END_TIME_STAT(
                  StreamTCPServerThread_VMACCEL_STREAM_TYPE_VMCL_UPLOAD);
               break;
            }

            while (g_exitSvrThreads == 0 && rxLen > 0) {
               rxSize = recv(clntFD, mapStatus->ptr.ptr_val + rxOffset,
                             rxLen, 0);

#if DEBUG_STREAMS
               VMACCEL_LOG("Stream[%d][%d]: exit=%d rxOffset=%ld rxLen=%ld, rxSize=%d\n",
                           s->stream.type, s->stream.index, g_exitSvrThreads,
                           rxOffset, rxLen, rxSize);
#endif
               if (rxSize <= 0) {
                  break;
               }
               if (rxLen - rxSize < 0) {
                  rxLen = -1;
                  break;
               }
               rxLen -= rxSize;
               rxOffset += rxSize;
               numPasses++;
            }

            s->cb.clSurfaceunmap_1(&unmapOp);

            INC_COUNTER_STAT(RxBytesPerSend, p.len);
            INC_COUNTER_STAT(RxPassesPerSend, numPasses);

#if DEBUG_STREAMS
            VMACCEL_LOG("Stream[%d][%d]: Type=%d Rx complete in %d passes\n",
                        s->stream.type, s->stream.index, p.type,
                        numPasses);
#endif

            if (rxLen < 0) {
               VMACCEL_WARNING("Overflow detected\n");
               END_TIME_STAT(
                  StreamTCPServerThread_VMACCEL_STREAM_TYPE_VMCL_UPLOAD);
               break;
            }
         } else {
            VMACCEL_WARNING("Unknown VMAccelStreamPacket type 0x%x\n", p.type);
            break;
         }
      }

      close(clntFD);
   }

exit_server:

   VMACCEL_LOG("Exiting TCP stream server thread for port %d\n", port);
   assert(g_svrFD[s->stream.type][s->stream.index] != -1);
   close(g_svrFD[s->stream.type][s->stream.index]);
   g_svrFD[s->stream.type][s->stream.index] = -1;
   IdentifierDB_ReleaseId(g_svrDB[s->stream.type], s->stream.index);

   if (g_exitSvrThreads > 0) {
      g_exitSvrThreads--;
   } else {
      g_initSvrThreads--;
   }

   END_TIME_STAT(StreamTCPServerThread);

   return args;
}


int vmaccel_stream_server(unsigned int type, unsigned int port,
                          VMAccelStreamCallbacks *cb) {
   int policy, ret;
   struct sched_param param;

   ret = pthread_getschedparam(pthread_self(), &policy, &param);

   if (ret == 0) {
      LogThreadScheduleAttr(__FUNCTION__, policy, &param);
   }

   if (g_init == 0) {
      VMACCEL_WARNING("%s: vmaccel_stream_poweron not called...\n",
                      __FUNCTION__);
      return VMACCEL_FAIL;
   }

   for (int i = 0; i < VMACCEL_MAX_STREAMS; i++) {
      if (IdentifierDB_AcquireId(g_svrDB[type], i)) {
         VMAccelStreamContext *s = malloc(sizeof(VMAccelStreamContext));
         pthread_t t;

         s->stream.type = type;
         s->stream.index = i;
         s->stream.accel.port = port;
         s->cb = *((VMAccelStreamCallbacks *)cb);

         if (pthread_create(&t, NULL, StreamTCPServerThread, s)) {
            VMACCEL_WARNING("Unable to create thread for stream\n");
            return VMACCEL_FAIL;
         }

         g_svrThread[type][i] = t;
      }
   }

   return VMACCEL_SUCCESS;
}


static int StreamTCPClientSend(VMAccelStreamSend *s) {
   VMAccelStreamPacket p = {
      0,
   };
   size_t txLen, txSize, numPasses = 0;
   START_TIME_STAT(StreamTCPClientSend);

   // Find socket...
   if (g_clntFD[s->type][s->index] == -1) {
      VMACCEL_WARNING("No server socket open\n");
      END_TIME_STAT(StreamTCPClientSend);
      return VMACCEL_FAIL;
   }

   p.type = s->type;
   p.desc.cl = s->desc.cl;
   p.len = s->ptr.ptr_len;

   // Mutex the client FD for the time of the transmission
   if (send(g_clntFD[s->type][s->index], &p, sizeof(p), 0) != sizeof(p)) {
      VMACCEL_WARNING("Unable to send packet type=%d index=%d\n", s->type, s->index);
      END_TIME_STAT(StreamTCPClientSend);
      return VMACCEL_FAIL;
   }

#if DEBUG_STREAMS
   VMACCEL_LOG("Stream[%d][%d]: Client Tx len=%d\n", s->type, s->index, p.len);
#endif

   txLen = s->ptr.ptr_len;

   while (g_exitClntThreads == 0 && txLen > 0 &&
          (txSize = send(g_clntFD[s->type][s->index], s->ptr.ptr_val, txLen,
                         0) < 0)) {
      txLen -= txSize;
      numPasses++;
   }

   INC_COUNTER_STAT(TxBytesPerSend, s->ptr.ptr_len);
   INC_COUNTER_STAT(TxPassesPerSend, numPasses);

   END_TIME_STAT(StreamTCPClientSend);

   return VMACCEL_SUCCESS;
}


static void *StreamTCPClientThread(void *args) {
   VMAccelStreamSend *s = (VMAccelStreamSend *)args;
   int policy, ret;
   struct sched_param param;
   START_TIME_STAT(StreamTCPClientThread);

#if DEBUG_STREAMS
   VMACCEL_LOG("%s: Starting thread type=%d index=%d\n", __FUNCTION__,
               s->type, s->index);
#endif

   ret = pthread_getschedparam(pthread_self(), &policy, &param);

   if (ret != 0) {
      VMACCEL_WARNING("Unable to get thread scheduling params\n");
      END_TIME_STAT(StreamTCPClientThread);
      return args;
   }
#if DEBUG_STREAMS
   else {
      LogThreadScheduleAttr(__FUNCTION__, policy, &param);
   }
#endif

   if (g_clntFD[s->type][s->index] == -1) {
      char host[4 * VMACCEL_MAX_LOCATION_SIZE];

      sprintf(host, "127.0.0.1");

      if (!VMAccel_AddressOpaqueAddrToString(&s->accel, host, sizeof(host))) {
         VMACCEL_WARNING("Unable to translate VMAccelAddress host=%s\n", host);
         IdentifierDB_ReleaseId(g_svrDB[s->type], s->index);
         END_TIME_STAT(StreamTCPClientThread);
         return args;
      }

      VMACCEL_LOG("%s: Connecting to %s\n", __FUNCTION__, host);

      struct sockaddr_in clnt;
      unsigned int clntFD;

      clntFD = socket(AF_INET, SOCK_STREAM, 0);

      if (clntFD == -1) {
         VMACCEL_WARNING("Server connection unavailable.\n");
         IdentifierDB_ReleaseId(g_svrDB[s->type], s->index);
         END_TIME_STAT(StreamTCPClientThread);
         return args;
      }

      clnt.sin_addr.s_addr = inet_addr(host);
      clnt.sin_family = AF_INET;
      clnt.sin_port = htons(s->accel.port + s->index);

      ConfigureSocket(clntFD, TCP_RCV_BUFFER_SIZE, TCP_SND_BUFFER_SIZE);

      if (connect(clntFD, (struct sockaddr *)&clnt, sizeof(clnt)) < 0) {
         VMACCEL_WARNING("Unable to connect to server %s:%d\n",
                         host, s->accel.port + s->index);
         close(clntFD);
         clntFD = -1;
         IdentifierDB_ReleaseId(g_svrDB[s->type], s->index);
         END_TIME_STAT(StreamTCPClientThread);
         return args;
      }

      g_clntFD[s->type][s->index] = clntFD;
   } else {
#if DEBUG_STREAMS
      VMACCEL_WARNING("Re-using server connection.\n");
#endif
   }

   g_initClntThreads++;

   StreamTCPClientSend(s);

   IdentifierDB_ReleaseId(g_svrDB[s->type], s->index);

#if DEBUG_STREAMS
   // Do not re-use the connection
   close(g_clntFD[s->type][s->index]);
   g_clntFD[s->type][s->index] = -1;
   VMACCEL_LOG("%s: Exiting thread\n", __FUNCTION__);
#endif

   if (g_exitClntThreads > 0) {
      g_exitClntThreads--;
   } else {
      g_initClntThreads--;
   }

   END_TIME_STAT(StreamTCPClientThread);

   return args;
}


int vmaccel_stream_send_async(VMAccelAddress *a, unsigned int type, void *args,
                              char *ptr_val, size_t ptr_len) {
   VMAccelStreamSend *s = malloc(sizeof(VMAccelStreamSend));
   void *res = NULL;
   unsigned int index = 0;
   int policy, ret;
   struct sched_param param;
   START_TIME_STAT(vmaccel_stream_send_async);

   VMACCEL_LOG("%s: addr_val=%p addr_len=%d\n", __FUNCTION__,
               a->addr.addr_val, a->addr.addr_len);

   ret = pthread_getschedparam(pthread_self(), &policy, &param);

   if (ret != 0) {
      VMACCEL_WARNING("Unable to read schedule parameters\n");
      END_TIME_STAT(vmaccel_stream_send_async);
      return VMACCEL_FAIL;
   }
#if DEBUG_STREAMS
   else {
      LogThreadScheduleAttr(__FUNCTION__, policy, &param);

      param.sched_priority = sched_get_priority_min(SCHED_RR);

      if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) != 0) {
         VMACCEL_WARNING("Unable to change scheduling policy\n");
      }

      LogThreadScheduleAttr(__FUNCTION__, policy, &param);
   }
#endif

   if (g_init == 0) {
      VMACCEL_WARNING("%s: vmaccel_stream_poweron not called...\n",
                      __FUNCTION__);
      assert(0);
   }

   if (!IdentifierDB_AllocId(g_svrDB[type], &index)) {
      VMACCEL_LOG("%s: Identifier unavailable, flushing active"
                  " client threads...\n",
                  __FUNCTION__);
      for (int i = 0; i < VMACCEL_MAX_STREAMS; i++) {
         if (pthread_join(g_clntThread[type][i], &res) != 0) {
            VMACCEL_WARNING("Unable to join thread 0x%lx\n",
                            g_clntThread[type][i]);
            END_TIME_STAT(vmaccel_stream_send_async);
            return VMACCEL_FAIL;
         } else {
            index = i;
            break;
         }
      }

      ret = IdentifierDB_AcquireId(g_svrDB[type], index);
      if (!ret) {
         VMACCEL_WARNING("%s: Unable to acquire ID on retry\n", __FUNCTION__);
         assert(0);
      }
   }

   pthread_t t;
   pthread_attr_t attr;
   s->accel = *a;
   s->type = type;
   s->index = index;
   s->desc.cl = *((VMCLSurfaceMapOp *)args);

#if DEBUG_STREAMS
   VMACCEL_LOG("%s: Sending update type=%d sid=%d gen=%d\n", __FUNCTION__, type,
               s->desc.cl.op.surf.id, s->desc.cl.op.surf.generation);
#endif

   // The following must be free for use after this call has exited.
   s->ptr.ptr_len = ptr_len;
   s->ptr.ptr_val = ptr_val;

#if ENABLE_ROUND_ROBIN_STREAM_SCHEDULING
   policy = SCHED_RR;
   param.sched_priority += VMACCEL_STREAM_PRIORITY_DELTA;

   if (pthread_attr_init(&attr) == 0 &&
       pthread_attr_setschedpolicy(&attr, policy) == 0 &&
       pthread_attr_setschedparam(&attr, &param) == 0) {
#else
   if (pthread_attr_init(&attr) == 0) {
#endif
      if (pthread_create(&t, &attr, StreamTCPClientThread, s)) {
         VMACCEL_WARNING("Unable to create thread for stream\n");
         END_TIME_STAT(vmaccel_stream_send_async);
         return VMACCEL_FAIL;
      }

#if DEBUG_STREAMS
      VMACCEL_LOG("%s: Created client thread type=%d index=%d\n",
                  __FUNCTION__, type, index);
#endif
      g_clntThread[type][index] = t;
   } else {
      VMACCEL_WARNING("Unable to set thread attributes\n");
   }

   END_TIME_STAT(vmaccel_stream_send_async);

   return VMACCEL_SUCCESS;
}


void vmaccel_stream_poweroff() {
   VMACCEL_LOG("%s: Stream module poweroff\n", __FUNCTION__);

   if ((g_initClntThreads > 0 || g_initSvrThreads > 0) &&
       (g_exitClntThreads > 0 || g_exitSvrThreads > 0)) {
      VMACCEL_WARNING("%s: Called poweroff multiple times for the process\n",
                      __FUNCTION__);
      assert(0);
   }

   g_exitClntThreads = g_initClntThreads;
   g_exitSvrThreads = g_initSvrThreads;

   while (g_exitClntThreads != 0 && g_exitSvrThreads != 0) {
      sleep(1);
   }

   for (int j = 0; j < VMACCEL_STREAM_TYPE_MAX; j++) {
      for (int i = 0; i < VMACCEL_MAX_STREAMS; i++) {
         void *res = NULL;
         if (g_clntThread[j][i] != 0) {
            VMACCEL_WARNING("Client thread type=%d, stream=%d not cleaned up\n",
                            j, i);
            if (pthread_join(g_clntThread[j][i], &res)) {
               VMACCEL_WARNING("Unable to join client thread %d,%d\n", j, i);
            } else {
               free(res);
            }
            g_clntThread[j][i] = 0;
         }

         if (g_svrThread[j][i] != 0) {
            VMACCEL_WARNING("Server thread type=%d, stream=%d not cleaned up\n",
                            j, i);
#if ENABLE_NON_BLOCKING_SOCKET
            if (pthread_join(g_svrThread[j][i], &res)) {
               VMACCEL_WARNING("Unable to join server thread %d,%d\n", j, i);
            } else {
               free(res);
            }
#endif
            g_svrThread[j][i] = 0;
         }

         if (g_svrFD[j][i] != -1) {
            VMACCEL_WARNING("Server FD type=%d, stream=%d not cleaned up\n", j,
                            i);
            close(g_svrFD[j][i]);
            g_svrFD[j][i] = -1;
         }

         if (g_svrDB[j] != NULL && IdentifierDB_ActiveId(g_svrDB[j], i)) {
            VMACCEL_WARNING("Server DB type=%d, stream=%d not cleaned up\n", j,
                            i);
            IdentifierDB_ReleaseId(g_svrDB[j], i);
         }
      }
      if (g_svrDB[j] != NULL) {
         IdentifierDB_Free(g_svrDB[j]);
      }
   }

   LOG_TIME_STAT(vmaccel_stream_send_async);
   LOG_TIME_STAT(StreamTCPServerThread);
   LOG_TIME_STAT(StreamTCPServerThread_VMACCEL_STREAM_TYPE_VMCL_UPLOAD);
   LOG_TIME_STAT(StreamTCPClientThread);
   LOG_TIME_STAT(StreamTCPClientSend);
   LOG_COUNTER_STAT(TxBytesPerSend);
   LOG_COUNTER_STAT(TxPassesPerSend);
}
