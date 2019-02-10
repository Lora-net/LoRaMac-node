# The code for the utility:
CODE:=airtime-calculator.c fake-board.c
EXECUTABLE:=airtime-calculator

# The compiler and flags:
CC:=clang
CFLAGS:=-dead_strip
# gcc would probably work as well, but needs a different flag for -dead_strip (maybe -Wl,--gc-sections)
# The purpose of this flag is to strip all irrelevant functions, and their dependencies on various hardware related functions such as timers.
# The alternative would be to have a complete fake-board.c which provides all functions that a typical board provides.

# The part of the LoRaWAN stack that contains the airtime calculation:
ROOT:=../..
STACK:=${ROOT}/radio/sx1276/sx1276.c
INCLUDES:=-I${ROOT}/radio/ -I${ROOT}/system/ -I${ROOT}/boards/ -I${ROOT}/mac/

all:
	${CC} ${CODE} ${STACK} ${CFLAGS} ${INCLUDES} -o ${EXECUTABLE}
