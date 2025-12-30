#ifndef OUTPUT_H
#define OUTPUT_H

#include "A_data.h"
#include <iostream>

OutputData generate_output_data(const std::vector<JobInfo>& job_info);
void print_output(const OutputData& output);
void print_details(const std::vector<JobInfo>& job_info);

#endif