#include "Petrinet.h"
#include <fstream>


static void print_marking(const multimap<string, shared_ptr<Token>>& marking) {//marking打印
	cout << "=== Marking ===\n";
	if (marking.empty()) {
		cout << "(empty)\n";
		return;
	}

	string last_place = "";
	for (auto it = marking.begin(); it != marking.end(); ++it) {
		const string& place_name = it->first;
		auto& token = it->second;

		if (place_name != last_place) {
			if (last_place != "") cout << "\n";
			cout << place_name << ": ";
			last_place = place_name;
		}

		Token::print_token(token);
		cout << " ";
	}

	cout << "\n";
}

static void print_marking(const shared_ptr<Node> node,//marking打印至txt
	const string tran_name,
	const int lambda,
	const std::string& filename = "marking_output.txt") {
	// 打开文件（ios::out 覆盖写入，ios::app 追加写入）
	std::ofstream outfile(filename, std::ios::app);
	if (!outfile.is_open()) {  // 检查文件是否成功打开
		std::cerr << "错误：无法打开文件 " << filename << " 进行写入！" << std::endl;
		return;
	}

	if (tran_name == "t_b.10-11" || tran_name == "t_c.10-11" || tran_name == "t_e.4-5" || tran_name == "t_f.4-5") {
		outfile << "当前激发变迁组【" << node->already_workpiece[node->counter - 1] << "】  λ=" << lambda << "  激发代价" << node->cost << endl;
	}
	else outfile << "当前激发变迁" << tran_name << "  λ=" << lambda << "  激发代价" << node->cost << endl;

	auto& marking = node->marking;

	outfile << "=== Marking ===\n";
	if (marking.empty()) {
		outfile << "(empty)\n";
		outfile.close();  // 关闭文件
		return;
	}

	std::string last_place = "";
	for (auto it = marking.begin(); it != marking.end(); ++it) {
		const std::string& place_name = it->first;
		auto& token = it->second;

		if (place_name != last_place) {
			if (last_place != "") outfile << "\n";
			outfile << place_name << ": ";
			last_place = place_name;
		}

		// 注意：需要修改Token::print_token，让它支持输出到文件流
		// 这里先提供适配后的print_token重载版本
		Token::print_token(token, outfile);  // 输出到文件
		outfile << " ";
	}

	outfile << "\n";
	outfile.close();  // 关闭文件（也可依赖析构自动关闭，但显式关闭更安全）
}

bool Petrinet::judge_possible_firable_trans(const shared_ptr<Node>& curr_node, const string& trans_name)
{
	auto& tran_ptr = transitions[trans_name];
	const auto& m = curr_node->marking;
	//判断改变迁的前置库所是否满足条件
	//1.前置库所中都应该有托肯

	for (const string& pre_place_name : tran_ptr->pre_places) {
		if (m.find(pre_place_name) == m.end()) return false;
	}

	//2.后置库所满足容量要求
	for (const string& post_place_name : tran_ptr->post_places) {
		auto p_it = places.find(post_place_name);
		if (p_it == places.end()) continue;
		if (p_it->second->capacity == 1) {
			// 如果 multimap m 中已有 token，则不可激发
			if (m.find(post_place_name) != m.end())return false;

		}
	}

	//3.前置库所的token需要进行placelock
	//pb3前的变迁
	if (tran_ptr->place_lock) {
		string& brench_place = tran_ptr->post_places[0];
		for (string cheak_place : tran_ptr->pre_places) {
			if (places[cheak_place]->color == PlaceColor::Workpiece || places[cheak_place]->color == PlaceColor::OnDevice) {
				auto token_it = m.find(cheak_place);
				if (token_it != m.end()) {
					auto& workpiece_assemble_place = token_it->second->token_attribute->workpiece->assemble_place[brench_place];
					if (m.find(workpiece_assemble_place) != m.end()) {
						return false;
					}
				}
			}
			if (places[cheak_place]->color == PlaceColor::Workpiece) {
				auto token_it = m.find(cheak_place);
				if (token_it != m.end()) {
					auto& workpiece_assemble_place = token_it->second->token_attribute->workpiece->assemble_place[brench_place];
					if (token_it->second->token_attribute->workpiece->numid != all_workpiece_pre_seq[workpiece_assemble_place][place_workpiece_counter[workpiece_assemble_place]]) {
						return false;
					}

				}
			}
		}
	}

	//4.token分支选择特定库位进入
	//pb4_1-pb4-16前的变迁
	else if (tran_ptr->place_check) {
		string& brench_place = tran_ptr->pre_places[0];
		auto it = m.find(brench_place);
		bool check = false;
		if (it != m.end()) {
			auto& token_it = it->second;
			for (string place : tran_ptr->post_places) {
				if (token_it->token_attribute->workpiece->assemble_place[brench_place] != place)continue;
				check = true;
			}
			if (!check) {
				return false;
			}
		}
	}

	//5.特殊处理连续激发
	else if (trans_name == "t_b.10-11" || trans_name == "t_c.10-11" || trans_name == "t_e.4-5" || trans_name == "t_f.4-5") {
		if (curr_node->already_workpiece[curr_node->counter] == -1) {
			return false;
		}
		if (m.find("pb12") != m.end() || m.find("pc12") != m.end() || m.find("pe6") != m.end() || m.find("pf6") != m.end()) {
			return false;
		}
		if (m.find("prgv_h2") == m.end() || m.find("prgv_h1") == m.end()) {
			return false;
		}
	}

	return true;


}

