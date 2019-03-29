

# vmaccel - VMware Interface for Accelerator APIs

## Overview

VMware Interface for Accelerator APIs enables day-zero usage of new Accelerator
APIs, by utilizing a client/server model. Typically, you would place your
server on a host or VM that has support for a given Accelerator API. A client
could live remotely on another host or inside a VM, providing access to a
non-local Accelerator before a formal virtualized device can be derived.

Each Accelerator's API is abstracted into a client/server protocol with remote
execution in mind, see the "specs" directory for each accelerator. The protocol
should contain atomic operations and as little implied or tracked state on the
server as possible.

## Try it out

### Prerequisites

* cmake 3.4.3 or newer
* MacOSX
  * macOS 10.13.4 or newer (note: OpenCL is deprecated as of 10.14)
  * XCode Version 9.3 or newer (w/command line tools)
* Linux
  * Ubuntu 16.04 or newer
    * Developer tools, gcc/g++ (e.g. build-essential g++)
    * OpenCL Libraries and Headers (Intel OpenCL 2.1+ SDK recommended)
    * python with distutils package (e.g. python-distutils-extra)
    * rpcgen 2.23+
    * rpcbind (for server/client communication)
      * ```systemctl add-wants multi-user.target rpcbind```

### Build & Run

The following steps assume paths relative to the project's root directory.

1. Setup the external modules

``` shell
    $ git submodule init
    $ git submodule update
    $ cd external/spirv-llvm/tools
    $ git clone -b spirv-1.1 https://github.com/KhronosGroup/SPIR clang
```

2. To build binaries, launch the make command

``` shell
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
```

3. Once built, the following directories will be populated:

* build/bin - Executables for each component
* build/external - External project build targets
* build/lib - Libraries for each component
* build/inc - Headers used for the libraries of each component
* build/specs - Spec files for use with the libraries and headers
* build/gen - Auto-generated files for the protocol specifications
* build/test - Compiled unit tests for the framework
* build/examples - Compiled examples for the framework

To launch one of the accelerators, you'll need two shell instances:

In one shell instance, launch the server as follows:

``` shell
$ build/bin/<accelerator>_svr
```

In the other shell instance, launch the client as follows:

``` shell
$ build/bin/<accelerator>_clnt 127.0.0.1
```

Example:

``` shell
  Shell 1 $ build/bin/vmcl_svr
  Shell 2 $ build/bin/vmcl_clnt 127.0.0.1
```

## Documentation

### Design Overview - Backend -> Server -> Client -> Frontend -> Application

To avoid instability due to over-commit, each Backend should be exclusive
to a given Server. The Backend is responsible for exporting a reasonable
capacity target for the workloads from a Client/Frontend/Application.
Assuming stability of the overall system, we can use Little's Law to
intelligently size resource usage and schedule queue based workloads.

In the context of Little's Law, when there is no contention:

&nbsp; W = T(execute task)

When there is contention we have:

&nbsp; T(context switch) = T(page-off) + T(reassign resource) + T(page-in)

&nbsp; W = T(extent) = T(context switch) + T(execute task)

If you have i-number of parallel processing units greater than the number of
pending queue entries, Total Execution Time is:

&nbsp; Total Execution Time = Max(T(extent[1]), T(extent[2]), ..., T(extent[i]))

When your arrival rate is greater than your service rate for the queue,
e.g. processor over-commit, there is a hidden serialization that is
created, breaking the possibility for parallelism:

&nbsp; Total Execution Time = Sum(T(extent[1]), T(extent[2]), ..., T(extent[i]))

The above assumes no pre-emption, since pre-emption could indirectly
result in T(context switch). Furthermore, resource contention has a
cascading feedback to all producer layers higher in the system stack.
To minimize this backpressure, we push consolidation functionality as
high up the stack as possible. Below are design characteristics to
achieve a stable system, with a goal of consolidation and ~99%
utilization.

#### Protocol (->):
1. Runtime complexity for any command in the protocol is bounded and
   predictable. Without predictable runtime complexity, it is difficult
   to discern between unbounded execution, device failure, communication
   failure, and semantic errors.
2. Each command is atomic, and can be interrupted depending on support
   from the Backend.
3. Each command is preferably stateless, to avoid expansion of runtime and
   storage complexity from tracking implicit state for the Accelerator.

#### Backend - *_ops -> ... -> Server:
1. *_ops contains the lowest level interface to the Host's Accelerator API.
2. *_ops is only responsible for one translation of Accelerator State to,
   the Host Accelerator API, instantiating Operations, and communicating
   an error.
3. *_ops does not validate state or Identifier Database usage.
4. Translation, State Tracking, Context Switching, simple Over-Commit, etc.
   must be handled in layers of Backend dispatch above *_ops (e.g. ...).

#### Server:
1. Handles retrieval of a workload from a Client.
2. Manages mutual exclusion properties for the Backend, to avoid unbounded
   runtime complexity.
