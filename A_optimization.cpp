// optimization.cpp
#include "A_optimization.h"
#include "A_constants.h"
#include "A_output.h"
#include <vector>
#include <queue>
#include <climits>
#include <numeric>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <random>
#include <unordered_set>
using namespace std;
using namespace chrono;
int N;
//随机数生成
mt19937_64 rng(12345);
double rnd() { return uniform_real_distribution<double>(0, 1)(rng); }
int rndi(int l, int r) { return uniform_int_distribution<int>(l, r)(rng); }
//工件信息数据
std::vector<int> door_pre_time;
std::vector<int> door_work_time;
std::vector<int> grid_pre_time;
std::vector<int> grid_work_time;
std::vector<int> assembly_work_time;
std::vector<string> series; //从json中读取的系列号信息
std::vector<std::vector<int>> grid_batches; //已经分好组的网格预装

std::vector<int> batch_series_id; // 数字表示每个批次的系列号，方便查找
std::unordered_map<int, std::vector<int>> series_id_to_batches; // 系列ID->所属批次索引  
//2->[2,3]比如数字系列号未2，它对应的批次号为2，3，用来记录同一系列号分几组的情况
std::unordered_map<int, int> job_to_batch_idx;    // 工件ID->批次索引

// 人员熟练度
std::vector<double> skill_door;
std::vector<double> skill_grid;
std::vector<double> skill_assembly;

//网格预装工件按照系列号进行分组
std::vector<std::vector<int>> calculate_grid_pre_order(const std::vector<int>& order) {
    //遍历工件id,放入哈希表
    std::unordered_map<std::string, std::vector<int>> diff_series; //型号->工件id
    for (int job_id : order)
    {
        std::string job_type = series[job_id];
        diff_series[job_type].push_back(job_id);
    }

    //对同一型号的工件进行平均分组
    std::vector<std::vector<int>> all_batches;  //存储最终分配信息
    batch_series_id.clear();
    series_id_to_batches.clear();
    job_to_batch_idx.clear();
    int total_series_num = 0;//后面为了简化字符串匹配，这里将型号通过数字反映
    for (auto& group : diff_series) {
        //std::string type = group.first;

        int current_series_id = total_series_num++;
        std::vector<int> jobs = group.second;
        int total = jobs.size();//对于一种工件有多少个

        int batch_num = ceil((double)total / 10); //向上取整ceil
        int base_num = total / batch_num;
        int extra = total % batch_num;

        //开始分组
        int start = 0;
        std::vector<int> current_series_batches;
        for (int i = 0; i < batch_num; i++) {
            std::vector<int> one_batch;//记录每一组的数据
            int batch_size = base_num;
            if (i < extra) {
                batch_size += 1;  //把余数进行分配
            }

            for (int j = 0; j < batch_size; j++) {
                if (start < total) {
                    one_batch.push_back(jobs[start]);
                    start++;
                }
            }

            int batch_idx = all_batches.size();
            all_batches.push_back(one_batch);//分好的每一组合并
            batch_series_id.push_back(current_series_id); //表示一个分组的序列号
            current_series_batches.push_back(batch_idx);

            for (int job : one_batch) //记录工件->批次的映射
            {
                job_to_batch_idx[job] = batch_idx;
            }
        }
        series_id_to_batches[current_series_id] = current_series_batches; //系列id->所属批次映射

    }
    return all_batches;
}

void init_global_batches() {
    std::vector<int> order(N);
    for (int i = 0; i < N; i++) {
        order[i] = i;
    }

    grid_batches = calculate_grid_pre_order(order);
}

//计算一组网格的加工时间系数，如果一起加工的多，那么会更快，一次最多加工10个工件
double get_batch_efficient(int batch_size)
{
    if (batch_size == 1)
    {
        return 1.0;
    }
    else if (batch_size >= 2 && batch_size <= 4)
    {
        return 0.9;
    }
    else if (batch_size >= 5 && batch_size <= 7)
    {
        return 0.8;
    }
    else if (batch_size >= 8 && batch_size <= 10)
    {
        return 0.7;
    }
    else
    {
        return 10;
    }
}

