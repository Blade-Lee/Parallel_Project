CC=g++

MPICC = mpic++
OMP=-fopenmp -DOMP

CFLAGS=-g -O3 -Wall -std=c++11
LDFLAGS= -lm

CFILES_SEQ = src/crun-seq.cpp
CFILES_PAR = src/crun-omp.cpp	
HFILES_SEQ = src/util.h src/SmallParsimony.hpp src/LargeParsimony.hpp
HFILES_PAR = src/util.h src/LargeParsimony-omp.hpp

all: crun-seq crun-omp

crun-seq: $(CFILES_SEQ) $(HFILES_SEQ) 
	$(CC) $(CFLAGS) -o crun-seq $(CFILES_SEQ) $(LDFLAGS)

crun-omp: $(CFILES_PAR) $(HFILES_PAR)
	$(CC) $(CFLAGS) $(OMP) -o crun-omp $(CFILES_PAR) $(LDFLAGS)

# crun-mpi: $(CFILES_PAR) $(XCFILES) $(HFILES_PAR) $(XHFILES)
# 	$(MPICC) $(CFLAGS) -o crun-mpi $(CFILES_PAR) $(XCFILES) $(LDFLAGS)

clean:
	rm -f *~ *.pyc
	rm -rf *.dSYM
	rm -f *.tgz
	rm -f crun-seq crun-omp
