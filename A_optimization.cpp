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

using namespace std;
using namespace chrono;
int N;
//随机数生成
mt19937_64 rng(12345);
double rnd() { return uniform_real_distribution<double>(0, 1)(rng); }
int rndi(int l, int r) { return uniform_int_distribution<int>(l, r)(rng); }

std::vector<int> door_pre_time;
std::vector<int> door_work_time;
std::vector<int> grid_pre_time;
std::vector<int> grid_work_time;
std::vector<int> assembly_work_time;

// 熟练度
std::vector<double> skill_door;
std::vector<double> skill_grid;
std::vector<double> skill_assembly;


int optimize_assembly(const std::vector<int>& order, const std::vector<int>& door_finish,
    const std::vector<int>& grid_finish, std::vector<JobInfo>& job_time_info,
    long long& inner_iter_count)
{
    int n = order.size();
    std::vector<int> cur(n);
    for (int i = 0; i < n; i++)
    {
        cur[i] = i % assembly;
    }

    // 评估函数
    auto eval = [&](const std::vector<int>& assign, bool record_info = false) -> int
        {
            std::vector<int> position_available(assembly, 0);
            int robot_cur_time = 0;
            int makespan = 0;
            std::vector<int> door_out_time(N, 0);
            std::vector<int> grid_out_time(N, 0);

            for (int i = 0; i < n; i++)
            {
                int job = order[i];
                int pos = assign[i];
                int robot_start = std::max({ door_finish[job], grid_finish[job], robot_cur_time });
                int robot_end = robot_start + robot_work_time;
                int robot_actual_end = std::max(robot_end, position_available[pos]);
                robot_cur_time = robot_actual_end;

                int start = std::max(robot_actual_end, position_available[pos]);
                int work_time = round(assembly_work_time[job] * skill_assembly[pos]);
                int end = start + work_time;
                position_available[pos] = end;
                makespan = std::max(makespan, end);

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

    int cur_val = eval(cur);
    int best_val = cur_val;
    std::vector<int> best_assembly_position = cur;
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
        std::greater<std::pair<int, int>>> door_pre_heap;
    for (int i = 0; i < door_pre; ++i)
    {
        door_pre_heap.push({ 0, i });
    }

    std::vector<int> door_work_free_time(door_work, 0);
    std::vector<int> door_cache_free_time(door_work, 0);
    std::vector<int> door_finish(N, 0);

    for (int i = 0; i < N; ++i)
    {
        int job = order[i];
        auto [pre_avail, pre_pos] = door_pre_heap.top();
        door_pre_heap.pop();
        int pre_start = pre_avail;
        int pre_end = pre_start + door_pre_time[job];
        job_info[job].door_pre_pos = pre_pos;
        job_info[job].door_pre_start = pre_start;
        job_info[job].door_pre_end = pre_end;
        door_pre_heap.push({ pre_end, pre_pos });

        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
            std::greater<std::pair<int, int>>> work_select_heap;
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
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
        std::greater<std::pair<int, int>>> grid_pre_heap;
    for (int i = 0; i < grid_pre; ++i)
    {
        grid_pre_heap.push({ 0, i });
    }
    std::vector<int> grid_work_free_time(grid_work, 0);
    std::vector<int> grid_cache_free_time(grid_work, 0);
    std::vector<int> grid_finish(N, 0);

    for (int i = 0; i < N; ++i)
    {
        int job = order[i];
        auto [pre_avail, pre_pos] = grid_pre_heap.top();
        grid_pre_heap.pop();
        int pre_start = pre_avail;
        int pre_end = pre_start + grid_pre_time[job];
        job_info[job].grid_pre_pos = pre_pos;
        job_info[job].grid_pre_start = pre_start;
        job_info[job].grid_pre_end = pre_end;
        grid_pre_heap.push({ pre_end, pre_pos });

        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
            std::greater<std::pair<int, int>>> work_select_heap;
        for (int k = 0; k < grid_work; k++)
        {
            int start = std::max({ pre_end, grid_work_free_time[k], grid_cache_free_time[k] });
            int work_time = round(grid_work_time[job] * skill_grid[k]);
            int end = start + work_time;
            work_select_heap.push({ end, k });
        }

        auto [best_end, best_pos] = work_select_heap.top();
        int work_start = std::max({ pre_end, grid_work_free_time[best_pos], grid_cache_free_time[best_pos] });

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