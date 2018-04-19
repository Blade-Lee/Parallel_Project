CC=g++

MPI=-DMPI
MPICC = mpicc

DEBUG=0
CFLAGS=-g -O3 -Wall -DDEBUG=$(DEBUG)
LDFLAGS= -lm

CFILES_SEQ = main.cpp
CFILES_PAR = main.cpp
HFILES = util.h

all: crun-seq crun-mpi

crun-seq: $(CFILES_SEQ) $(HFILES) 
	$(CC) $(CFLAGS) -o crun-seq $(CFILES_SEQ) $(LDFLAGS)

crun-mpi: $(CFILES_PAR) $(XCFILES) $(HFILES) $(XHFILES)
	$(MPICC) $(CFLAGS) $(MPI) -o crun-mpi $(CFILES_PAR) $(XCFILES) $(LDFLAGS)

clean:
	rm -f *~ *.pyc
	rm -rf *.dSYM
	rm -f *.tgz
	rm -f crun crun-seq crun-mpi