//总成部分优化
//返回完工时间
int optimize_assembly(const std::vector<int>& order, const std::vector<int>& door_finish,
    const std::vector<int>& grid_finish, std::vector<JobInfo>& job_time_info,
    long long& inner_iter_count)
{
    int n = order.size();
    std::vector<int> cur(n);//分配总成工位
    for (int i = 0; i < n; i++)
    {
        cur[i] = i % assembly;
    }

    //计算完工时间
    auto eval = [&](const std::vector<int>& assign, bool record_info = false) -> int
        {
            std::vector<int> position_available(assembly, 0);//工位可用时间初始化
            int robot_cur_time = 0;
            int makespan = 0;
            std::vector<int> door_out_time(N, 0);// 门板搬出立库时间
            std::vector<int> grid_out_time(N, 0);

            for (int i = 0; i < n; i++)
            {
                int job = order[i];
                int pos = assign[i];
                //开始加工时间考虑机器人加工完成时间以及工位空闲时间
                int robot_start = std::max({ door_finish[job], grid_finish[job], robot_cur_time });
                int robot_end = robot_start + robot_work_time;
                int robot_actual_end = std::max(robot_end, position_available[pos]);
                robot_cur_time = robot_actual_end;

                int start = std::max(robot_actual_end, position_available[pos]);
                int work_time = round(assembly_work_time[job] * skill_assembly[pos]);
                int end = start + work_time;
                position_available[pos] = end;
                makespan = std::max(makespan, end);

                //记录信息
                if (record_info)
                {
                    job_time_info[job].robot_start = robot_start;
                    job_time_info[job].robot_end = robot_actual_end;
                    job_time_info[job].assembly_pos = pos;
                    job_time_info[job].assembly_start = start;
                    job_time_info[job].assembly_end = end;
                    job_time_info[job].assembly_real_time = work_time;
                }
            }

            return makespan;
        };
    //记录初始分配方案
    int cur_val = eval(cur);
    int best_val = cur_val;
    std::vector<int> best_assembly_position = cur;
    //内层模拟退火
    double T = INNER_T_INIT;
    int no_improve = 0;
    inner_iter_count = 0;

    while (T > INNER_T_MIN && no_improve < INNER_NO_IMPROVE_LIMIT)
    {
        for (int iter = 0; iter < INNER_ITER_PER_T; ++iter)
        {
            inner_iter_count++;
            std::vector<int> next = cur;
            int pos = rndi(0, n - 1);
            next[pos] = rndi(0, assembly - 1);
            int next_val = eval(next);

            int delta = next_val - cur_val;
            if (delta < 0 || rnd() < exp(-delta / T))
            {
                cur = next;
                cur_val = next_val;
                if (cur_val < best_val)
                {
                    best_val = cur_val;
                    best_assembly_position = cur;
                    no_improve = 0;
                }
                else
                {
                    no_improve++;
                }
            }
        }
        T *= INNER_ALPHA;
    }

    eval(best_assembly_position, true);
    return best_val;
}

