#pragma once
#include"Place.h"
#include"Transition.h"
#include"Token.h"
#include"A_constants.h"
#include<unordered_set>
#include<set>
#include<queue>
/***************************************Node**********************************************************/
class Node {
public:
	multimap<string, shared_ptr<Token>>marking;//节点标识
	unordered_map<shared_ptr<Workpiece>, int> workpiece_instorage_counter;//初始化所有workpiece，int=0
	vector<int> already_workpiece;//(顺序：workpiece_id)初始化所有顺序并将id全部置为-1
	int cost = 0;//消耗时间
	int counter = 0;//每次激发already_workpiece加1

};
/***************************************Petrinet**********************************************************/
class Petrinet {
public:
	//--------------------petri网相关----------------------
	unordered_map<string, shared_ptr<Place>>places;//保存的是这个petri网里所有库所的对象，格式为{"库所id": 库所对象指针}
	unordered_map<string, shared_ptr<Transition>>transitions;//保存的是这个petri网里所有变迁的对象，格式为{ "变迁id": 变迁对象指针 }
	unordered_map<string, shared_ptr<Workpiece>>workpiece;
	unordered_map<string, shared_ptr<Device>>device;
	//---------------------初始化相关----------------------

	multimap<string, shared_ptr<Token>>m0;//初始状态{"inplace":托肯指针}
	vector<shared_ptr<Transition>> trans_group;//需要连续激发的变迁
	unordered_map<string, vector<int>> all_workpiece_pre_seq;//string为库所名（pc2_1-pc2_10,pb4_1-pb4-16以及pc4_1-pc4_10:工件加工的序列）
	unordered_map<string, int> place_workpiece_counter;//工位对应的计数器，用于记录每个工位加工到哪里了

	//---------------------对应关系表----------------------

	vector<shared_ptr<Workpiece>>workpiece_id;//（工件id：工件指针）（固定表）工件id找工件
	vector<string>init_door_places;//（固定表）
	vector<string>init_grid_places;//（固定表)
	vector<string> door_workpiece_place;	//传入的门板数组转成库所名（固定表）（0-15：pb4_1-pb4-16）
	vector<string> grid_workpiece_place;	//传入的网格数组转成库所名（固定表）（0-9：pc4_1-pc4-10）
	vector<string> pre_grid_workpiece_place;//传入的预装网格数组转成库所名（固定表）（0-9：pc1_1-pc1-10）
	vector<string> assembly_workpiece_place;//传入的总成数组转成库所名（固定表）（0-14：pd2_1-pd2-15）


	struct CompareLambda {
		bool operator()(const pair<int, string>& a, const pair<int, string>& b) {
			if (a.first != b.first) {
				return a.first > b.first; // lambda升序
			}
			return a.second > b.second;
		}
	};
	int compute_delay(shared_ptr<Token> token, const vector<vector<int>> grid_pre_workpiece_seq);

	//priority_queue<pair<int, string>, vector<pair<int, string>>, Petrinet::CompareLambda> search_firable_transition(const multimap<string, shared_ptr<Token>>& m, const unordered_map<string, int>& ctrl_m);
	//寻找可激发变迁  F1
	set<string>Get_possible_firable_trans(const multimap<string, shared_ptr<Token>>& m);//存储有token的库所其后置变迁可能是可激发变迁，并用中间变量（possible_firable_trans）存储 F1-1
	bool judge_possible_firable_trans(const shared_ptr<Node>& curr_node, const string& trans_name);
	priority_queue<pair<int, string>, vector<pair<int, string>>, Petrinet::CompareLambda> search_firable_transition(shared_ptr<Node>& node, const vector<vector<int>> grid_pre_workpiece_seq);
	void group_Fire(shared_ptr<Node> curr_node, int lambda);
	void single_Fire(const string& trans_name, shared_ptr<Node> curr_node, int lambda, vector<vector<int>> grid_pre_workpiece_seq);


	void create_token(shared_ptr<Node> curr_node, string place_name, vector<shared_ptr<TokenAttribute>>& attrs);
	int seq_Fire(shared_ptr<Node> node, const vector<vector<int>>& door_pre_workpiece_seq, const vector<vector<int>>& grid_pre_workpiece_seq, const vector<int>& robot_workpiece_seq, const vector<vector<int>>& door_workpiece_workplace_seq, const vector<vector<int>>& grid_workpiece_workplace_seq, const vector<vector<int>>& assembly_workpiece_workplace_seq);
	void remove_token(shared_ptr<Node>curr_node, multimap<string, shared_ptr<Token>>::iterator token_iter);
	void remove_token(shared_ptr<Node>curr_node, vector<shared_ptr<Token>>::iterator token_iter, string pre_place_name);

	int compute_delay(shared_ptr<Token> curr_token);
	int compute_lambda(const string& trans_name, shared_ptr<Node>& curr_node);
	int compute_lambda(const string& trans_name, shared_ptr<Node>& curr_node, const vector<vector<int>> grid_pre_workpiece_seq);
	shared_ptr<Node> init_node(shared_ptr<Node> node, const vector<vector<int>>& door_pre_workpiece_seq, const vector<vector<int>>& grid_pre_workpiece_seq, const vector<int>& robot_workpiece_seq);
	void init_door_workpiece(const vector<vector<int>>& door_workpiece_workplace);
	void init_grid_workpiece(const vector<vector<int>>& grid_workpiece_workplace_seq);
	void init_assembly_workpiece(const vector<vector<int>>& assembly_workpiece_workplace_seq);
	void init_robot_workpiece(const vector<int>& robot_workpiece_workplace_seq);
	void init_net(const vector<vector<int>>& Workpiece_seq, const vector<vector<int>>& Workpiece_workplace);

	void add_place(shared_ptr<Place> place) {
		places[place->place_name] = place;
	}
	void add_transition(shared_ptr<Transition> trans) {
		transitions[trans->trans_name] = trans;
	}


};

