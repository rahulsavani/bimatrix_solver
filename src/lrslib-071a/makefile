#makefile for lrslib-071    2020.5.23 

# C compiler requires __int128 support for 128bit arithmetic (eg. gcc v. 4.6.0 or later for 128bit integer support) 
# otherwise use %make lrs64 to compile
# add -DTIMES if your compiler does *not* support ptimes() and get_time()
# add -DSIGNALS if your compiler does *not* support <signal.h> <unistd.h>

#try uncommenting next line if cc is the default C compiler
#CC = gcc

default: lrs

#choose line below instead if __int128 not supported
#default: lrs64 lrsgmp 


#make lrs               lrs,lrsgmp       hybrid and gmp versions 
#make lrs64             lrs,lrsgmp    compilers without 128 bit support
#make mplrs             mplrs,mplrsgmp hybrid and gmp versions,  make sure mpicc and an MPI library is installed
#make mplrs64           mplrs,mplrsgmp for compilers without 128 bit support

#make flint             lrs and mplrs with FLINT arithmetic 
#make single            makes lrs with various arithmetic packages (depending on compiler),lrsnash 
#make singlemplrs        makes mplrs with various arithmetic packages (depending on compiler)
#make allmp             uses native mp and long arithmetic
#make demo              various demo programs for lrslib     
#make lrsnash           Nash equilibria for 2-person games: lrsnash (gmp), lrsnash1 (64bit), lrsnash2 (128bit)
#make fourier	        Fourier elimination (buggy, needs fixing)
#make clean             removes binaries                                      

#INCLUDEDIR = /usr/include
#LIBDIR     = /usr/lib

#Kyoto machines usage
INCLUDEDIR = /usr/local/include
LIBDIR     = /usr/local/lib

CFLAGS     = -O3 -Wall 
#CFLAGS     = -g -Wall 

#use this if you want only output file contain data between begin/end lines
#CFLAGS     = -O3 -Wall -DLRS_QUIET

SHLIB_CFLAGS = -fPIC
mpicxx=mpicc

LRSOBJ=lrs.o lrslong1.o lrslong2.o lrslib1.o lrslib2.o lrslibgmp.o lrsgmp.o lrsdriver.o
MPLRSOBJ=lrslong1-mplrs.o lrslong2-mplrs.o lrslib1-mplrs.o lrslib2-mplrs.o lrslibgmp-mplrs.o lrsgmp-mplrs.o lrsdriver-mplrs.o mplrs.o

LRSOBJ64=lrs64.o lrslong1.o lrslib1.o lrslibgmp.o lrsgmp.o lrsdriver.o
MPLRSOBJ64=lrslong1-mplrs.o lrslib1-mplrs.o lrslibgmp-mplrs.o lrsgmp-mplrs.o lrsdriver-mplrs.o mplrs64.o

lrs: ${LRSOBJ}
	$(CC) ${CFLAGS} -DMA -DB128 -L${LIBDIR} -o lrs ${LRSOBJ} -lgmp
	$(CC)  -O3   -DGMP -I${INCLUDEDIR} -o lrsgmp lrs.c lrslib.c lrsgmp.c lrsdriver.c -L${LIBDIR}  -lgmp
	$(CC) -O3 hvref.c -o hvref
	ln -s -f lrs redund
	ln -s -f lrsgmp redundgmp

lrs64: ${LRSOBJ64}
	$(CC) ${CFLAGS} -DMA -L${LIBDIR} -o lrs ${LRSOBJ64} -lgmp

lrs.o: lrs.c
	$(CC) ${CFLAGS} -DMA -DB128 -c -o lrs.o lrs.c

lrs64.o: lrs.c
	$(CC) ${CFLAGS} -DMA -c -o lrs64.o lrs.c

lrslong1.o: lrslong.c lrslong.h
	$(CC) ${CFLAGS} -DMA -DSAFE -DLRSLONG -c -o lrslong1.o lrslong.c

