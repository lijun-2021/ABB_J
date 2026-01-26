#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "A_data.h"
#include <vector>
//网格预装分组
std::vector<std::vector<int>> calculate_grid_pre_order(const std::vector<int>& order);
//计算网格预装合并加工效率系数（同时做的越多，效率越高）
double get_batch_efficient(int batch_size);
//总成部分优化
int optimize_assembly(const std::vector<int>& order, const std::vector<int>& door_finish,
    const std::vector<int>& grid_finish, std::vector<JobInfo>& job_time_info,
    long long& inner_iter_count);
//完整工序的评估
int evaluate_order(const std::vector<int>& order, std::vector<JobInfo>& job_info,
    long long& inner_iter);

OutputData run_optimization();

#endif