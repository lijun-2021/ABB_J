#include<iostream>
#include <chrono>
#include"Petrinet.h"
#include"read_json.h"
#include"Process.h"
#include"DebugTool.h"
#include "A_optimization.h"
////////////////////////////////////////////////////////////////////
//							_ooOoo_								  //
//						   o8888888o							  //	
//						   88" . "88							  //	
//						   (| ^_^ |)							  //	
//						   O\  =  /O							  //
//						____/`---'\____							  //						
//					  .'  \\|     |//  `.						  //
//					 /  \\|||  :  |||//  \						  //	
//				    /  _||||| -:- |||||-  \						  //
//				    |   | \\\  -  /// |   |						  //
//					| \_|  ''\---/''  |   |						  //		
//					\  .-\__  `-`  ___/-. /						  //		
//				  ___`. .'  /--.--\  `. . ___					  //	
//				."" '<  `.___\_<|>_/___.'  >'"".				  //
//			  | | :  `- \`.;`\ _ /`;.`/ - ` : | |				  //	
//			  \  \ `-.   \_ __\ /__ _/   .-` /  /                 //
//		========`-.____`-.___\_____/___.-`____.-'========		  //	
//				             `=---='                              //
//		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^        //
//         佛祖保佑       永无BUG		永不修改				  //
////////////////////////////////////////////////////////////////////

int main() {


	// 1. 创建Petri网对象
	Petrinet pn;
	read_place_json(pn);
	read_trans_json(pn);
	read_tokens_json(pn);
	//PetriNetDebugTool::printPetriNetInfo(pn.places, pn.transitions);
	auto node = make_shared<Node>();

	//调度结果
	OutputData result = run_optimization();
	vector<vector<int>> door_pre_workpiece_seq = result.door_pre;
	vector<vector<int>> door_workpiece_workplace_seq = result.door_work;
	vector<vector<int>> grid_pre_workpiece_seq = result.grid_pre;
	vector<vector<int>> grid_workpiece_workplace_seq = result.grid_work;
	vector<int> robot_workpiece_seq = result.robot_tasks;
	vector<vector<int>> assembly_workpiece_workplace_seq = result.assembly;

	pn.init_door_places = { "pb1_1","pb1_2","pb1_3","pb1_4" };//（固定表）
	pn.init_grid_places = { "pc1_1","pc1_2","pc1_3","pc1_4","pc1_5" };//（固定表)
	pn.door_workpiece_place = { "pb4_1","pb4_2" ,"pb4_3" ,"pb4_4" ,"pb4_5" ,"pb4_6" ,"pb4_7" ,"pb4_8" ,"pb4_9" ,"pb4_10" ,"pb4_11" ,"pb4_12" ,"pb4_13" ,"pb4_14" ,"pb4_15" ,"pb4_16" };	//传入的门板数组转成库所名（固定表）（0-15：pb4_1-pb4-16）
	pn.grid_workpiece_place = { "pc4_1","pc4_2" ,"pc4_3" ,"pc4_4" ,"pc4_5" ,"pc4_6" ,"pc4_7" ,"pc4_8" ,"pc4_9" ,"pc4_10" };	//传入的网格数组转成库所名（固定表）（0-9：pc4_1-pc4-10）
	pn.assembly_workpiece_place = { "pd2_1","pd2_2" ,"pd2_3" ,"pd2_4" ,"pd2_5" ,"pd2_6" ,"pd2_7" ,"pd2_8" ,"pd2_9" ,"pd2_10" ,"pd2_11" ,"pd2_12" ,"pd2_13" ,"pd2_14" ,"pd2_15" };//传入的总成数组转成库所名（固定表）（0-14：pd2_1-pd2-15）

	pn.seq_Fire(node, door_pre_workpiece_seq, grid_pre_workpiece_seq, robot_workpiece_seq, door_workpiece_workplace_seq, grid_workpiece_workplace_seq, assembly_workpiece_workplace_seq);

	return 0;
}