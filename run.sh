#!/bin/bash

export PATH=$PATH:/home/asic/opt/nicta/tools/gcc-3.4.4-glibc-2.3.5/arm-linux/bin/

#oklinux,oklinux2
#./tools/build.py machine=versatile project=examples example=oklinux,oklinux2 wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False profile=true

#echo,hello
#./tools/build.py machine=versatile project=examples example=echo,hello wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False profile=true

#echo,chatterbox
#./tools/build.py machine=versatile project=examples example=echo,chatterbox wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False profile=true

#soc-sdk
#./tools/build.py machine=versatile project=soc-sdk wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False profile=true

#ctest
#./tools/build.py machine=versatile project=ctest wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False profile=true

#kbench /home/asic/okl4_3.0/cells/kbench/SConscript:76 Error: type object 'versatile' has no attribute 'pmu_irq'
#./tools/build.py machine=versatile project=kbench wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False profile=true

#ktest
./tools/build.py machine=versatile project=ktest wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False profile=true