priority_queue<pair<int, string>, vector<pair<int, string>>, Petrinet::CompareLambda> Petrinet::search_firable_transition(shared_ptr<Node>& node)
{

	priority_queue<pair<int, string>, vector<pair<int, string>>, Petrinet::CompareLambda> firable_trans;
	set<string>possible_firable_trans;
	//存储有token的库所其后置变迁可能是可激发变迁，并用中间变量（possible_firable_trans）存储
	possible_firable_trans = Get_possible_firable_trans(node->marking);
	//对possible_firable_trans里的每个变迁进行筛选(1.前置库所都要有弧上对应类型的token 2.后置库所需要满足容量要求)
	for (auto& it : possible_firable_trans) {
		if (judge_possible_firable_trans(node, it)) {
			//cout << "已经进入" << endl;
			const auto& firable_pre_place = transitions[it]->pre_places;
			const auto& curr_place = places[firable_pre_place[0]];
			const auto& curr_token = node->marking.find(firable_pre_place[0])->second;

			if (curr_place->is_time_place) {
				firable_trans.push({ compute_lambda(it,node) ,it });
			}
			else {
				firable_trans.push({ 0,it });
				break;
			}
		}
	}
	return firable_trans;
}



/****function::Get_possible_firable_trans****/
set<string> Petrinet::Get_possible_firable_trans(const multimap<string, shared_ptr<Token>>& m) {
	set<string>possible_firable_trans;
	for (auto itr = m.begin(); itr != m.end(); ++itr) {
		auto& place_name = itr->first;
		for (string post_trans : places[place_name]->post_arcs) {
			possible_firable_trans.emplace(post_trans);
		}
	}
	return possible_firable_trans;
}



