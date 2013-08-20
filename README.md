mpi-pi-monte-carlo
================

C/C++ MPI program calculating pi value on distributed nodes using MPI. Developed in M$ Visual C++ 2010 Express. It needs Windows HPC Pack SDK to work.

The best performance is shown when the number of nodes is equal to number of cores + 1 (for management process). E.g.:

    > mpiexec.exe -n 9 MPI_PIMonteCarlo.exe
