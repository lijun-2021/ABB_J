#pragma once
#include "Token.h"
#include "Place.h"
#include "Transition.h"
using namespace std;




class SpaceZone {
public:
	string ID;//身份标识
	map<string, vector<vector<vector<float>>>> boundary_position;//空间区域边界位置，(位置ID,3维向量（x,y,z）)表示
	map<string, vector<vector<vector<float>>>> in;//空间区域入口位置，(位置ID,3维向量（x,y,z）)表示
	map<string, vector<vector<vector<float>>>> out;//空间区域出口位置，(位置ID,3维向量（x,y,z）)表示
	map<string, shared_ptr<Place>>places;//库所状态{托肯类型：数量}

	//	map<string, int>space_zone(string& token_attribute);//生成空间区域

	// 1. 默认构造函数（无参）：初始化ID为空字符串，容器默认空，智能指针nullptr
	SpaceZone() : ID("") {}

	// 2. 带参构造函数（仅初始化核心ID）：适合快速创建对象
	SpaceZone(const string& id) : ID(id) {}

	// 3. 全参构造函数（按需初始化所有核心成员）：适合复杂场景
	SpaceZone(const string& id,
		const map<string, vector<vector<vector<float>>>>& boundary,
		const map<string, vector<vector<vector<float>>>>& in_pos,
		const map<string, vector<vector<vector<float>>>>& out_pos,
		const map<string, shared_ptr<Place>>& ps)
		: ID(id), boundary_position(boundary), in(in_pos), out(out_pos), places(ps) {
	}
};

class Workpiece {
public:
	string ID;//身份标识
	string name_id;//K0
	string order_id;//订单号（同一订单最好一起加工）
	string serial_id;//系列号（同一系列号的工件是一样的，同一系列号的网格部分同时加工多个）
	int numid;//数字id

	vector<string> process_flow; // 工作流程<工序名称，所需设备名称>
	shared_ptr<SpaceZone> space_zone;//所处空间区域
	shared_ptr<Place> place;//所处库所
	//vector<pair<string, shared_ptr<Transition>>>schedule;//工作流程调度<工序名称，变迁对象指针>



	unordered_map<string, string> assemble_place;//(分支点库所：加工库所)
	int robot_work = -1;//机器人加工顺序(-1为还未分配)
	vector<int> per_worktime;//vector[stage] = 对应工序的加工时间
	//对应工序（0--无,1--门板预装，2--门板加工，3--网格预装，4--网格加工，5--顶板加工，6--底板加工，7--机器人加工，8--总成加工）
	Workpiece() = default;
	Workpiece(const string& ID) :ID(ID) {}
	Workpiece(const string& ID, int numid, int robot_work, const vector<int>& per_worktime) :ID(ID), robot_work(robot_work), numid(numid), per_worktime(per_worktime) {}
	void print_workpiece() const;
};

class Device {
public:
	string ID;//身份标识
	shared_ptr<Place> place;//所处库所
	int TimeInPlace; // 在库所内的停留时间
	vector<pair<string, shared_ptr<Transition>>>schedule;//工作流程调度<工序名称，变迁对象指针>
	// 1. 默认构造函数（无参）
	Device() : ID(""), TimeInPlace(0) {}

	// 2. 核心参数构造函数（初始化ID）
	Device(const string& id) : ID(id), TimeInPlace(0) {}

	// 3. 全参构造函数（复杂场景）
	Device(const string& id,
		const shared_ptr<Place>& p,
		int time_in_p,
		const vector<pair<string, shared_ptr<Transition>>>& sch)
		: ID(id), place(p), TimeInPlace(time_in_p), schedule(sch) {
	}
	~Device() = default;
};

class Storage {
public:
	string ID;//身份标识
	int TimeInPlace; // 在库所内的停留时间
	//vector<pair<string, shared_ptr<Transition>>>schedule;//工作流程调度<工序名称，变迁对象指针>
	Storage(const string& id) : ID(id), TimeInPlace(0) {}
};

class TokenAttribute {
public:
	PlaceColor color;
	shared_ptr<Workpiece> workpiece;
	shared_ptr<Device> device;
	shared_ptr<Storage> storage;
	TokenAttribute(PlaceColor color, shared_ptr<Workpiece> workpiece, shared_ptr<Device> device, shared_ptr<Storage> storage) :color(color), workpiece(workpiece), device(device), storage(storage) {}

	//string和颜色类型转换，用于txt的读写
	static string color_to_string(PlaceColor c)
	{
		switch (c) {
		case PlaceColor::Workpiece:		return "Workpiece";
		case PlaceColor::Device:		return "Device";
		case PlaceColor::Storage:		return "Storage";
		case PlaceColor::CarryStorage:	return"CarryStorage";
		case PlaceColor::OnDevice:		return"OnDevice";
		case PlaceColor::InStorage:		return"InStorage";
		case PlaceColor::Empty:			return"Empty";
		default:						return "Empty";
		}
	}

	static inline PlaceColor string_to_place_color(const std::string& s)
	{
		if (s == "Empty")          return PlaceColor::Empty;
		if (s == "Workpiece")      return PlaceColor::Workpiece;
		if (s == "Device")         return PlaceColor::Device;
		if (s == "Storage")        return PlaceColor::Storage;
		if (s == "OnDevice")       return PlaceColor::OnDevice;
		if (s == "InStorage")      return PlaceColor::InStorage;
		if (s == "CarryStorage")   return PlaceColor::CarryStorage;

		throw std::runtime_error("Unknown PlaceColor string: " + s);
	}
};

//class Human {
//public:
//	string ID;//身份标识
//	shared_ptr<SpaceZone>space_zone; // 设备所处的空间区域
//	//map<string, vector<vector<vector<float>>>> position;//("位置",3维向量（x,y,z）)表示
//	//map<string, vector<vector<vector<float>>>> velocity;//(“速度”,3维向量（x,y,z）)表示
//	//map<string, vector<vector<vector<float>>>> acceleration;//("加速度",3维向量（x,y,z）)表示
//	shared_ptr<Place> place;//所处库所
//	float time_in_space_zone; // 在空间区域内的停留时间
//	float time_in_place; // 在库所内的停留时间
//	vector<pair<string, shared_ptr<Transition>>> schedule;//工作流程调度<工序名称，变迁对象指针>
//
////	map<string, int>human(string& token_attribute);//生成空间区域
//};