void Petrinet::group_Fire(shared_ptr<Node>curr_node, int lambda) {

	vector<shared_ptr<TokenAttribute>> token_attribute_set;
	for (auto& tran_ptr : trans_group) {
		for (const string& pre_place_name : tran_ptr->pre_places) {
			if (places[pre_place_name]->color == PlaceColor::InStorage) {
				int wid = curr_node->already_workpiece[curr_node->counter];
				auto& delete_workpiece = workpiece_id[wid];
				auto [begin_it, end_it] = curr_node->marking.equal_range(pre_place_name);
				for (auto& it = begin_it; it != end_it; ++it) {
					auto& [place_name, token_ptr] = *it; // 结构化绑定拆分键值对
					// 筛选逻辑
					if (token_ptr->token_attribute->workpiece == delete_workpiece) {
						token_attribute_set.push_back(token_ptr->token_attribute);
						remove_token(curr_node, it);
						break;
					}
				}

			}
			else {
				auto token_iter = curr_node->marking.find(pre_place_name);
				if (token_iter == curr_node->marking.end()) {
					// 第一步：先把库所名转成独立字符串，避免拼接时隐式错误
					std::string place_name_str = pre_place_name;
					// 第二步：用格式化字符串，明确拼接库所名，且加换行/分隔符增强可读性
					std::string error_msg =
						"[group_Fire 异常] 未找到令牌！库所名称：" + place_name_str
						+ " | 节点地址：" + std::to_string(reinterpret_cast<uintptr_t>(curr_node.get())) // 兜底：用节点地址定位
						+ " | 触发变迁：" + tran_ptr->trans_name; // 补充变迁名，更易定位

					// 第三步：先打印到控制台（兜底，避免异常被吞时无日志）
					std::cerr << "[ERROR] " << error_msg << std::endl;
					// 第四步：抛出异常（确保字符串完整）
					throw std::runtime_error(error_msg);
				}
				token_attribute_set.push_back(token_iter->second->token_attribute);
				remove_token(curr_node, token_iter);
			}
		}
		for (const string& post_place_name : tran_ptr->post_places) {
			create_token(curr_node, post_place_name, token_attribute_set);
		}
		token_attribute_set.clear();
	}
	curr_node->counter++;
	return;
}
void Petrinet::single_Fire(const string& trans_name, shared_ptr<Node>curr_node, int lambda) {//单步激发
	auto& trans = transitions[trans_name];
	vector<shared_ptr<TokenAttribute>> token_attribute_set;
	token_attribute_set.reserve(trans->pre_places.size());
	curr_node->cost += lambda;

	//更新token的等待时间
	for (auto& [place, token] : curr_node->marking) {
		shared_ptr<Place> const& curr_place = places[place];

		if (curr_place->is_time_place) {
			token->waiting_time += lambda;
			int delay = compute_delay(token);

			if (token->waiting_time > delay) {
				token->waiting_time = delay;
			}
		}
	}
	if (trans->place_lock) {
		string& brench_place = trans->post_places[0];
		for (string cheak_place : trans->pre_places) {
			if (places[cheak_place]->color == PlaceColor::Workpiece) {
				auto token_it = curr_node->marking.find(cheak_place);
				if (token_it != curr_node->marking.end()) {
					auto& workpiece_assemble_place = token_it->second->token_attribute->workpiece->assemble_place[brench_place];
					place_workpiece_counter[workpiece_assemble_place] += 1;
				}
			}
		}
	}


	for (const string& pre_place_name : trans->pre_places) {
		auto token_iter = curr_node->marking.find(pre_place_name);//默认删第一个token的位置
		token_attribute_set.push_back(token_iter->second->token_attribute);

		remove_token(curr_node, token_iter);
	}

	for (const string& post_place_name : trans->post_places) {
		create_token(curr_node, post_place_name, token_attribute_set);
	}


}
void Petrinet::remove_token(shared_ptr<Node>curr_node, vector<shared_ptr<Token>>::iterator token_iter, string pre_place_name) //删除token
{
	auto [begin_it, end_it] = curr_node->marking.equal_range(pre_place_name);
	for (auto& marking_it = begin_it; marking_it != end_it; ++marking_it) {
		if (marking_it->second == (*token_iter)) { // 匹配到目标 Token
			curr_node->marking.erase(marking_it); // 删除该迭代器
			break; // 一个 Token 只在 marking 中出现一次，找到后直接退出
		}
	}
}
void Petrinet::remove_token(shared_ptr<Node>curr_node, multimap<string, shared_ptr<Token>>::iterator token_iter)//删除token
{
	curr_node->marking.erase(token_iter);
}
void Petrinet::create_token(shared_ptr<Node> curr_node, string place_name, vector<shared_ptr<TokenAttribute>>& attrs)//创造token
{
	auto color = places[place_name]->color;
	auto new_token = make_shared<Token>();
	new_token->inplace = place_name;
	new_token->waiting_time = 0;

	auto get_workpiece = [&](const vector<shared_ptr<TokenAttribute>>& v) -> shared_ptr<Workpiece> {
		for (auto& a : v) if (a && a->workpiece) return a->workpiece;
		return nullptr;
		};
	auto get_storage = [&](const vector<shared_ptr<TokenAttribute>>& v) -> shared_ptr<Storage> {
		for (auto& a : v) if (a && a->storage) return a->storage;
		return nullptr;
		};
	auto get_device = [&](const vector<shared_ptr<TokenAttribute>>& v) -> shared_ptr<Device> {
		for (auto& a : v) if (a && a->device) return a->device;
		return nullptr;
		};
	auto wp_ptr = get_workpiece(attrs);
	auto dev_ptr = get_device(attrs);
	auto st_ptr = get_storage(attrs);

	switch (color) {
	case PlaceColor::Empty:
		return;

	case PlaceColor::Workpiece:
		new_token->token_attribute = make_shared<TokenAttribute>(color, wp_ptr, nullptr, nullptr);
		break;

	case PlaceColor::Device:
		new_token->token_attribute = make_shared<TokenAttribute>(color, nullptr, dev_ptr, nullptr);
		break;

	case PlaceColor::Storage:
		new_token->token_attribute = make_shared<TokenAttribute>(color, nullptr, nullptr, st_ptr);
		break;

	case PlaceColor::OnDevice:
		new_token->token_attribute = make_shared<TokenAttribute>(color, wp_ptr, dev_ptr, nullptr);
		break;

	case PlaceColor::InStorage:
		new_token->token_attribute = make_shared<TokenAttribute>(color, wp_ptr, nullptr, st_ptr);
		break;

	case PlaceColor::CarryStorage:
		new_token->token_attribute = make_shared<TokenAttribute>(color, wp_ptr, dev_ptr, st_ptr);
		break;

	default:


		return;
	}

	curr_node->marking.emplace(place_name, new_token);

	//判断工件是否成套
	if (color == PlaceColor::InStorage) {
		curr_node->workpiece_instorage_counter[wp_ptr] += 1;
		if (curr_node->workpiece_instorage_counter[wp_ptr] == 4) {
			curr_node->already_workpiece[wp_ptr->robot_work] = wp_ptr->numid;
			//cout << "工件" << wp_ptr->numid << "已经准备好" << endl;
		}
	}

}

