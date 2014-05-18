MACHINE=		versatile
PATH := 	    /home/asic/opt/nicta/tools/gcc-3.4.4-glibc-2.3.5/arm-linux/bin/:$(PATH) 
COMMAND=		./tools/build.py
OPTIONS=		MACHINE=${MACHINE} wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true 

all: multicell
.PHONY: multicell
install:
	export PATH
	echo $(PATH)
multicell:
	${COMMAND} ${OPTIONS} PROJECT=examples EXAMPLE=echo,chatterbox 

two_oklinux:
	${COMMAND} ${OPTIONS} PROJECT=examples EXAMPLE=oklinux,oklinux2

hello:
	${COMMAND} ${OPTIONS} PROJECT=examples EXAMPLE=hello

oklinux:
	${COMMAND} ${OPTIONS} PROJECT=examples EXAMPLE=oklinux

adder:
	${COMMAND} ${OPTIONS} PROJECT=examples EXAMPLE=adder

ktest:
	${COMMAND} ${OPTIONS} PROJECT=ktest 

ctest:
	${COMMAND} ${OPTIONS} PROJECT=ctest 

soc-sdk:
	${COMMAND} ${OPTIONS} PROJECT=soc-sdk

clean:
	rm -vfr build/
	rm -vf *.pyc



