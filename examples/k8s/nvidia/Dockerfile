FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y nvidia-opencl-dev clinfo rpcbind
COPY vmcl_svr /usr/bin/vmcl_svr
