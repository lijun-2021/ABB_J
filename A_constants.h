#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <vector>
#include<string>
// 任务数量
//const int N = 100;
extern int N;
// 工人数量
const int door_pre = 4;
const int door_work = 16;
const int grid_pre = 2;
const int grid_work = 10;
const int robot = 1;
const int assembly = 15;
const int robot_work_time = 6;

// 立库库位
const int TOTAL_STORAGE = 87;
const int DOOR_STORAGE = 20;
const int GRID_STORAGE = 60;

// 外层模拟退火参数
const double OUTER_T_INIT = 200;
const double OUTER_T_MIN = 1e-3;
const double OUTER_ALPHA = 0.995;
const int OUTER_ITER_PER_T = 100;
const int OUTER_NO_IMPROVE_LIMIT = 70000;
// 内层模拟退火参数
const double INNER_T_INIT = 200;
const double INNER_T_MIN = 1e-3;
const double INNER_ALPHA = 0.995;
const int INNER_ITER_PER_T = 100;
const int INNER_NO_IMPROVE_LIMIT = 1000;
const int INNER_MAX = 100000000;

// 工件数据定义
extern std::vector<int> door_pre_time;
extern std::vector<int> door_work_time;
extern std::vector<int> grid_pre_time;
extern std::vector<int> grid_work_time;
extern std::vector<int> assembly_work_time;

extern std::vector<std::string> series; //系列号，同一系列号的网格要一起加工
extern std::vector<int> priority; //3-停止并更换 2-做完并更换 1-正常订单
extern std::vector<std::vector<int>> grid_batches; //分组后的工件

// 工人熟练度定义
extern std::vector<double> skill_door;
extern std::vector<double> skill_grid;
extern std::vector<double> skill_assembly;

#endif