lrslong2.o: lrslong.c lrslong.h
	$(CC) ${CFLAGS} -DMA -DSAFE -DB128 -DLRSLONG -c -o lrslong2.o lrslong.c

lrslib1.o: lrslib.c lrslib.h
	$(CC) ${CFLAGS} -DMA -DSAFE -DLRSLONG -c -o lrslib1.o lrslib.c

lrslib2.o: lrslib.c lrslib.h
	$(CC) ${CFLAGS} -DMA -DSAFE -DB128 -DLRSLONG -c -o lrslib2.o lrslib.c

lrslibgmp.o: lrslib.c lrslib.h
	$(CC) ${CFLAGS} -DMA -DGMP -I${INCLUDEDIR} -c -o lrslibgmp.o lrslib.c

lrsgmp.o: lrsgmp.c lrsgmp.h
	$(CC) ${CFLAGS} -DMA -DGMP -I${INCLUDEDIR} -c -o lrsgmp.o lrsgmp.c


lrslong1-mplrs.o: lrslong.c lrslong.h
	$(mpicxx) ${CFLAGS} -DTIMES -DSIGNALS -DMA -DSAFE -DLRSLONG -DPLRS -c -o lrslong1-mplrs.o lrslong.c

lrslong2-mplrs.o: lrslong.c lrslong.h
	$(mpicxx) ${CFLAGS} -DTIMES -DSIGNALS -DMA -DSAFE -DB128 -DLRSLONG -DPLRS -c -o lrslong2-mplrs.o lrslong.c

lrslib1-mplrs.o: lrslib.c lrslib.h
	$(mpicxx) ${CFLAGS} -DTIMES -DSIGNALS -DMA -DSAFE -DLRSLONG -DPLRS -c -o lrslib1-mplrs.o lrslib.c

lrslib2-mplrs.o: lrslib.c lrslib.h
	$(mpicxx) ${CFLAGS} -DTIMES -DSIGNALS -DMA -DSAFE -DB128 -DLRSLONG -DPLRS -c -o lrslib2-mplrs.o lrslib.c

lrslibgmp-mplrs.o: lrslib.c lrslib.h
	$(mpicxx) ${CFLAGS} -DMA -DTIMES -DSIGNALS -DGMP -DPLRS -I${INCLUDEDIR} -c -o lrslibgmp-mplrs.o lrslib.c

lrsgmp-mplrs.o: lrsgmp.c lrsgmp.h
	$(mpicxx) ${CFLAGS} -DMA -DTIMES -DSIGNALS -DGMP -DPLRS -I${INCLUDEDIR} -c -o lrsgmp-mplrs.o lrsgmp.c

lrsdriver-mplrs.o: lrsdriver.c lrsdriver.h lrslib.h
	$(mpicxx) $(CFLAGS) -c -o lrsdriver-mplrs.o lrsdriver.c

mplrs.o: mplrs.c mplrs.h lrslib.h lrsgmp.h
	$(mpicxx) ${CFLAGS} -I${INCLUDEDIR} -DMA -DPLRS -DTIMES -DB128 -DSIGNALS -D_WITH_GETLINE -c -o mplrs.o mplrs.c

mplrs64.o: mplrs.c mplrs.h lrslib.h lrsgmp.h
	$(mpicxx) ${CFLAGS} -I${INCLUDEDIR} -DMA -DPLRS -DTIMES -DSIGNALS -D_WITH_GETLINE -c -o mplrs64.o mplrs.c

mplrs: ${MPLRSOBJ} mplrsgmp
	$(mpicxx) ${CFLAGS} -DTIMES -DSIGNALS -D_WITH_GETLINE -DPLRS -DMA -DB128 -L${LIBDIR} -o mplrs ${MPLRSOBJ} -lgmp

mplrs64: ${MPLRSOBJ64} mplrsgmp
	$(mpicxx) ${CFLAGS} -DTIMES -DSIGNALS -D_WITH_GETLINE -DPLRS -DMA -L${LIBDIR} -o mplrs ${MPLRSOBJ64} -lgmp

