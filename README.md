mpi-pi-monte-carlo
================

C/C++ MPI program calculating pi value on distributed nodes using MPI. Developed on Windows. It needs Windows HPC Pack SDK to work.

The best performance is achieved when the number of nodes is equal to number of cores. E.g.:

    > mpiexec.exe -n 8 MPI_PIMonteCarlo.exe
    
The program performs by default 1 000 iterations with 100 000 hit attempts each. You can change the number of iterations by passing a parameter to the program:

    > mpiexec.exe -n 4 MPI_PIMonteCarlo.exe 5000
