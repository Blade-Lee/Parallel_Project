CC=g++

MPICC = mpic++

CFLAGS=-g -O3 -Wall -std=c++11
LDFLAGS= -lm

CFILES_SEQ = src/crun-seq.cpp
CFILES_PAR = src/crun-mpi.cpp	
#HFILES = src/util.h src/SmallParsimony.hpp src/LargeParsimony.hpp
HFILES = src/util.h src/LargeParsimony-mpi.hpp

all: crun-seq crun-mpi

crun-seq: $(CFILES_SEQ) $(HFILES) 
	$(CC) $(CFLAGS) -o crun-seq $(CFILES_SEQ) $(LDFLAGS)

crun-mpi: $(CFILES_PAR) $(XCFILES) $(HFILES) $(XHFILES)
	$(MPICC) $(CFLAGS) -o crun-mpi $(CFILES_PAR) $(XCFILES) $(LDFLAGS)

clean:
	rm -f *~ *.pyc
	rm -rf *.dSYM
	rm -f *.tgz
	rm -f crun-seq crun-mpi