mplrsgmp: mplrs.c mplrs.h lrslib.c lrslib.h lrsgmp.c lrsgmp.h lrsdriver.h lrsdriver.c
	$(mpicxx) ${CFLAGS} -DTIMES -DSIGNALS -D_WITH_GETLINE -DPLRS -DGMP -I${INCLUDEDIR} mplrs.c lrslib.c lrsgmp.c lrsdriver.c -L${LIBDIR} -o mplrsgmp -lgmp

mplrs1: mplrs.c mplrs.h lrslib.c lrslib.h lrslong.c lrslong.h lrsdriver.h lrsdriver.c
	$(mpicxx) ${CFLAGS} -DTIMES -DSIGNALS -D_WITH_GETLINE -DPLRS -DSAFE -DLRSLONG mplrs.c lrslib.c lrslong.c lrsdriver.c -o mplrs1

mplrs2: mplrs.c mplrs.h lrslib.c lrslib.h lrslong.c lrslong.h lrsdriver.h lrsdriver.c
	$(mpicxx) ${CFLAGS} -DTIMES -DSIGNALS -D_WITH_GETLINE -DPLRS -DSAFE -DLRSLONG -DB128 mplrs.c lrslib.c lrslong.c lrsdriver.c -o mplrs2

mplrsmp: mplrs.c mplrs.h lrslib.c lrslib.h lrsmp.c lrsmp.h lrsdriver.h lrsdriver.c
	$(mpicxx) ${CFLAGS} -DMP -DTIMES -DSIGNALS -D_WITH_GETLINE -DPLRS mplrs.c lrslib.c lrsmp.c lrsdriver.c -o mplrsmp

singlemplrs: mplrsgmp mplrs1 mplrs2

flint:	 	lrs.c lrslib.c lrslib.h lrsgmp.c lrsgmp.h
		@test -d  ${INCLUDEDIR}/flint || { echo ${INCLUDEDIR}/flint not found; exit 1; }
		$(CC) -O3 -DFLINT -I${INCLUDEDIR} -I${INCLUDEDIR}/flint lrs.c lrsdriver.c lrslib.c lrsgmp.c -L${LIBDIR} -lflint -o lrsflint -lgmp

mplrsflint:	mplrs.c mplrs.h lrslib.c lrslib.h lrsgmp.c lrsgmp.h lrsdriver.c lrsdriver.h
	${mpicxx} ${CFLAGS} -DTIMES -DSIGNALS -D_WITH_GETLINE -DFLINT -I${INCLUDEDIR}/flint -DPLRS -o mplrsflint mplrs.c lrsdriver.c lrslib.c lrsgmp.c -L${LIBDIR} -lflint -lgmp

#comment out lines with -DB128 if __int128 not supported by your C compiler

lrsgmp:		lrs.c lrslib.c lrslib.h lrsgmp.c lrsgmp.h lrsdriver.h lrsdriver.c 
		$(CC)  -O3   -DGMP -I${INCLUDEDIR} -o lrsgmp lrs.c lrslib.c lrsgmp.c lrsdriver.c -L${LIBDIR}  -lgmp

single:		lrs.c lrslong.c lrslong.h lrslib.c lrslib.h lrsgmp.c lrsgmp.h lrsdriver.h lrsdriver.c
		$(CC)  -O3 -DSAFE  -DLRSLONG -o lrs1 lrs.c lrslib.c lrslong.c lrsdriver.c
		$(CC)  -O3 -DB128 -DSAFE  -DLRSLONG -o lrs2 lrs.c lrslib.c lrslong.c lrsdriver.c
		ln -s -f lrs1 redund1
		ln -s -f lrs2 redund2

