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

#ifndef _VMCODEC_H_
#define _VMCODEC_H_ 1

#include "vmaccel_defs.h"

#define VMCODEC_MAX_CONTEXTS 32
#define VMCODEC_MAX_SURFACES 32

enum VMCODECCapsShift {
   VMCODEC_JPEG_CAP_SHIFT = 0,
   VMCODEC_S3TC_CAP_SHIFT = 1,
   VMCODEC_MPEG_1_0_CAP_SHIFT = 2,
   VMCODEC_MPEG_2_0_CAP_SHIFT = 3,
   VMCODEC_H264_1_0_CAP_SHIFT = 4,
   VMCODEC_HEVC_1_0_CAP_SHIFT = 5,
   VMCODEC_CAP_MAX = 7,
};
typedef enum VMCODECCapsShift VMCODECCapsShift;

#define VMCODEC_JPEG_CAP (1 << VMCODEC_JPEG_CAP_SHIFT)
#define VMCODEC_S3TC_CAP (1 << VMCODEC_S3TC_CAP_SHIFT)
#define VMCODEC_MPEG_1_0_CAP (1 << VMCODEC_MPEG_1_0_CAP_SHIFT)
#define VMCODEC_MPEG_2_0_CAP (1 << VMCODEC_MPEG_2_0_CAP_SHIFT)
#define VMCODEC_H264_1_0_CAP (1 << VMCODEC_H264_1_0_CAP_SHIFT)
#define VMCODEC_HEVC_1_0_CAP (1 << VMCODEC_HEVC_1_0_CAP_SHIFT)
#define VMCODEC_CAP_MASK ((1 << VMCODEC_CAP_MAX) - 1)
typedef unsigned int VMCODECCaps;

#endif /* _VMCODEC_H_ */
