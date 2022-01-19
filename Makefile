CC = cc
CCFLAGS = -O2 -std=c11

all: rz36 as21 sim21

rz36:
	${CC} ${CCFLAGS} -I src/rz36/comp/. -o rz36 src/rz36/comp/*.c

as21:
	${CC} ${CCFLAGS} -o as21 src/as21/*.c

sim21:
	${CC} ${CCFLAGS} -o sim21 src/sim21/*.c


clean:
	rm rz36
	rm as21
	rm sim21