int evaluate_order(const std::vector<int>& order, std::vector<JobInfo>& job_info,
    long long& inner_iter)
{
    std::fill(job_info.begin(), job_info.end(), JobInfo{ 0 });
    for (int i = 0; i < N; ++i)
    {
        job_info[i].job_id = i;
    }

    // 门板工序
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
        std::greater<std::pair<int, int>>> door_pre_heap;//预装工位的空闲时间与ID 
    for (int i = 0; i < door_pre; ++i)
    {
        door_pre_heap.push({ 0, i });
    }

    std::vector<int> door_work_free_time(door_work, 0);
    std::vector<int> door_cache_free_time(door_work, 0);
    std::vector<int> door_finish(N, 0);
    
    //门板完工时间
    for (int i = 0; i < N; ++i)
    {
        int job = order[i];
        //门板预装
        auto [pre_avail, pre_pos] = door_pre_heap.top();
        door_pre_heap.pop();
        int pre_start = pre_avail;
        int pre_end = pre_start + door_pre_time[job];
        job_info[job].door_pre_pos = pre_pos;
        job_info[job].door_pre_start = pre_start;
        job_info[job].door_pre_end = pre_end;
        door_pre_heap.push({ pre_end, pre_pos });

        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
            std::greater<std::pair<int, int>>> work_select_heap;  //这个优先队列保留最早能够开始进行加工的工位，
        for (int k = 0; k < door_work; k++)
        {
            int start = std::max({ pre_end, door_work_free_time[k], door_cache_free_time[k] });
            int work_time = round(door_work_time[job] * skill_door[k]);
            int end = start + work_time;
            work_select_heap.push({ end, k });
        }

        auto [best_end, best_pos] = work_select_heap.top();
        int work_start = std::max({ pre_end, door_work_free_time[best_pos], door_cache_free_time[best_pos] });

        job_info[job].door_work_pos = best_pos;
        job_info[job].door_work_start = work_start;
        job_info[job].door_work_end = best_end;
        door_finish[job] = best_end;

        door_cache_free_time[best_pos] = work_start;
        door_work_free_time[best_pos] = best_end;
    }

    // 网格加工
    std::vector<bool> batch_pre_completed(grid_batches.size(), false);//记录每个预装分组是否完成
    std::unordered_set<int> series_completed; // 已完成的系列数字ID

    std::vector<int> grid_work_free_time(grid_work, 0);
    std::vector<int> grid_cache_free_time(grid_work, 0);
    std::vector<int> grid_finish(N, 0);

    int grid_pre_free_time = 0; //网格加工的空闲时间
    std::vector<int> grid_pre_end_time(N, 0); //记录每个工件预装完成时间

    for (int i = 0; i < N; ++i)
    {
        int job = order[i];
        int batch_idx = job_to_batch_idx[job]; // 找到工件分配的批次
        int current_series_id = batch_series_id[batch_idx]; // 查批次的系列数字ID
        const auto& batch = grid_batches[batch_idx];

        if (series_completed.find(current_series_id) == series_completed.end())  //如果这一组没有预装完成，那么要将这个系列号的所有工件进行加工
        {
            // 遍历该系列的所有批次，连续加工
            for (int bid : series_id_to_batches[current_series_id]) {
                if (batch_pre_completed[bid]) continue;
                const auto& batch = grid_batches[bid];
                int batch_size = batch.size();
                int first_job = batch[0]; //取第一个工件的时间表示一组时间
                int single_pre_time = grid_pre_time[first_job];
                int batch_pre_time = round(single_pre_time * batch_size * get_batch_efficient(batch_size) / grid_pre); //计算一组工件的完工时间
                int batch_pre_start = grid_pre_free_time;
                int batch_pre_end = batch_pre_start + batch_pre_time;//计算一组工件完成时间

                for (int job : batch)
                {
                    job_info[job].grid_pre_pos = 0;
                    job_info[job].grid_pre_start = batch_pre_start;
                    job_info[job].grid_pre_end = batch_pre_end;
                    grid_pre_end_time[job] = batch_pre_end;
                }
                grid_pre_free_time = batch_pre_end;
                batch_pre_completed[bid] = true;

            }
            series_completed.insert(current_series_id);
        }
        int job_pre_end = grid_pre_end_time[job];


//网格加工
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
            std::greater<std::pair<int, int>>> work_select_heap;
        for (int k = 0; k < grid_work; k++)
        {
            int start = std::max({ job_pre_end, grid_work_free_time[k], grid_cache_free_time[k] });
            int work_time = round(grid_work_time[job] * skill_grid[k]);
            int end = start + work_time;
            work_select_heap.push({ end, k });
        }

        auto [best_end, best_pos] = work_select_heap.top();
        int work_start = std::max({ job_pre_end, grid_work_free_time[best_pos], grid_cache_free_time[best_pos] });

        job_info[job].grid_work_pos = best_pos;
        job_info[job].grid_work_start = work_start;
        job_info[job].grid_work_end = best_end;
        grid_finish[job] = best_end;

        grid_cache_free_time[best_pos] = work_start;
        grid_work_free_time[best_pos] = best_end;
    }

    // 总成装配
    long long inner_iter_single = 0;
    int assembly_makespan = optimize_assembly(order, door_finish, grid_finish, job_info, inner_iter_single);
    inner_iter += inner_iter_single;
    return assembly_makespan;
}