int Petrinet::seq_Fire(shared_ptr<Node> node,
	const vector<vector<int>>& door_pre_workpiece_seq,
	const vector<vector<int>>& grid_pre_workpiece_seq,
	const vector<int>& robot_workpiece_seq,
	const vector<vector<int>>& door_workpiece_workplace_seq,
	const vector<vector<int>>& grid_workpiece_workplace_seq,
	const vector<vector<int>>& assembly_workpiece_workplace_seq
)
{
	priority_queue<pair<int, string>, vector<pair<int, string>>, Petrinet::CompareLambda> candidate_transition;
	init_node(node, door_pre_workpiece_seq, grid_pre_workpiece_seq, robot_workpiece_seq);
	init_door_workpiece(door_workpiece_workplace_seq);
	init_grid_workpiece(grid_workpiece_workplace_seq);
	init_robot_workpiece(robot_workpiece_seq);
	init_assembly_workpiece(assembly_workpiece_workplace_seq);
	cout << "初始化完成" << endl;
	//这块之后写到初始化网当中
	trans_group.push_back(transitions["t_b.10-11"]);
	trans_group.push_back(transitions["t_b.11-12"]);
	trans_group.push_back(transitions["t_c.10-11"]);
	trans_group.push_back(transitions["t_c.11-12"]);
	trans_group.push_back(transitions["t_e.4-5"]);
	trans_group.push_back(transitions["t_e.5-6"]);
	trans_group.push_back(transitions["t_f.4-5"]);
	trans_group.push_back(transitions["t_f.5-6"]);

	candidate_transition = search_firable_transition(node);
	while (!candidate_transition.empty()) {
		pair<int, string> curr_trans = candidate_transition.top();
		const string& trans_name = curr_trans.second;
		const int lambda = curr_trans.first;

		if (trans_name == "t_b.10-11" || trans_name == "t_c.10-11" || trans_name == "t_e.4-5" || trans_name == "t_f.4-5") {
			group_Fire(node, lambda);
			cout << "当前激发变迁组【" << node->already_workpiece[node->counter - 1] << "】  λ=" << lambda << "  激发代价" << node->cost << endl;
			//print_marking(node, trans_name, lambda);
		}
		else {

			single_Fire(trans_name, node, lambda);
			//if(trans_name=="t_b.5-6.1")
			cout << "当前激发变迁" << trans_name << "  λ=" << lambda << "  激发代价" << node->cost << endl;
			//print_marking(node, trans_name, lambda);
		}
		candidate_transition = search_firable_transition(node);

	}
	return node->cost;
}


