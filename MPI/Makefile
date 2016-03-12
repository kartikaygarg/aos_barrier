OMPFLAGS = -fopenmp
OMPLIBS = -lgomp

CC = gcc
CPPFLAGS = -g -Wall
LDFLAGS = -g -Wall
LDLIBS = $(OMPLIBS)

#MPICC = mpicc
MPICC = /opt/openmpi-1.4.3-gcc44/bin/mpicc
#MPICH = /usr/lib64/openmpi/1.4-gcc
MPICH = /opt/openmpi-1.4.3-gcc44
CFLAGS = -I $(MPICH)/include

all: mpi_all openmp_mpi

mpi_all: mpi_sense mpi_diss mpi_tour

mpi_sense: mpi_sense.o
	$(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS)
	
mpi_sense_try: mpi_sense_try.o
	$(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

mpi_diss: mpi_diss.o
	$(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

mpi_tour: mpi_tour.o
	$(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS) $(OMPLIBS)

openmp_mpi: openmp_mpi.o
	$(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS) $(OMPLIBS)

openmp_mpi.o: openmp_mpi.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(OMPFLAGS) $^

cleanall: clean
	rm -f *.log *_job.e* *_job.o*

clean:
	rm -f *.o mpi_sense mpi_diss mpi_tour hello_world communicate_world openmp_mpi
