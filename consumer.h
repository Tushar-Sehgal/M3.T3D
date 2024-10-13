#ifndef CONSUMER_H
#define CONSUMER_H

#include <vector>

void compute_and_distribute_traffic(int rank, int num_procs);
void collect_and_report_data(int num_procs);

#endif
