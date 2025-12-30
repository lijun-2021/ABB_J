// output.cpp
#include "A_output.h"
#include "A_constants.h"
#include <algorithm>
#include <iomanip>
#include <numeric>

OutputData generate_output_data(const std::vector<JobInfo>& job_info) {
    OutputData output;

    // 初始化二维数组
    output.door_pre.resize(door_pre);
    output.door_work.resize(door_work);
    output.grid_pre.resize(grid_pre);
    output.grid_work.resize(grid_work);
    output.assembly.resize(assembly);

    // 门板预装
    for (const auto& info : job_info) {
        output.door_pre[info.door_pre_pos].push_back(info.job_id);
    }
    for (auto& station : output.door_pre) {
        std::sort(station.begin(), station.end(), [&](int a, int b) {
            return job_info[a].door_pre_start < job_info[b].door_pre_start;
            });
    }

    // 门板加工
    for (const auto& info : job_info) {
        output.door_work[info.door_work_pos].push_back(info.job_id);
    }
    for (auto& station : output.door_work) {
        std::sort(station.begin(), station.end(), [&](int a, int b) {
            return job_info[a].door_work_start < job_info[b].door_work_start;
            });
    }

    // 网格预装
    for (const auto& info : job_info) {
        output.grid_pre[info.grid_pre_pos].push_back(info.job_id);
    }
    for (auto& station : output.grid_pre) {
        std::sort(station.begin(), station.end(), [&](int a, int b) {
            return job_info[a].grid_pre_start < job_info[b].grid_pre_start;
            });
    }

    // 网格加工
    for (const auto& info : job_info) {
        output.grid_work[info.grid_work_pos].push_back(info.job_id);
    }
    for (auto& station : output.grid_work) {
        std::sort(station.begin(), station.end(), [&](int a, int b) {
            return job_info[a].grid_work_start < job_info[b].grid_work_start;
            });
    }

    // 机器人任务
    output.robot_tasks.resize(job_info.size());
    std::iota(output.robot_tasks.begin(), output.robot_tasks.end(), 0);
    std::sort(output.robot_tasks.begin(), output.robot_tasks.end(), [&](int a, int b) {
        return job_info[a].robot_start < job_info[b].robot_start;
        });

    // 总成加工
    for (const auto& info : job_info) {
        output.assembly[info.assembly_pos].push_back(info.job_id);
    }
    for (auto& station : output.assembly) {
        std::sort(station.begin(), station.end(), [&](int a, int b) {
            return job_info[a].assembly_start < job_info[b].assembly_start;
            });
    }

    return output;
}

void print_output(const OutputData& output) {
    // 打印门板预装
    std::cout << "\n===== 门板预装 =====" << std::endl;
    for (int i = 0; i < output.door_pre.size(); ++i) {
        std::cout << "工作站 " << i << ": ";
        for (int job : output.door_pre[i]) {
            std::cout << job << " ";
        }
        std::cout << std::endl;
    }

    // 打印门板加工
    std::cout << "\n===== 门板加工 =====" << std::endl;
    for (int i = 0; i < output.door_work.size(); ++i) {
        std::cout << "工作站 " << i << ": ";
        for (int job : output.door_work[i]) {
            std::cout << job << " ";
        }
        std::cout << std::endl;
    }

    // 打印网格预装
    std::cout << "\n===== 网格预装 =====" << std::endl;
    for (int i = 0; i < output.grid_pre.size(); ++i) {
        std::cout << "工作站 " << i << ": ";
        for (int job : output.grid_pre[i]) {
            std::cout << job << " ";
        }
        std::cout << std::endl;
    }

    // 打印网格加工
    std::cout << "\n===== 网格加工 =====" << std::endl;
    for (int i = 0; i < output.grid_work.size(); ++i) {
        std::cout << "工作站 " << i << ": ";
        for (int job : output.grid_work[i]) {
            std::cout << job << " ";
        }
        std::cout << std::endl;
    }

    // 打印机器人任务
    std::cout << "\n===== 机器人任务 =====" << std::endl;
    for (int job : output.robot_tasks) {
        std::cout << job << " ";
    }
    std::cout << std::endl;

    // 打印总成加工
    std::cout << "\n===== 总成加工 =====" << std::endl;
    for (int i = 0; i < output.assembly.size(); ++i) {
        std::cout << "工作站 " << i << ": ";
        for (int job : output.assembly[i]) {
            std::cout << job << " ";
        }
        std::cout << std::endl;
    }
}

void print_details(const std::vector<JobInfo>& job_info) {
    std::cout << "\n===================== 加工系统详细调度表 =====================" << std::endl;

    // 门板预装详情
    std::cout << "\n----- 门板预装 -----" << std::endl;
    std::cout << "任务ID | 工作站 | 开始时间 | 结束时间" << std::endl;
    for (const auto& info : job_info) {
        std::cout << std::setw(6) << info.job_id
            << " | " << std::setw(6) << info.door_pre_pos
            << " | " << std::setw(8) << info.door_pre_start
            << " | " << std::setw(8) << info.door_pre_end << std::endl;
    }

    // 门板加工详情
    std::cout << "\n----- 门板加工 -----" << std::endl;
    std::cout << "任务ID | 工作站 | 开始时间 | 结束时间" << std::endl;
    for (const auto& info : job_info) {
        std::cout << std::setw(6) << info.job_id
            << " | " << std::setw(6) << info.door_work_pos
            << " | " << std::setw(8) << info.door_work_start
            << " | " << std::setw(8) << info.door_work_end << std::endl;
    }

    // 网格预装详情
    std::cout << "\n----- 网格预装 -----" << std::endl;
    std::cout << "任务ID | 工作站 | 开始时间 | 结束时间" << std::endl;
    for (const auto& info : job_info) {
        std::cout << std::setw(6) << info.job_id
            << " | " << std::setw(6) << info.grid_pre_pos
            << " | " << std::setw(8) << info.grid_pre_start
            << " | " << std::setw(8) << info.grid_pre_end << std::endl;
    }

    // 网格加工详情
    std::cout << "\n----- 网格加工 -----" << std::endl;
    std::cout << "任务ID | 工作站 | 开始时间 | 结束时间" << std::endl;
    for (const auto& info : job_info) {
        std::cout << std::setw(6) << info.job_id
            << " | " << std::setw(6) << info.grid_work_pos
            << " | " << std::setw(8) << info.grid_work_start
            << " | " << std::setw(8) << info.grid_work_end << std::endl;
    }

    // 机器人详情
    std::cout << "\n----- 机器人 -----" << std::endl;
    std::cout << "任务ID | 开始时间 | 结束时间" << std::endl;
    for (const auto& info : job_info) {
        std::cout << std::setw(6) << info.job_id
            << " | " << std::setw(8) << info.robot_start
            << " | " << std::setw(8) << info.robot_end << std::endl;
    }

    // 总成加工详情
    std::cout << "\n----- 总成加工 -----" << std::endl;
    std::cout << "任务ID | 工作站 | 开始时间 | 结束时间 | 实际加工时间" << std::endl;
    for (const auto& info : job_info) {
        std::cout << std::setw(6) << info.job_id
            << " | " << std::setw(6) << info.assembly_pos
            << " | " << std::setw(8) << info.assembly_start
            << " | " << std::setw(8) << info.assembly_end
            << " | " << std::setw(10) << info.assembly_real_time << std::endl;
    }
}