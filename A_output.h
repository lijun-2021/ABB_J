#ifndef OUTPUT_H
#define OUTPUT_H

#include "A_data.h"
#include <iostream>

OutputData generate_output_data(const std::vector<JobInfo>& job_info);
//工作站点数组的输出
void 
_output(const OutputData& output);
//每个位置的具体工件加工时间的输出
void print_details(const std::vector<JobInfo>& job_info);

#endif