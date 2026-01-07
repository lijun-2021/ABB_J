#ifndef DATA_H
#define DATA_H

#include <vector>
#include <unordered_map>

// 任务信息结构体
struct JobInfo {
    int job_id;
    // 门板预装
    int door_pre_pos;
    int door_pre_start;
    int door_pre_end;
    // 门板加工
    int door_work_pos;
    int door_work_start;
    int door_work_end;

    // 网格预装
    int grid_pre_pos;
    int grid_pre_start;
    int grid_pre_end;
    // 网格加工
    int grid_work_pos;
    int grid_work_start;
    int grid_work_end;

    // 机器人
    int robot_start;
    int robot_end;

    // 总成
    int assembly_pos;
    int assembly_start;
    int assembly_end;
    int assembly_real_time;
};

// 工作站信息结构体
struct Station {
    int station_id;
    std::vector<std::pair<int, std::pair<int, int>>> tasks; // 任务id, <开始时间, 结束时间>
};

// 输出数据结构
struct OutputData {
    std::vector<std::vector<int>> door_pre;    // 门板预装: [工作站][任务ID,任务ID,...]
    std::vector<std::vector<int>> door_work;   // 门板加工: [工作站][任务ID,任务ID,...]
    std::vector<std::vector<int>> grid_pre;    // 网格预装: [工作站][任务ID,任务ID,...]
    std::vector<std::vector<int>> grid_work;   // 网格加工: [工作站][任务ID,任务ID,...]
    std::vector<int> robot_tasks;              // 机器人任务序列[任务ID,任务ID,...]
    std::vector<std::vector<int>> assembly;    // 总成加工: [工作站][任务ID,任务ID,...]
};

#endif