int Petrinet::compute_delay(shared_ptr<Token> curr_token) {
	const auto& curr_place = places[curr_token->inplace];
	int time = static_cast<int>(
		std::round(curr_place->proficiency *
			curr_token->token_attribute->workpiece->per_worktime[curr_place->stage]));
	return time;
}
int Petrinet::compute_lambda(const string& trans_name, shared_ptr<Node>& curr_node) {
	auto& tran_ptr = transitions[trans_name];
	auto& m = curr_node->marking;

	int maxlambda = 0;
	for (string pre_place_name : tran_ptr->pre_places) {
		auto& pre_place_ptr = places[pre_place_name];
		if (pre_place_ptr->is_time_place) {
			if (m.find(pre_place_name) == m.end())return -1;
			const auto& curr_token = m.find(pre_place_name)->second;

			int curr_lambda = compute_delay(curr_token) - curr_token->waiting_time;
			if (maxlambda < curr_lambda) {
				maxlambda = curr_lambda;
			}
		}
	}
	return maxlambda;
}
//***********************************************************初始化*****************************************************************
shared_ptr<Node> Petrinet::init_node(shared_ptr<Node> node, const vector<vector<int>>& door_pre_workpiece_seq, const vector<vector<int>>& grid_pre_workpiece_seq, const vector<int>& robot_workpiece_seq) {
	node->marking.clear();
	node->cost = 0;
	node->already_workpiece.resize(100, -1);
	if (door_pre_workpiece_seq.size() != init_door_places.size()) { cout << "传入门板二维数组与需要初始化的库所维度不匹配" << endl; return nullptr; }
	if (grid_pre_workpiece_seq.size() != init_grid_places.size()) { cout << "数组尺寸" << grid_pre_workpiece_seq.size() << "网格库所尺寸" << init_grid_places.size() << "传入网格二维数组与需要初始化的库所维度不匹配" << endl; return nullptr; }
	if (door_pre_workpiece_seq.size() == init_door_places.size()) { cout << "数组尺寸" << door_pre_workpiece_seq.size() << "网格库所尺寸" << init_door_places.size() << endl; }
	if (grid_pre_workpiece_seq.size() == init_grid_places.size()) { cout << "数组尺寸" << grid_pre_workpiece_seq.size() << "网格库所尺寸" << init_grid_places.size() << endl; }


	for (int i = 0; i < door_pre_workpiece_seq.size(); ++i) {
		auto& place_name = init_door_places[i];
		for (int id : door_pre_workpiece_seq[i]) {

			auto new_token = make_shared<Token>(place_name, make_shared<TokenAttribute>(PlaceColor::Workpiece, workpiece_id[id], nullptr, nullptr));
			node->marking.emplace(place_name, new_token);
			//workpiece_to_tokens[workpiece_id[id]].push_back(new_token);

		}
	}
	for (int i = 0; i < grid_pre_workpiece_seq.size(); ++i) {
		auto& place_name = init_grid_places[i];
		for (int id : grid_pre_workpiece_seq[i]) {

			auto new_token = make_shared<Token>(place_name, make_shared<TokenAttribute>(PlaceColor::Workpiece, workpiece_id[id], nullptr, nullptr));
			node->marking.emplace(place_name, new_token);
			//workpiece_to_tokens[workpiece_id[id]].push_back(new_token);

		}
	}
	for (int id = 0; id < robot_workpiece_seq.size(); ++id) {
		string top_place_name = "pe0";
		string bottom_place_name = "pf0";

		auto top_new_token = make_shared<Token>(top_place_name, make_shared<TokenAttribute>(PlaceColor::Workpiece, workpiece_id[robot_workpiece_seq[id]], nullptr, nullptr));
		auto bottom_new_token = make_shared<Token>(bottom_place_name, make_shared<TokenAttribute>(PlaceColor::Workpiece, workpiece_id[robot_workpiece_seq[id]], nullptr, nullptr));

		node->marking.emplace(top_place_name, top_new_token);
		node->marking.emplace(bottom_place_name, bottom_new_token);

		//workpiece_to_tokens[workpiece_id[robot_workpiece_seq[id]]].push_back(top_new_token);
		//workpiece_to_tokens[workpiece_id[robot_workpiece_seq[id]]].push_back(bottom_new_token);

	}
	auto rgv_b_token = make_shared<Token>("prgv_b", make_shared<TokenAttribute>(PlaceColor::Device, nullptr, make_shared<Device>("rgv_b"), nullptr));
	node->marking.emplace("prgv_b", rgv_b_token);
	auto rgv_c_token = make_shared<Token>("prgv_c", make_shared<TokenAttribute>(PlaceColor::Device, nullptr, make_shared<Device>("rgv_c"), nullptr));
	node->marking.emplace("prgv_c", rgv_c_token);
	auto agv_a_token = make_shared<Token>("pagv_a", make_shared<TokenAttribute>(PlaceColor::Device, nullptr, make_shared<Device>("agv_a"), nullptr));
	node->marking.emplace("pagv_a", agv_a_token);
	auto agv_d_token = make_shared<Token>("pagv_d", make_shared<TokenAttribute>(PlaceColor::Device, nullptr, make_shared<Device>("agv_d"), nullptr));
	node->marking.emplace("pagv_d", agv_d_token);
	auto rgv_h1_token = make_shared<Token>("prgv_h1", make_shared<TokenAttribute>(PlaceColor::Device, nullptr, make_shared<Device>("rgv_h1"), nullptr));
	node->marking.emplace("prgv_h1", rgv_h1_token);
	auto rgv_h2_token = make_shared<Token>("prgv_h2", make_shared<TokenAttribute>(PlaceColor::Device, nullptr, make_shared<Device>("rgv_h2"), nullptr));
	node->marking.emplace("prgv_h2", rgv_h2_token);

	for (int id = 0; id < 87; ++id) {
		string co1_id = "co1_" + to_string(id);
		node->marking.emplace("phouse1", make_shared<Token>("phouse1", make_shared<TokenAttribute>(PlaceColor::Storage, nullptr, nullptr, make_shared<Storage>(co1_id))));
	}

	for (int id = 0; id < 26; ++id) {
		string co2_id = "co2_" + to_string(id);
		node->marking.emplace("phouse2", make_shared<Token>("phouse2", make_shared<TokenAttribute>(PlaceColor::Storage, nullptr, nullptr, make_shared<Storage>(co2_id))));
	}
	cout << "节点初始化完成" << endl;
	return node;
}

