#!/bin/bash

export PATH=$PATH:/home/asic/opt/nicta/tools/gcc-3.4.4-glibc-2.3.5/arm-linux/bin/

#./tools/build.py machine=versatile project=examples example=oklinux,oklinux2 wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False

#./tools/build.py machine=versatile project=examples example=echo,hello wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False

./tools/build.py machine=versatile project=examples example=echo,chatterbox wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False

