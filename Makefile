# Author Alphée GROSDIDIER

PROJDIR := .
SOURCEDIR := $(PROJDIR)/src
BUILDDIR := $(PROJDIR)/build
DEBUGDIR :=  $(PROJDIR)/debug

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    OUTDIR := ${DEBUGDIR}
else
    OUTDIR := ${BUILDDIR}
endif

CC=gcc
CFLAGS=-g -Wall -std=c99 -O2 -Os
LDLIBS=-I${SOURCEDIR}/lib
LOADLIBES=-L${OUTDIR}/lib
LDFLAGS=-lmessage
TARGETST=server client launch_daemon
GAMEST=test_cli test_serv hangman_cli hangman_serv

ifeq ($(DEBUG), 1)
    CFLAGS+= -DDEBUG
	VALGRIND ?= 0
ifeq ($(VALGRIND), 1)
    CFLAGS+= -DVALGRIND
endif
endif

TARGETS := $(foreach target, ${TARGETST}, $(addprefix ${OUTDIR}/, ${target}))
GAMES := $(foreach game, ${GAMEST}, $(addprefix ${OUTDIR}/games/, ${game}))
OBJSTARGETS := $(foreach target, ${TARGETST}, $(addprefix ${OUTDIR}/objects/, ${target}))
OBJSGAMES := $(foreach game, ${GAMEST}, $(addprefix ${OUTDIR}/objects/games/, ${game}))
OBJS := ${OBJSTARGETS} ${OBJSGAMES}

.SECONDARY: $(OBJS:./%=%.o)

#export LD_LIBRARY_PATH="${PWD}/lib"

all: ${OUTDIR}/data/ ${TARGETS} ${GAMES}


#Build shared library
${OUTDIR}/objects/lib/message.o: ${SOURCEDIR}/lib/message.c ${SOURCEDIR}/lib/message.h
	@mkdir -p $(@D)
	${CC} ${CFLAGS} ${LDLIBS} -fPIC -c -o $@ $<

${OUTDIR}/lib/libmessage.so: ${OUTDIR}/objects/lib/message.o
	@mkdir -p $(@D)
	${CC} -shared -o $@ $<

#Build other library
${OUTDIR}/objects/lib/utils.o: ${SOURCEDIR}/lib/utils.c ${SOURCEDIR}/lib/utils.h
	@mkdir -p $(@D)
	${CC} ${CFLAGS} ${LDLIBS} -c -o $@ $<

#Build objects
${OUTDIR}/objects/%.o: ${SOURCEDIR}/%.c
	@mkdir -p $(@D)
	${CC} ${CFLAGS} ${LDLIBS} -c -o $@ $<

#build main target
${OUTDIR}/%: ${OUTDIR}/objects/%.o ${OUTDIR}/lib/libmessage.so ${OUTDIR}/objects/lib/utils.o
	@mkdir -p $(@D)
	${CC} ${LOADLIBES} $< ${OUTDIR}/objects/lib/utils.o ${LDFLAGS} -o $@ 

#Build games
${OUTDIR}/games/%: ${OUTDIR}/objects/games/%.o ${OUTDIR}/lib/libmessage.so ${OUTDIR}/objects/lib/utils.o
	@mkdir -p $(@D)
	${CC} ${LOADLIBES} $< ${OUTDIR}/objects/lib/utils.o ${LDFLAGS} -o $@ 


#Build directory and copy data
${OUTDIR}/data/: ${SOURCEDIR}/data/
	@mkdir -p ${OUTDIR}/data/
	cp -r -t ${OUTDIR} ${SOURCEDIR}/data


clean:
	rm -fR ${OUTDIR}/objects
mrproper: clean
	rm -fR ${OUTDIR}
