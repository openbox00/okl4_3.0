MACHINE=		versatile


COMMAND=		./tools/build.py
OPTIONS=		MACHINE=${MACHINE} wombat=true TOOLCHAIN=gnu_arm_eabi_toolchain pistachio.TOOLCHAIN=gnu_arm_toolchain PYFREEZE=False kdb_serial=True KDB_BREAKIN=False pistachio.enter_kdb=true 

all: multicell

.PHONY: multicell
multicell:
	${COMMAND} ${OPTIONS} PROJECT=examples EXAMPLE=echo,chatterbox 

two_oklinux:
	${COMMAND} ${OPTIONS} PROJECT=examples EXAMPLE=oklinux,oklinux2

hello:
	${COMMAND} ${OPTIONS} PROJECT=examples EXAMPLE=hello

clean:
	rm -vfr build/
	rm -vf *.pyc



