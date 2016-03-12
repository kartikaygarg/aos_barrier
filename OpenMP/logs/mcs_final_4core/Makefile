OMPFLAGS = -fopenmp
OMPLIBS = -lgomp

CC = gcc
CPPFLAGS = -g -Wall $(OMPFLAGS)
LDFLAGS = -g -Wall
LDLIBS = $(OMPLIBS)

MPICC = /opt/openmpi-1.4.3-gcc44/bin/mpicc

#MPICH = /opt/openmpi-1.4.3-gcc44
#CFLAGS = -I $(MPICH)/include

all: mpi_all omp_all openmp_mpi

omp_all: omp_sense omp_mcs 

mpi_all: mpi_sense mpi_diss mpi_tour

omp_sense: omp_sense.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

omp_mcs: omp_mcs.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	
omp_mcs2: omp_mcs2.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	
omp_mcs_ed: omp_mcs_ed.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mpi_sense: mpi_sense.o
	$(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

mpi_diss: mpi_diss.o
	$(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

mpi_tour: mpi_tour.o
	$(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS) $(OMPLIBS)

openmp_mpi: openmp_mpi.o
	$(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS) $(OMPLIBS)

openmp_mpi.o: openmp_mpi.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(OMPFLAGS) $^
	
priv_vars: priv_vars.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

shared_vars: shared_vars.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

parallel_for: parallel_for.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

cleanall: clean
	rm -f *.log *_job*.e* *_job*.o*

clean:
	rm -f *.o mpi_sense mpi_diss mpi_tour hello_world communicate_world openmp_mpi omp_sense omp_mcs priv_vars shared_vars parallel_for