allmp:		lrs.c lrslib.c lrslib.h lrsmp.c lrsmp.h lrsdriver.h lrsdriver.c
		$(CC) -Wall -O3  -o lrs lrs.c lrslib.c lrsdriver.c lrsmp.c
		$(CC) -Wall -O3  -DSAFE -DLRSLONG -o lrs1 lrs.c lrslib.c lrsdriver.c lrslong.c
		$(CC) -Wall -O3  -DSAFE -DLRSLONG -DB128 -o lrs2 lrs.c lrslib.c lrsdriver.c lrslong.c
		$(CC) -O3 -DLRS_QUIET   -o lrsnash lrsnash.c lrsnashlib.c lrslib.c lrsdriver.c lrsmp.c
		$(CC) -O3  -o setnash setupnash.c lrslib.c lrsdriver.c lrsmp.c
		$(CC) -O3  -o setnash2 setupnash2.c lrslib.c lrsdriver.c lrsmp.c
		$(CC) -O3  -o 2nash 2nash.c

demo:	lpdemo1.c lrslib.c lrsdriver.c lrslib.h lrsgmp.c lrsgmp.h
	$(CC) -O3   -I${INCLUDEDIR} -L${LIBDIR} -o lpdemo1 lpdemo1.c lrslib.c lrsdriver.c lrsgmp.c -lgmp -DGMP
	$(CC) -O3   -I${INCLUDEDIR} -L${LIBDIR} -o lpdemo lpdemo.c lrslib.c lrsdriver.c lrsgmp.c -lgmp -DGMP
	$(CC) -O3   -I${INCLUDEDIR} -L${LIBDIR} -o lpdemo2 lpdemo2.c lrslib.c lrsdriver.c lrsgmp.c -lgmp -DGMP
	$(CC) -O3   -I${INCLUDEDIR} -L${LIBDIR} -o vedemo  vedemo.c lrslib.c lrsdriver.c lrsgmp.c -lgmp -DGMP
	$(CC) -O3   -I${INCLUDEDIR} -L${LIBDIR} -o chdemo  chdemo.c lrslib.c lrsdriver.c lrsgmp.c -lgmp -DGMP

lrsnash:	lrsnash.c nashdemo.c lrsnashlib.c lrslib.c lrsnashlib.h lrslib.h lrsgmp.c lrsgmp.h lrslong.h lrsdriver.h lrsdriver.c
		$(CC) -O3   -I${INCLUDEDIR} -L${LIBDIR} -o lrsnashgmp lrsnash.c lrsnashlib.c lrslib.c lrsgmp.c lrsdriver.c  -lgmp -DGMP
		$(CC) -O3   -I${INCLUDEDIR} -L${LIBDIR} -o lrsnash1 lrsnash.c lrsnashlib.c lrslib.c lrslong.c lrsdriver.c -DLRSLONG -DSAFE
		$(CC) -O3   -I${INCLUDEDIR} -L${LIBDIR} -o lrsnash2 lrsnash.c lrsnashlib.c lrslib.c lrslong.c lrsdriver.c -DLRSLONG -DSAFE -DB128
		$(CC) -O3   -I${INCLUDEDIR} -L${LIBDIR} -o nashdemo nashdemo.c lrsnashlib.c lrslib.c lrsgmp.c lrsdriver.c -lgmp -DGMP
		$(CC) -O3  -I${INCLUDEDIR} -L${LIBDIR} -o 2nash 2nash.c
		cp lrsnashgmp lrsnash

fourier:	fourier.c lrslib.h lrslib.c lrsgmp.h lrsgmp.c
	$(CC) -O3   -DGMP -I${INCLUDEDIR} fourier.c lrslib.c lrsdriver.c lrsgmp.c -L${LIBDIR}  -lgmp -o fourier

######################################################################
# From here on the author is David Bremner <bremner@unb.ca> to whom you should turn for help             
#
# Shared library variables
SONAME ?=liblrs.so.1
SOMINOR ?=.0.0
SHLIB ?=$(SONAME)$(SOMINOR)
SHLINK ?=liblrs.so

SHLIBOBJ=lrslong1-shr.o lrslong2-shr.o lrslib1-shr.o lrslib2-shr.o \
	lrslibgmp-shr.o lrsgmp-shr.o lrsdriver-shr.o

SHLIBBIN=lrs-shared lrsnash-shared

