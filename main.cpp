#include <mpi.h>
#include <iostream>
#include "producer.h"  // Include the producer header
#include "consumer.h"  // Include the consumer header

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (rank == 0) {
        generate_sample_traffic_data();  // Generate the sample data file
        MPI_Barrier(MPI_COMM_WORLD);
        collect_and_report_data(num_procs);  // Collect and report results
    } else {
        MPI_Barrier(MPI_COMM_WORLD);
        compute_and_distribute_traffic(rank, num_procs);  // Compute and send traffic data
    }

    MPI_Finalize();
    return 0;
}