void Petrinet::init_door_workpiece(const vector<vector<int>>& door_workpiece_workplace_seq)
{
	for (int i = 0; i < door_workpiece_workplace_seq.size(); ++i) {
		for (int id : door_workpiece_workplace_seq[i]) {
			workpiece_id[id]->assemble_place.emplace("pb3", door_workpiece_place[i]);
			all_workpiece_pre_seq[door_workpiece_place[i]].push_back(id);
			place_workpiece_counter[door_workpiece_place[i]] = 0;
		}
	}


	cout << "门板工件初始化完成" << endl;
}
void Petrinet::init_grid_workpiece(const vector<vector<int>>& grid_workpiece_workplace_seq)
{
	for (int i = 0; i < grid_workpiece_workplace_seq.size(); ++i) {
		for (int id : grid_workpiece_workplace_seq[i]) {
			workpiece_id[id]->assemble_place.emplace("pc3", grid_workpiece_place[i]);
			all_workpiece_pre_seq[grid_workpiece_place[i]].push_back(id);
			place_workpiece_counter[grid_workpiece_place[i]] = 0;
		}
	}
	cout << "网格工件初始化完成" << endl;

}
void Petrinet::init_assembly_workpiece(const vector<vector<int>>& assembly_workpiece_workplace_seq)
{
	for (int i = 0; i < assembly_workpiece_workplace_seq.size(); ++i) {
		for (int id : assembly_workpiece_workplace_seq[i]) {
			workpiece_id[id]->assemble_place.emplace("pd1", assembly_workpiece_place[i]);
		}
	}
	cout << "总成工件初始化完成" << endl;

}
void Petrinet::init_robot_workpiece(const vector<int>& robot_workpiece_workplace_seq) {
	for (int i = 0; i < robot_workpiece_workplace_seq.size(); ++i) {
		workpiece_id[robot_workpiece_workplace_seq[i]]->robot_work = i;
	}
	cout << "机器人工件初始化完成" << endl;

}



