#!/bin/bash

echo "TESTING: nox11"
./lkm_test_nox11.exp

echo "TESTING: nodbus"
./lkm_test_nodbus.exp

echo "TESTING: nox11, nodbus"
./lkm_test_all.exp
