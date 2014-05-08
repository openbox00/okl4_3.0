#!/bin/bash

./tools/build.py machine=versatile project=examples example=oklinux,oklinux2 wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true KDB_BREAKIN=False
