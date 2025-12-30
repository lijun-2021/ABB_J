
#include<iostream>
#include <chrono>
#include"Process.h"
#include"read_json.h"
#include "Token.h"
using namespace std;

void Workpiece::print_workpiece() const {
	std::cout << "==================================== 工件信息 ====================================" << std::endl;
	std::cout << "ID: " << ID << std::endl;
	std::cout << "数字ID: " << numid << std::endl;
	std::cout << "机器人加工顺序: " << (robot_work == -1 ? "未设置" : std::to_string(robot_work)) << std::endl;



	// 打印各工序加工时间
	std::cout << "各工序加工时间: " << std::endl;
	const std::vector<std::string> stageNames = {
		"0--无", "1--门板预装", "2--门板加工", "3--网格预装",
		"4--网格加工", "5--顶板加工", "6--底板加工",
		"7--机器人加工", "8--总成加工"
	};
	for (size_t i = 0; i < per_worktime.size(); ++i) {
		if (i < stageNames.size()) {
			std::cout << "  " << stageNames[i] << ": " << per_worktime[i] << "单位时间" << std::endl;
		}
		else {
			std::cout << "  未知工序(" << i << "): " << per_worktime[i] << "单位时间" << std::endl;
		}
	}

	// 打印所处库所（如果有）
	std::cout << "所处库所: " << (place ? place->place_name : "未设置") << std::endl;


	// 打印分支点与加工库所映射
	std::cout << "分支点-加工库所映射数: " << assemble_place.size() << std::endl;
	for (const auto& pair : assemble_place) {
		std::cout << "  分支点库所: " << pair.first << " -> 加工库所: " << pair.second << std::endl;
	}
	std::cout << "================================================================================" << std::endl;

}