#versatile_uboot
MACHINE=		versatile_uboot
PATH := 	    /home/asic/opt/nicta/tools/gcc-3.4.4-glibc-2.3.5/arm-linux/bin/:$(PATH) 
COMMAND=		./tools/build.py
OPTIONS=		MACHINE=${MACHINE} wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True \
				KDB_BREAKIN=True pistachio.enter_kdb=true

all: multicell
.PHONY: multicell
install:
	export PATH
	echo $(PATH)
multicell:
	${COMMAND} ${OPTIONS} PROJECT=examples example=echo,chatterbox 

2multicell:
	${COMMAND} ${OPTIONS} PROJECT=examples example=oklinux,decrypt  LINUX_APPS=cross-cell-demo

decrypt:
	${COMMAND} ${OPTIONS} PROJECT=examples example=decrypt

chatterbox:
	${COMMAND} ${OPTIONS} PROJECT=examples example=chatterbox

two_oklinux:
	${COMMAND} ${OPTIONS} PROJECT=examples example=oklinux,oklinux2

echo:
	${COMMAND} ${OPTIONS} PROJECT=examples example=echo,hello 

hello:
	${COMMAND} ${OPTIONS} PROJECT=examples example=hello

#linux_apps=lmbench
oklinux:
	${COMMAND} ${OPTIONS} PROJECT=examples example=oklinux LINUX_APPS=cross-cell-demo 

adder:
	${COMMAND} ${OPTIONS} PROJECT=examples example=adder

ktest:
	${COMMAND} ${OPTIONS} PROJECT=ktest 

ctest:
	${COMMAND} ${OPTIONS} PROJECT=ctest 

soc-sdk:
	${COMMAND} ${OPTIONS} PROJECT=soc-sdk

clean:
	rm -vfr build/
	rm -vf *.pyc



