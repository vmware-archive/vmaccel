#!/bin/bash

#
# Usage: setup-plugins.sh <hostname:port> <username> <password>
#

#
# Get the Jenkins JAR
#
echo "Getting Jenkins JAR from $1"
rm -rf jenkins-cli.jar
wget http://$1/jnlpJars/jenkins-cli.jar
java -jar jenkins-cli.jar -s http://$1/ -auth $2:$3 list-plugins

#
# Download the plugins and install
#
export PLUGIN_URI=http://ftp-nyc.osuosl.org/pub/jenkins
echo "Downloading plugins from $PLUGIN_URI"

declare -A plugins=(
  [apache-httpcomponents-client-4-api]=4.5.5.4.0
  [bouncycastle-api]=2.18
  [scm-api]=2.2.6
  [snakeyaml-api]=1.26.4
  [kubernetes-credentials]=0.7.0
  [structs]=1.20
  [authentication-tokens]=1.4
  [durable-task]=1.34
  [variant]=1.3
  [jackson2-api]=2.11.2
  [plain-credentials]=1.7
  [workflow-api]=2.40
  [workflow-step-api]=2.22
  [cloudbees-folder]=6.14
  [credentials]=2.3.13
  [kubernetes-client-api]=4.11.1
  [kubernetes]=1.27.5
)

for plugin in "${!plugins[@]}"; do
  version="${plugins[$plugin]}"
  echo "Downloading $plugin"
  curl -o plugins/$plugin-$version.hpi $PLUGIN_URI/plugins/$plugin/$version/$plugin.hpi
  #echo "Installing $plugin"
  #java -jar jenkins-cli.jar -s http://$1/ -auth $2:$3 install-plugin $(pwd)/plugins/$plugin-$version.hpi
done