# Building (linking) the shared library, and relevant symlinks.

${SHLIB}: ${SHLIBOBJ}
	$(CC) -shared -Wl,-soname=$(SONAME) $(SHLIBFLAGS) -o $@ ${SHLIBOBJ} -lgmp

${SONAME}: ${SHLIB}
	ln -sf ${SHLIB} ${SONAME}

${SHLINK}: ${SONAME}
	ln -sf $< $@

# binaries linked against the shared library

all-shared: ${SHLIBBIN}

lrs-shared: ${SHLINK} lrs.o
	$(CC) lrs.o -o $@ -L . -llrs


lrsnash-shared: ${SHLINK}  lrsnash.c
	$(CC) -DGMP -DMA lrsnash.c  lrsnashlib.c -I${INCLUDEDIR} -o $@ -L . -llrs -lgmp

# build object files for the shared library

lrslib1-shr.o: lrslib.c lrslib.h
	$(CC) ${CFLAGS} ${SHLIB_CFLAGS} -DMA -DSAFE -DLRSLONG -c -o $@ lrslib.c

lrsdriver-shr.o: lrsdriver.c
	$(CC) ${CFLAGS} ${SHLIB_CFLAGS} -c -o $@ $<

lrslong1-shr.o: lrslong.c lrslong.h
	$(CC) ${CFLAGS} ${SHLIB_CFLAGS} -DMA -DSAFE -DLRSLONG -c -o $@ lrslong.c

lrslong2-shr.o: lrslong.c lrslong.h
	$(CC) ${CFLAGS} ${SHLIB_CFLAGS} -DMA -DSAFE -DB128 -DLRSLONG -c -o $@ lrslong.c

lrslibgmp-shr.o: lrslib.c lrslib.h
	$(CC) ${CFLAGS} ${SHLIB_CFLAGS} -DMA -DGMP -I${INCLUDEDIR} -c -o $@ lrslib.c

lrsgmp-shr.o: lrsgmp.c lrsgmp.h
	$(CC) ${CFLAGS} ${SHLIB_CFLAGS} -DMA -DGMP -I${INCLUDEDIR} -c -o $@ lrsgmp.c

lrslib2-shr.o: lrslib.c lrslib.h
	$(CC) ${CFLAGS} ${SHLIB_CFLAGS} -DMA -DSAFE -DB128 -DLRSLONG -c -o $@ lrslib.c

######################################################################
# install targets
# where to install binaries, libraries, include files
prefix ?= /usr/local
INSTALL_INCLUDES=lrslib.h lrsdriver.h lrsgmp.h lrslong.h lrsmp.h

install: all-shared install-common
	mkdir -p $(DESTDIR)${prefix}/bin
	for file in ${SHLIBBIN}; do cp $${file} $(DESTDIR)${prefix}/bin/$$(basename $$file -shared); done
	mkdir -p $(DESTDIR)${prefix}/lib
	install -t $(DESTDIR)${prefix}/lib $(SHLIB)
	cd $(DESTDIR)${prefix}/lib && ln -sf $(SHLIB) $(SHLINK)
	cd $(DESTDIR)${prefix}/lib && ln -sf $(SHLIB) $(SONAME)

install-common:
	mkdir -p $(DESTDIR)${prefix}/include/lrslib
	install -t $(DESTDIR)${prefix}/include/lrslib ${INSTALL_INCLUDES}

######################################################################
clean:		
	rm -f  lrs lrs1 lrsgmp lrs1n lpdemo lpdemo1 lpdemo2 mplrs1 mplrs mplrsmp  mplrsgmp lrs2 mplrs2 lrsflint mplrsflint *.o *.exe *.so
	rm -f  hvref setnash setnash2 fourier lrsnashgmp lrsnash lrsnash1 lrsnash2 nashdemo 2nash vedemo
	rm -f ${LRSOBJ} ${LRSOBJ64} ${SHLIBOBJ} ${SHLIB} ${SONAME} ${SHLINK}
	rm -f ${SHLIBBIN}
