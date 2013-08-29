// MPI_PIMonteCarlo.cpp
//
// By Tomasz Janeczko (2013)

#include "stdafx.h"
#include <cassert>
#include <mpi.h>
#include <ctime>
#include <iomanip>

#define DEFAULT_ITERATIONS_COUNT 1000
#define LOOPS_IN_ITERATION 100000
#define TASK_TAG 1024

using namespace std;

long calculate_hits(int iterations);
void perform_process(long total_iterations, int current_rank, int world_size);

// Main entry point to the program.
//
// Specifying a number argument allows to control number of iterations done in the program
// $1 - number of iteration to be performed
int main(int argc, char* argv[])
{
    // Determine the number of iterations to be done using user input
    long total_iterations = 0;

    if (argc == 2)
    {
        char *input = argv[1];
        total_iterations = strtol(input, NULL, 10);
    }

    if (total_iterations == 0)
    {
        total_iterations = DEFAULT_ITERATIONS_COUNT;
    }

    // Randomize
    srand(time(NULL));

    // Initialize MPI and basic variables
    MPI_Init(&argc, &argv);

    double tic = MPI_Wtime();

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int current_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &current_rank);

    // Run the main process
    perform_process(total_iterations, current_rank, world_size);

    // Finish and produce summary
    double toc = MPI_Wtime();
    MPI_Finalize();

    if (current_rank == 0)
    {
        cout << "--- Operation took " << (toc - tic) << " seconds ---" << endl;
        system("pause");
    }

    return 0;
}

void perform_process(long total_iterations, int current_rank, int world_size)
{
    // Start of the process
    cout << "[Node #" << current_rank << "] Starting process of calculating pi value." << endl;

    int *iterations_distribution = new int[world_size];

    if (current_rank == 0)
    {
        int iterations_left = total_iterations;
        int iterations_per_worker = total_iterations / world_size;

        // Fill in a number of loops except for the last worker
        for (int i = 0; i < world_size - 1; i++)
        {
            iterations_distribution[i] = iterations_per_worker;
            iterations_left -= iterations_per_worker;
        }

        // Last worker gets the remaining loops to do
        iterations_distribution[world_size - 1] = iterations_left;

        cout << "[Node #0] Base nodes iterations: " << iterations_per_worker << " last node: " << iterations_left << endl;
    }

    // Distribute and perform the work
    int iterations;
    MPI_Scatter(iterations_distribution, 1, MPI_INT, &iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);

    cout << "[Node #" << current_rank << "] Got " << iterations << " iterations to perform." << endl;

    // Perform the work and collect the result
    long long total_hits = 0;
    for (int i = 0; i < iterations; i++)
    {
        long result = calculate_hits(LOOPS_IN_ITERATION);
        total_hits += result;
    }

    // Gather incoming responses
    long long *received_hits = new long long[world_size];
    MPI_Gather(&total_hits, 1, MPI_LONG_LONG, received_hits, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    if (current_rank == 0)
    {
        long long total_received_hits = 0;
        for (int i = 0; i < world_size; i++)
        {
            total_received_hits += received_hits[i];
        }

        double total_attempts = (double)total_iterations * LOOPS_IN_ITERATION;
        double pi_value = (double)total_received_hits * 4.0 / total_attempts;
        cout << "[Node #0] Received " << total_received_hits << " received hits in " << total_attempts << " attempts." << endl;
        cout << setprecision(15);
        cout << "[Node #0] Calculated PI value is " << pi_value << endl;
        cout << "[Node #0] Difference is " << 3.14159265358979323846 - pi_value << endl;
    }

    cout << "[Node #" << current_rank << "] dies." << endl;

    // Clean up
    delete[] iterations_distribution;
    delete[] received_hits;
}

long calculate_hits(int iterations)
{
    long i;
    long hits = 0;
    for (i = 0; i < iterations; i++)
    {
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;
        
        if (x * x + y * y <= 1.0)
        {
            hits++;
        }
    }

    return hits;
}