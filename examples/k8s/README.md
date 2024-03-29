# Ubuntu 20.04 (Focal) Requirements

* 3+ Machines (VM or Native) with Ubuntu 20.04 LTS (Ansible and OpenSSH)
  - Development Machine for Ansible orchestration
  - Control Plane
  - Worker Node(s)
* Docker Repository

## Control Plane and Node OS Setup

i) Install Ubuntu 20.04 LTS
ii) Create <host username> and add to /etc/sudoers for password-less sudo
iii) Install NFS and OpenSSH
``` shell
  $ sudo apt-get update
  $ sudo apt-get install openssh-server
  $ sudo ufw allow ssh
```
iv) Reboot
``` shell
  $ sudo reboot now
```

## Development Machine Setup

1) Install Ansible and OpenSSH
``` shell
  $ sudo apt-get update
  $ sudo apt-get install ansible openssh-client openssh-server
  $ sudo ufw allow ssh
```
2) Generate a key using ssh-keygen
``` shell
  $ ssh-keygen
```
3) Execute ssh-copy-id for each of the Control Plane and Worker Nodes
``` shell
  $ ssh-copy-id -i ~/.ssh/id_rsa.pub <host username>@<host ip>
```
## Control Plane Setup

Setup the Kubernetes cluster, control plane, and core compute nodes.
``` shell
  devel-machine $ ansible-playbook -i hosts k8s-setup-<cri>-xenial.yml -e "ansible_user=<host username>"
  devel-machine $ ansible-playbook -i hosts control-plane-setup-<cri>.yml -e "ansible_user=<host username>"
```
Verify the Kubernetes Control Plane node is healthy:
``` shell
  k8s-control-plane $ systemctl status <cri>
  k8s-control-plane $ systemctl status kubelet
  k8s-control-plane $ kubectl get pods --all-namespaces
```
## Node Setup

Setup the compute nodes:
``` shell
  devel-machine $ ansible-playbook -i hosts node-setup-<cri>.yml -e "ansible_user=<host username>"
```
Verify the worker nodes are active:
``` shell
  k8s-control-plane $ kubectl get nodes
  k8s-control-plane $ kubectl apply -f https://k8s.io/examples/pods/commands.yaml
  k8s-control-plane $ kubectl get pods
```
Verify interactive shell container:
``` shell
  k8s-control-plane $ kubectl apply -f https://k8s.io/examples/application/shell-demo.yaml
  k8s-control-plane $ kubectl get pods
  k8s-control-plane $ kubectl exec --stdin --tty shell-demo -- /bin/bash
```
Verify the worker nodes are active with nginx:
``` shell
  k8s-control-plane $ kubectl run --image=nginx nginx-server --port=80 --env="DOMAIN=cluster"
  k8s-control-plane $ kubectl expose deployment nginx-server --port=80 --name=nginx-http
  k8s-control-plane $ kubectl get svc
```
With worker nodes active:
``` shell
  k8s-control-plane $ kubectl config view
  k8s-control-plane $ kubectl cluster-info
  k8s-control-plane $ kubectl token list
```
With the nodes active with CPU resources, configure accelerators using the vendor instructions.

### AMD GPU Node Setup

  https://rocmdocs.amd.com/en/latest/Installation_Guide/Installation-Guide.html

### Intel GPU Node Setup

  https://dgpu-docs.intel.com/installation-guides/index.html

### NVIDIA GPU Node Setup

  https://docs.nvidia.com/datacenter/cloud-native/kubernetes/install-k8s.html#install-nvidia-dependencies

## VMCL Service

To start the VMCL service, start the control plane:
``` shell
  devel-machine $ ansible-playbook -i hosts control-plane-start.yml -e "ansible_user=test"
```
Vendor specific containers for VMCL can be built by uncommenting their respective lines in
control-plane-setup-<cri>.yml.
``` shell
  devel-machine $ cd <gpu vendor>
  devel-machine $ cp ../../../build/bin/vmcl_svr
  devel-machine $ sudo docker build --network=host -t <image name> .
  devel-machine $ sudo docker tag <image name> <my repository>/<image name>
  devel-machine $ sudo docker push <my repository>/<image name>
```
Below is a list of the image names:

  * AMD - vmcl_server:amd-gpu-<distro and version>
  * Intel - vmcl_server:intel-gpu-<distro and version>
  * Mesa - vmcl_server:mesa-<distro and version>
  * NVIDIA - vmcl_server:nvidia-gpu-<distro and version>

## Troubleshooting
``` shell
  k8s-control-plane $ journalctl -xe
  ...
  k8s-control-plane $ systemctl restart kubelet
```
Corrupted images:
``` shell
  k8s-control-plane $ sudo docker image prune
  k8s-control-plane $ sudo docker system prune
```
As a last resort, reset Kubernetes:
``` shell
  devel-machine $ ansible-playbook -i hosts k8s-reset.yml -e "ansible_user=test"
```
If you execute "kubeadm reset", go back to the Control Plane Setup step.
