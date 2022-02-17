= Ubuntu 20.04 (Focal) Requirements = 

* 3+ Machines (VM or Native) with Ubuntu 20.04 LTS (Ansible and OpenSSH)
  - Development Machine for Ansible orchestration
  - Control Plane
  - Worker Node(s)
* Docker Repository

== Control Plane and Node OS Setup ==

i) Install Ubuntu 20.04 LTS
ii) Create <host username> and add to /etc/sudoers for password-less sudo
iii) Install NFS and OpenSSH

  $ sudo apt-get update
  $ sudo apt-get install openssh-server
  $ sudo ufw allow ssh

iv) Reboot

  $ sudo reboot now

== Development Machine Setup ==

1) Install Ansible and OpenSSH

  $ sudo apt-get update
  $ sudo apt-get install ansible openssh-client openssh-server
  $ sudo ufw allow ssh

2) Generate a key using ssh-keygen

  $ ssh-keygen

3) Execute ssh-copy-id for each of the Control Plane and Worker Nodes

  $ ssh-copy-id -i ~/.ssh/id_rsa.pub <host username>@<host ip>

== Control Plane Setup ==

Setup the Kubernetes cluster, control plane, and core compute nodes.

  devel-machine $ ansible-playbook -i hosts k8s-setup-<cri>-xenial.yml -e "ansible_user=<host username>"
  devel-machine $ ansible-playbook -i hosts control-plane-setup-<cri>.yml -e "ansible_user=<host username>"

Verify the Kubernetes Control Plane node is healthy:

  k8s-control-plane $ systemctl status <cri>
  k8s-control-plane $ systemctl status kubelet
  k8s-control-plane $ kubectl get pods --all-namespaces

== Node Setup ==

Setup the compute nodes:

  devel-machine $ ansible-playbook -i hosts node-setup-<cri>.yml -e "ansible_user=<host username>"

Verify the worker nodes are active:

  k8s-control-plane $ kubectl get nodes
  k8s-control-plane $ kubectl apply -f https://k8s.io/examples/pods/commands.yaml
  k8s-control-plane $ kubectl get pods

Verify interactive shell container:

  k8s-control-plane $ kubectl apply -f https://k8s.io/examples/application/shell-demo.yaml
  k8s-control-plane $ kubectl get pods
  k8s-control-plane $ kubectl exec --stdin --tty shell-demo -- /bin/bash

Verify the worker nodes are active with nginx:

  k8s-control-plane $ kubectl run --image=nginx nginx-server --port=80 --env="DOMAIN=cluster"
  k8s-control-plane $ kubectl expose deployment nginx-server --port=80 --name=nginx-http
  k8s-control-plane $ kubectl get svc

With worker nodes active:

  k8s-control-plane $ kubectl config view
  k8s-control-plane $ kubectl cluster-info
  k8s-control-plane $ kubectl token list

With the nodes active with CPU resources, configure accelerators using the vendor instructions.

=== AMD GPU Setup ===

  https://rocmdocs.amd.com/en/latest/Installation_Guide/Installation-Guide.html

=== Intel GPU Setup ===

  https://dgpu-docs.intel.com/installation-guides/index.html

=== NVIDIA GPU Setup ===

  https://docs.nvidia.com/datacenter/cloud-native/kubernetes/install-k8s.html#install-nvidia-dependencies

== VMCL Service ==

To start the VMCL service, start the control plane:

  devel-machine $ ansible-playbook -i hosts control-plane-start.yml -e "ansible_user=test"

Vendor specific containers for VMCL can be built by uncommenting their respective lines in
control-plane-setup-<cri>.yml. Below is a list of the image names:

  AMD - vmaccel/vmcl_server-amd
  Intel - vmaccel/vmcl_server-intel
  Mesa - vmaccel/vmcl_server-mesa
  NVIDIA - vmaccel/vmcl_server-nvidia

== Troubleshooting ==

  k8s-control-plane $ journalctl -xe
  ...
  k8s-control-plane $ systemctl restart kubelet

Corrupted images:

  k8s-control-plane $ sudo docker image prune
  k8s-control-plane $ sudo docker system prune

As a last resort, reset Kubernetes:

  devel-machine $ ansible-playbook -i hosts k8s-reset.yml -e "ansible_user=test"

If you execute "kubeadm reset", go back to the Control Plane Setup step.
