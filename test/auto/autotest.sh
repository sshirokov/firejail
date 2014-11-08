#!/bin/bash

# remove previous reports and output file
cleanup() {
	rm -f output*
	rm -f report*
	rm -f outrep*
	rm -fr firejail-trunk
}

while [ $# -gt 0 ]; do    # Until you run out of parameters . . .
    case "$1" in
    --cleanup)
    	cleanup
    	exit
	;;
    --help)
    	echo "./autotest.sh [--cleanup|--help]"
    	exit
    	;;
    esac
    shift       # Check next set of parameters.
done


cleanup
# enable sudo
sudo ls -al

#*****************************************************************
# TEST 1
#*****************************************************************
# - checkout source code
# - check compilation,
# - install
#*****************************************************************
svn checkout svn://svn.code.sf.net/p/firejail/code-0/trunk firejail-trunk
cd firejail-trunk
./configure --prefix=/usr 2>&1 | tee ../output-configure
make -j4 2>&1 | tee ../output-make
sudo make install 2>&1 | tee ../output-install
cd test
sudo ./configure > /dev/null
cd ../..
grep warning output-configure output-make output-install > ./report-test1
grep error output-configure output-make output-install >> ./report-test1
echo
echo
echo
echo "TEST 1 Report:"
cat report-test1
echo
echo
echo

#*****************************************************************
# TEST 1.1
#*****************************************************************
# - run cppcheck
#*****************************************************************
cd firejail-trunk
cp /home/netblue/bin/cfg/std.cfg .
cppcheck --force . 2>&1 | tee ../output-cppcheck
cd ..
grep error output-cppcheck > report-test1.1
echo
echo
echo
echo "TEST 1.1 Report:"
cat report-test1.1
echo
echo
echo





#*****************************************************************
# TEST 2
#*****************************************************************
# - expect test as root, no malloc perturb
#*****************************************************************
cd firejail-trunk/test
sudo ./test-root.sh 2>&1 | tee ../../output-test2 | grep TESTING
cd ../..
grep TESTING output-test2 > ./report-test2

#*****************************************************************
# TEST 3
#*****************************************************************
# - expect test as user, no malloc perturb
#*****************************************************************
cd firejail-trunk/test
./test.sh 2>&1 | tee ../../output-test3 | grep TESTING
cd ../..
grep TESTING output-test3 > ./report-test3



#*****************************************************************
# TEST 4
#*****************************************************************
# - expect test as root, no malloc perturb
#*****************************************************************
export MALLOC_CHECK_=3
export MALLOC_PERTURB_=$(($RANDOM % 255 + 1))
cd firejail-trunk/test
sudo ./test-root.sh 2>&1 | tee ../../output-test4 | grep TESTING
cd ../..
grep TESTING output-test4 > ./report-test4

#*****************************************************************
# TEST 5
#*****************************************************************
# - expect test as user, no malloc perturb
#*****************************************************************
cd firejail-trunk/test
./test.sh 2>&1 | tee ../../output-test5 | grep TESTING
cd ../..
grep TESTING output-test5 > ./report-test5

#*****************************************************************
# PRINT REPORTS
#*****************************************************************
echo
echo
echo
echo
echo
echo
echo "TEST 1 Report:"
cat report-test1
echo "TEST 1.1 Report:"
cat report-test1.1
echo "TEST 2 Report:"
cat ./report-test2 
echo "TEST 3 Report:"
cat ./report-test3 
echo "TEST 4 Report:"
cat ./report-test4 
echo "TEST 5 Report:"
cat ./report-test5 
echo

cat report-test1 > outrep1
cat report-test1.1 > outrep1.1
grep ERROR report-test2 > outrep2
grep ERROR report-test3 > outrep3
grep ERROR report-test4 > outrep4
grep ERROR report-test5 > outrep5
wc -l outrep*
echo




exit
