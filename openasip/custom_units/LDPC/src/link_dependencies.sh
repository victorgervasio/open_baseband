# Override the object temporarily
P=${PWD}
cd /home/victor/openairinterface5g/cmake_targets/ran_build/build
mv CMakeFiles/ldpctest.dir/openair1/PHY/CODING/TESTBENCH/ldpctest.c.o{,.orig}
cp "${P}/ldpctest.c.o" CMakeFiles/ldpctest.dir/openair1/PHY/CODING/TESTBENCH/ldpctest.c.o
ninja ldpctest
cp ldpctest "${P}/ldpctest"
mv CMakeFiles/ldpctest.dir/openair1/PHY/CODING/TESTBENCH/ldpctest.c.o{.orig,}
cd ${P}
