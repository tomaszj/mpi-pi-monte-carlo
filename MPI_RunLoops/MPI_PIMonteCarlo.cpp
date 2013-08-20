// MPI_RunLoops.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cassert>
#include <mpi.h>
#include <time.h>

#define LOOPS_COUNT 1000
#define ITERATIONS_IN_LOOP 100000
#define TASK_TAG 1024
#define DIE_TAG 666


using namespace std;

long calculate_hits(int iterations);
void main_process(int world_size);
void worker_process(int rank, int world_size);

int main(int argc, char* argv[])
{
    clock_t tic = clock();

    // Seed the random generator for the process
    srand(time(NULL));
    
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int current_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &current_rank);

    if (current_rank == 0)
    {
        main_process(world_size);
    }
    else
    {
        worker_process(current_rank, world_size);
    }

    MPI_Finalize();
    clock_t toc = clock();

    if (current_rank == 0)
    {
        cout << "--- Operation took " << (double)(toc - tic) / CLOCKS_PER_SEC << " seconds ---" << endl;
        system("pause");
    }
	return 0;
}

void main_process(int world_size)
{
    // Coordinator process
    cout << "[Manager] Starting coordinator process" << endl;

    int sent_work_count = 0;
    int received_work_count = 0;
    
    long long total_attempts = 0;
    long long total_hits = 0;

    // Distribute initial work
    for (int i = 1; i < world_size; i++)
    {
        MPI_Send(&sent_work_count, 1, MPI_INT, i, TASK_TAG, MPI_COMM_WORLD); 
        //cout << "[Manager] Sent work #" << sent_work_count << " to #" << i << endl;
            
        sent_work_count++;
    }

    // Wait for incoming responses and deal remaining work
    while (received_work_count < LOOPS_COUNT)
    {
        int work_results[2];
        MPI_Status status;
        MPI_Recv(work_results, 2, MPI_LONG, MPI_ANY_SOURCE, TASK_TAG, MPI_COMM_WORLD, &status);

        received_work_count++;
        total_attempts += work_results[0];
        total_hits += work_results[1];

        if (sent_work_count < LOOPS_COUNT)
        {
            MPI_Send(&sent_work_count, 1, MPI_INT, status.MPI_SOURCE, TASK_TAG, MPI_COMM_WORLD);
            //cout << "[Manager] Sent work #" << sent_work_count << " to #" << status.MPI_SOURCE << endl;

            sent_work_count++;
        }
    }

    // Kill the workers
    for (int i = 1; i < world_size; i++)
    {
        int kill_message = -1;
        MPI_Send(&kill_message, 1, MPI_INT, i, DIE_TAG, MPI_COMM_WORLD);
    }

    double calculated_pi_value = 4.0 * total_hits / total_attempts;

    cout << "[Manager] Finished work with calculated PI: " << calculated_pi_value << "." << endl;
}

void worker_process(int rank, int world_size)
{
    // Worker process.
    cout << "[Worker #" << rank << "] Started." << endl;

    while(1)
    {
        int received_data;
        MPI_Status status;
        MPI_Recv(&received_data, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TASK_TAG)
        {
            int current_work_id = (int)received_data;
            //cout << "[Worker #" << rank << "] Received work #" << current_work_id << endl;
            long hits = calculate_hits(ITERATIONS_IN_LOOP);

            int result_array[2] = {ITERATIONS_IN_LOOP, hits};
            MPI_Send(result_array, 2, MPI_LONG, 0, TASK_TAG, MPI_COMM_WORLD);
            //cout << "[Worker #" << rank << "] Sent work #" << current_work_id << endl;
        }
        else if (status.MPI_TAG == DIE_TAG)
        {
            cout << "[Worker #" << rank << "] Killed." << endl;
            break;
        }
        else
        {
            // Should have discovered a proper tag already.
            MPI_Abort(MPI_COMM_WORLD, 666);
        }
    }
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