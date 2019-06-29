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

#include <assert.h>
#include <iostream>
#include <stdbool.h>
#include <string>

#include "vmaccel_rpc.h"
#include "vmaccel_utils.h"

#include "log_level.h"


using namespace std;

int main(int argc, char **argv) {
   IdentifierDB *db;

   VMACCEL_LOG("%s: Running self-test of Identifier DB...\n", __FUNCTION__);

   // Allocate 3 dwords worth of data.
   db = IdentifierDB_Alloc(78);

   assert(IdentifierDB_Count(db) == 0);
   assert(IdentifierDB_Size(db) == 78);

   assert(IdentifierDB_AcquireId(db, 77) == true);

   IdentifierDB_Log(db, "ID 77");

   assert(IdentifierDB_ActiveId(db, 77) == true);
   assert(IdentifierDB_ActiveId(db, 76) == false);
   assert(IdentifierDB_ActiveId(db, 78) == false);
   assert(IdentifierDB_ActiveId(db, 128) == false);
   assert(IdentifierDB_ActiveId(db, 78) == false);

   IdentifierDB_AcquireIdRange(db, 30, 36);

   IdentifierDB_Log(db, "ID 30-36, 77");

   assert(IdentifierDB_ActiveId(db, 30) == true);
   assert(IdentifierDB_ActiveId(db, 31) == true);
   assert(IdentifierDB_ActiveId(db, 32) == true);
   assert(IdentifierDB_ActiveId(db, 33) == true);
   assert(IdentifierDB_ActiveId(db, 34) == true);
   assert(IdentifierDB_ActiveId(db, 35) == true);
   assert(IdentifierDB_ActiveId(db, 36) == true);

   IdentifierDB_ReleaseIdRange(db, 31, 33);

   IdentifierDB_Log(db, "ID 30, 34-36, 77");

   assert(IdentifierDB_ActiveId(db, 31) == false);
   assert(IdentifierDB_ActiveId(db, 32) == false);
   assert(IdentifierDB_ActiveId(db, 33) == false);

   IdentifierDB_Free(db);

   VMACCEL_LOG("%s: Self-test complete...\n", __FUNCTION__);

   return 0;
}
