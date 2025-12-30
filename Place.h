#pragma once
#include<iostream>
#include<vector>
#include<string>
#include<map>
#include<utility>
#include <typeindex>
using namespace std;
class Device;

enum class PlaceColor {
	Empty,         // 空的占位（explicit empty）
	Workpiece,    // 只放工件
	Device,            // 只放 AGV
	Storage,        // 只放立库资源（storage）
	OnDevice,              // AGV 上放着工件（carry_workpiece）
	InStorage,          // 工件已放入立库（workpiece_storage）
	CarryStorage      // 三元组合：AGV 搬运并占用 storage（carry_storage）


};


class Place {
public:
	string place_name;//库所的名字
	int capacity = 100;//库所容量
	vector<string>pre_arcs;//保存库所和它的前置变迁之间的对应关系
	vector<string>post_arcs;//保存库所和它的后置变迁之间的对应关系
	PlaceColor color = PlaceColor::Empty;
	bool is_time_place = false;//是否是赋时库所
	float proficiency; //人员熟练度
	int stage = 0;//对应工序（0--无工序，1--门板预装，2--门板加工，3--网格预装，4--网格加工，5--顶板加工，6--底板加工，7--机器人加工，8--总成加工）

	Place() = default;
	Place(const string& name, int cap, PlaceColor color, bool is_time_place = false, float prof = 1.0f, int stage = 0) : place_name(name), capacity(cap), color(color), is_time_place(is_time_place), proficiency(prof), stage(stage) {}


};