#include <fstream>
#include <iostream>
#include <random>
#include <iomanip>
#include <string>
#include <sstream>
#include "producer.h"
#include <sys/stat.h>  // For creating directories

const int NUM_LIGHTS = 5;
const int HOURS = 24;
const int MINUTES = 60;
const std::string SAMPLE_TRAFFIC_DATA_FILE = "output/sample_traffic_data.txt";  // Updated to write to output folder

// Generates a formatted time string based on the given hour and minute (HH:MM format)
std::string create_time_label(int hour, int minute) {
    std::ostringstream label;
    label << std::setw(2) << std::setfill('0') << hour << ":" << std::setw(2) << std::setfill('0') << minute;
    return label.str();
}

// Creates random traffic data and stores it into a file
void generate_sample_traffic_data() {
    std::ofstream output_file(SAMPLE_TRAFFIC_DATA_FILE);
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open file for writing traffic data!" << std::endl;
        return;
    }

    std::random_device rd;
    std::mt19937 random_gen(rd());
    std::uniform_int_distribution<> dist(0, 50);  // Traffic between 0 and 50 cars

    for (int hour = 0; hour < HOURS; ++hour) {
        for (int minute = 0; minute < MINUTES; minute += 5) {  // Every 5 minutes
            for (int light_id = 0; light_id < NUM_LIGHTS; ++light_id) {
                int cars_passed = dist(random_gen);  // Generate random traffic
                output_file << create_time_label(hour, minute) << " Light" << light_id 
                            << " " << cars_passed << "\n";
                std::cout << "Generated: " << create_time_label(hour, minute) << " Light " 
                          << light_id << " " << cars_passed << " cars\n";
            }
        }
    }
    output_file.close();
}