OutputData run_optimization() {
    // 初始化任务加工顺序
    std::vector<int> cur_order(N);
    init_global_batches();
    std::iota(cur_order.begin(), cur_order.end(), 0);

    std::vector<JobInfo> job_info(N);
    std::vector<JobInfo> best_job_info(N);
    int best_makespan = INT_MAX;

    // 迭代计数
    long long outer_total_iter = 0;
    long long inner_total_iter = 0;

    // 模拟退火
    auto start_time = std::chrono::high_resolution_clock::now();
    long long inner_iter_init = 0;
    int cur_makespan = evaluate_order(cur_order, job_info, inner_iter_init);
    inner_total_iter += inner_iter_init;
    best_makespan = cur_makespan;
    best_job_info = job_info;
    std::vector<int> best_order = cur_order;

    double T = OUTER_T_INIT;
    int no_improve = 0;

    std::cout << "初始最大完工时间: " << cur_makespan << std::endl;
    std::cout << "初始内部迭代次数: " << inner_iter_init << std::endl;

    while (T > OUTER_T_MIN && no_improve < OUTER_NO_IMPROVE_LIMIT && inner_total_iter < INNER_MAX)
    {
        for (int iter = 0; iter < OUTER_ITER_PER_T; ++iter)
        {
            outer_total_iter++;
            std::vector<int> new_order = cur_order;
            int a = rndi(0, N - 1), b = rndi(0, N - 1);
            std::swap(new_order[a], new_order[b]);

            // 评估新顺序
            long long inner_iter_single = 0;
            int new_makespan = evaluate_order(new_order, job_info, inner_iter_single);
            inner_total_iter += inner_iter_single;

            int delta = new_makespan - cur_makespan;
            if (delta < 0 || rnd() < exp(-delta / T))
            {
                cur_order = new_order;
                cur_makespan = new_makespan;
                if (cur_makespan < best_makespan)
                {
                    best_makespan = cur_makespan;
                    best_order = cur_order;
                    best_job_info = job_info;
                    no_improve = 0;
                    std::cout << "找到更优解: 完工时间=" << best_makespan
                        << " 温度=" << T
                        << " 当前外部迭代=" << outer_total_iter
                        << " 当前内部迭代=" << inner_total_iter << std::endl;
                }
                else
                {
                    no_improve++;
                }
            }
        }
        T *= OUTER_ALPHA;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur = end_time - start_time;
    std::cout << "\n===== 优化结果 =====" << std::endl;
    std::cout << "最优最大完工时间: " << best_makespan << std::endl;
    std::cout << "外层SA总迭代次数(任务排序优化): " << outer_total_iter << std::endl;
    std::cout << "内层SA总迭代次数(装配工位优化): " << inner_total_iter << std::endl;
    std::cout << "算法总耗时: " << dur.count() << " 秒" << std::endl;

    // 生成并输出结果
    OutputData output = generate_output_data(best_job_info);
    return output;
    //print_output(output);
    //print_details(best_job_info);
}