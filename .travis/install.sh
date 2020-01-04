#!/bin/bash

set -ex

if [[ ${TRAVIS_OS_NAME} == "linux" ]]; then
  sudo apt-get remove libcurl4-gnutls-dev
  sudo apt-get install libcurl4-openssl-dev
fi
