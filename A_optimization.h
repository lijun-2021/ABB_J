#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "A_data.h"
#include <vector>

int optimize_assembly(const std::vector<int>& order, const std::vector<int>& door_finish,
    const std::vector<int>& grid_finish, std::vector<JobInfo>& job_time_info,
    long long& inner_iter_count);

int evaluate_order(const std::vector<int>& order, std::vector<JobInfo>& job_info,
    long long& inner_iter);

OutputData run_optimization();

#endif