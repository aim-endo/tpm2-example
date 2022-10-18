#!/bin/bash

if [ -x ./tpm2-example ]; then
  rm -f ./tpm2-example
fi

g++ main.cpp -std=c++17 -Wall -L/usr/lib/x86_64-linux-gnu -ltss2-esys -ltss2-tctildr -o tpm2-example

if [ -x ./tpm2-example ]; then
  sudo chown root:root ./tpm2-example
  sudo chmod u+s ./tpm2-example
fi