3. Communicates workload to the top level Backend dispatch.
4. Will issue a callback for registered Events, if the protocol supports
   this.

#### Client:
1. Communicates a workload from the Frontend to the Server.
2. Listens for callbacks from server, if the protocol supports this.

#### Frontend:
1. Abstracts the communication from the Application to the Host's Backend.
2. Handles Migration and Load Balancing to avoid expansion of runtime
   and storage complexity for a given Server, this avoids blocking clients
   for unbounded management tasks that add to resource contention latency.
3. Exposes either a low-level Accelerator API or a high-level Managed API,
   e.g. VMAccel.
4. The Managed API abstraction allocates Accelerator resources on-demand,
   and maintains a stateless design.

### Memory Model

#### Application Memory

Due to the remote procedure call (RPC) abstraction, the memory model must
handle the asynchronous access of a network device. The lifetime of an object
is determined not only by the scope of the caller, but the operation itself.
Since an operation's asynchronous execution window may reference an object
at any given time, the vmaccel::ref_object class was created with the
following semantics, implemented by usage of std::shared_ptr:

1. Reference count is incremented for the producer and consumer when handing
   an object to the consumer.
2. Reference count is decremented for the producer within the scope of the
   application code.
3. Reference count is decremented for the consumer when the consumer has
   either copied the contents or an operation has completed and control is
   returned to the caller, e.g. completion of enqueueing the associated
   RPC Call from a client.

Example:

``` shell
   {
      std::shared_ptr<char> obj;                     // obj.REFCOUNT == 1
      std::shared_ptr<char> obj2;                    // obj2.REFCOUNT == 1
      vmaccel::ref_object<char> ref(obj, ...);       // obj.REFCOUNT == 2
      ...
      {
         vmaccel::ref_object<char> ref2(obj2, ...);  // obj2.REFCOUNT == 2
         RPC Call(obj, obj2);
         ...
      }                                              // obj2.REFCOUNT == 1
   }                                                 // obj and obj2 deleted
```

The above keeps an allocation alive within the context of an operation's time
within a queue, e.g. queue extent. C++ provides a construct in std::shared_ptr,
which a client *MUST* use with vmaccel::ref_object to retain an allocation.

#### RPC Memory

RPC Memory can be considered a transient storage. One can enqueue multiple
objects over the wire, but until they reach their destination they take no
memory on the destination. This property lends to a producer/consumer
over-commit, and relies on detecting backpressure to avoid a deinal of service
attack.

In the context of the rpcgen model, we will denote handoff of completed
contents through the protocol as "==>" and "~~" as wire transmission of
the content. Contents of memory are passed between abstraction layers
for a VMAccel Client as follows:

``` shell
  std::shared_ptr ==> ref_object<std::shared_ptr> ==> RPC Call (*_clnt.c)
  ~~
  RPC Results ==> temp allocation ==> caller copies content and frees
```

Contents of memory are passed between abstraction layers for a VMAccel Server
as follows:

``` shell
  ~~
  Service RPC Call (*rpc_svc.c, *rpc_server.c)
     ==> temp allocation ==> server copies content and frees
  ...
  global memory
     ==> RPC Results ... deferred re-use of memory at next Service RPC Call
  ~~
```

Asynchronous content handoff will be denoted as "~>", where the completion of
transmission and receiving of memory contents is noticed through an Event or
Fence object. Below is a diagram depicting the client/server interaction in
execution order:

``` shell
  CLIENT: RPC Call
  ~~
  SERVER: Service RPC ==> Execute Asynchronous Operation ~> RPC Results
  ~~
  CLIENT: Wait or Event ...
  ~~
  SERVER: Asynchronous Operation complete, trigger Event
  ~~
  CLIENT: Event Triggered ==> caller copies content and frees
```

### Auto-generated Files
1. Auto-generated files are placed in build/gen
2. Header files should be copied as follows

``` shell
    $ cp build/gen/*.h common/inc/
```

3. *_rpc_xdr.c files are the RPC translation files for declared structures.
   Copy them as follows:

``` shell
    $ cp build/gen/<accelerator>_rpc_xdr.c accelerators/<accelerator>/src/
```

## Releases & Major Branches

## Contributing

The vmaccel project team welcomes contributions from the community. Before you start working with vmaccel, please read our [Developer Certificate of Origin](https://cla.vmware.com/dco). All contributions to this repository must be signed as described on that page. Your signature certifies that you wrote the patch or have the right to pass it on as an open-source patch. For more detailed information, refer to [CONTRIBUTING.md](CONTRIBUTING.md).

## Acknowledgements

Special thanks to the following people for their contributions during Borathon:

- Neha Bhende
- Charmaine Lee
- Deepak Singh Rawat
- Sinclair Yeh

## License

VMware Interface for Accelerator APIs
Copyright (c) 2019 VMware, Inc.  All rights reserved				

The BSD-2 license (the "License") set forth below applies to all parts of the VMware Interface for Accelerator APIs project.  You may not use this file except in compliance with the License.

BSD-2 License 

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
