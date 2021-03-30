# $Id: Makefile,v 1.19 2021-02-17 18:20:54-08 - - $

MKFILE      = Makefile
DEPFILE     = ${MKFILE}.dep
NOINCL      = ci clean spotless
NEEDINCL    = ${filter ${NOINCL}, ${MAKECMDGOALS}}
GMAKE       = ${MAKE} --no-print-directory

GPPWARN     = -Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast
GPPOPTS     = ${GPPWARN} -fdiagnostics-color=always
COMPILECPP  = g++ -std=gnu++17 -g -O0 ${GPPOPTS}
MAKEDEPCPP  = g++ -std=gnu++17 -MM ${GPPOPTS}
UTILBIN     = /afs/cats.ucsc.edu/courses/cse111-wm/bin

MODULES     = logstream protocol sockets
EXECBINS    = cxi cxid
ALLMODS     = ${MODULES} ${EXECBINS}
SOURCELIST  = ${foreach MOD, ${ALLMODS}, ${MOD}.h ${MOD}.tcc ${MOD}.cpp}
CPPSOURCE   = ${wildcard ${MODULES:=.cpp} ${EXECBINS:=.cpp}}
ALLSOURCE   = ${wildcard ${SOURCELIST}} ${MKFILE}
CPPLIBS     = ${wildcard ${MODULES:=.cpp}}
OBJLIBS     = ${CPPLIBS:.cpp=.o}
CXIOBJS     = cxi.o ${OBJLIBS}
CXIDOBJS    = cxid.o ${OBJLIBS}
CLEANOBJS   = ${OBJLIBS} ${CXIOBJS} ${CXIDOBJS}
LISTING     = Listing.ps

export PATH := ${PATH}:/afs/cats.ucsc.edu/courses/cse110a-wm/bin

# all: ${DEPFILE} ${EXECBINS}
all: clearscreen ${DEPFILE} ${EXECBINS}

clearscreen:
	clear

cxi: ${CXIOBJS}
	${COMPILECPP} -o $@ ${CXIOBJS}

cxid: ${CXIDOBJS}
	${COMPILECPP} -o $@ ${CXIDOBJS}

%.o: %.cpp
	- checksource $<
	- cpplint.py.perl $<
	${COMPILECPP} -c $<

ci: ${ALLSOURCE}
	cid -is ${ALLSOURCE}
	- checksource ${ALLSOURCE}

lis: all ${ALLSOURCE} ${DEPFILE}
	- pkill -g 0 gv || true
	mkpspdf ${LISTING} ${ALLSOURCE} ${DEPFILE}

clean:
	- rm ${LISTING} ${LISTING:.ps=.pdf} ${CLEANOBJS} core

spotless: clean
	- rm ${EXECBINS} ${DEPFILE}


test: ${EXECBINS}
	@ cp ${ALLSOURCE} README
	@ cd ../dot.score && ./mk.build && cp ./cxi local && cp ./cxid remote


dep: ${ALLCPPSRC}
	@ echo "# ${DEPFILE} created $(LC_TIME=C date)" >${DEPFILE}
	${MAKEDEPCPP} ${CPPSOURCE} >>${DEPFILE}

${DEPFILE}:
	@ touch ${DEPFILE}
	${GMAKE} dep

again: ${ALLSOURCE}
	${GMAKE} spotless dep ci all lis

ifeq (${NEEDINCL}, )
include ${DEPFILE}
endif

