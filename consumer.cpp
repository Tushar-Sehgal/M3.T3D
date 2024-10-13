#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <mpi.h>
#include "consumer.h"
#include <sys/stat.h>

const int NUM_LIGHTS = 5;
const int HOURS = 24;
const int MINUTES = 60;
const std::string SAMPLE_TRAFFIC_DATA_FILE = "output/sample_traffic_data.txt";  // Same path as in producer
const std::string CONGESTION_OUTPUT_FILE = "output/top_n_output.txt";  // Updated path to output folder
const int TOP_LIGHTS = 3;

// Reads traffic data for each process and aggregates it
void read_traffic_data(int rank, int num_procs, std::vector<std::vector<int>>& traffic_data) {
    std::ifstream data_file(SAMPLE_TRAFFIC_DATA_FILE);
    if (!data_file.is_open()) {
        std::cerr << "Process " << rank << ": Unable to open traffic data file!" << std::endl;
        return;
    }

    int total_lines = HOURS * (MINUTES / 5) * NUM_LIGHTS; // Total lines (5-minute intervals)
    int lines_per_process = total_lines / num_procs;
    int start_line = rank * lines_per_process;
    int end_line = (rank == num_procs - 1) ? total_lines : start_line + lines_per_process; // Last process gets the remaining lines

    std::string line;
    int line_num = 0;

    while (std::getline(data_file, line)) {
        if (line_num >= start_line && line_num < end_line) {
            std::istringstream line_stream(line);
            std::string timestamp, light;
            int traffic_count;

            // Parse timestamp, light ID, and traffic count
            line_stream >> timestamp >> light >> traffic_count;

            // Extract hour and minute from the timestamp
            int hour, minute;
            char delim;
            std::istringstream time_stream(timestamp);
            time_stream >> hour >> delim >> minute;

            if (light.rfind("Light", 0) == 0) {
                int light_index = std::stoi(light.substr(5));  // Extract the traffic light number after "Light"
                traffic_data[hour][light_index] += traffic_count;
            }
        }
        ++line_num;
    }

    data_file.close();
}

// Sends total traffic data to the master process
void compute_and_distribute_traffic(int rank, int num_procs) {
    std::vector<std::vector<int>> traffic(HOURS, std::vector<int>(NUM_LIGHTS, 0));
    read_traffic_data(rank, num_procs, traffic);

    for (int hour = 0; hour < HOURS; ++hour) {
        std::cout << "Process " << rank << " sending data for hour: " << hour << std::endl;  // Debug print
        MPI_Send(traffic[hour].data(), NUM_LIGHTS, MPI_INT, 0, hour, MPI_COMM_WORLD);
    }
}

// Receives and aggregates total traffic data from all workers and writes to output file from 6:00 onwards
void collect_and_report_data(int num_procs) {
    std::ofstream output_file(CONGESTION_OUTPUT_FILE);
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open congestion output file!" << std::endl;
        return;
    }

    const int START_HOUR = 5;  // Start writing data from 5:00 onwards

    for (int hour = 0; hour < HOURS; ++hour) {
        std::vector<std::pair<int, int>> traffic_stats(NUM_LIGHTS, std::make_pair(0, 0));  // Total traffic

        // Initialize to track total cars passed for each light
        for (int light = 0; light < NUM_LIGHTS; ++light) {
            traffic_stats[light].first = light;  // Light ID
            traffic_stats[light].second = 0;     // Initialize to 0 cars
        }

        std::cout << "Collecting data for hour: " << hour << "\n";  // Debug print

        // Receive data from all workers and aggregate
        for (int proc = 1; proc < num_procs; ++proc) {
            std::vector<int> recv_data(NUM_LIGHTS);
            MPI_Recv(recv_data.data(), NUM_LIGHTS, MPI_INT, proc, hour, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  // Correct tag

            std::cout << "Received data from process " << proc << " for hour: " << hour << "\n";  // Debug print

            // Aggregate total cars for each light
            for (int light = 0; light < NUM_LIGHTS; ++light) {
                traffic_stats[light].second += recv_data[light];  // Sum cars for each light
                std::cout << "Traffic Light ID: " << light << " -- Cars Passed: " << recv_data[light] << " for hour: " << hour << "\n";  // Debug print
            }
        }

        // Sort traffic data by total cars (descending order)
        std::sort(traffic_stats.begin(), traffic_stats.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.second > b.second;
        });

        // Only write to the file starting from hour 6
        if (hour >= START_HOUR) {
            // Print top N congested lights to the output file
            output_file << "================= Top 3 Congested Traffic Lights for Hour: " << hour << ":00 =================\n";
            for (int i = 0; i < TOP_LIGHTS; ++i) {
                output_file << "Traffic Light ID: " << traffic_stats[i].first 
                            << " -- Cars Passed: " << traffic_stats[i].second << "\n";
            }
            output_file << "==================================================================================\n";
        }
    }

    output_file.close();
}
