#pragma once
#include <iostream>
#include <fstream>
#include "rapidjson/document.h"
#include"rapidjson/istreamwrapper.h"
#include"Place.h"
#include"Transition.h"
#include"Petrinet.h"
#include<memory>
#include<tuple>
#include "A_constants.h"

constexpr auto Place_path = "./data/ABB_DB_place_1.json";
constexpr auto Transition_path = "./data/ABB_DB_trans_1.json";
constexpr auto Token_path = "./data/ABB_DB_token_1.json";



//读取json文件，初始化库所信息
inline void read_place_json(Petrinet& petrinet) {
    //流文件打开json文件
    std::ifstream ifs(Place_path);//创建输入文件流对象
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return;
    }
    // 读取JSON数据
    rapidjson::IStreamWrapper isw(ifs);//创建IStreamWrapper对象
    rapidjson::Document doc;//创建Document对象
    doc.ParseStream(isw);//解析JSON数据

    //是否解析成功
    if (doc.HasParseError()) {
        std::cerr << "Failed to parse JSON" << std::endl;
        return;
    }

    skill_door.clear();
    skill_grid.clear();
    skill_assembly.clear();

    // 遍历每个对象
    for (auto itr1 = doc.MemberBegin(); itr1 != doc.MemberEnd(); ++itr1) {
        auto place_ptr = make_shared<Place>(); //生成Place对象，创建Place智能指针
        place_ptr->place_name = itr1->name.GetString();//获取库所名称
        place_ptr->capacity = itr1->value["capacity"].GetInt();//获取capacity的值
        //place_ptr->is_control_place = itr1->value["is_control_place"].GetInt();
        place_ptr->is_time_place = itr1->value["is_time_place"].GetBool();
        place_ptr->proficiency = itr1->value["proficiency"].GetFloat();
        place_ptr->stage = itr1->value["stage"].GetInt();
        /* place_ptr->color = TokenAttribute::string_to_place_color(itr1->value["color"].GetString());*/

        std::string name = place_ptr->place_name;
        if (name.find("pb5") != std::string::npos)
        {
            skill_door.push_back(place_ptr->proficiency);
        }
        else if (name.find("pc5") != std::string::npos)
        {
            skill_grid.push_back(place_ptr->proficiency);
        }
        else if (name.find("pd2") != std::string::npos)
        {
            skill_assembly.push_back(place_ptr->proficiency);
        }

        if (!itr1->value.HasMember("color")) {
            throw std::runtime_error(
                "Place " + place_ptr->place_name + " missing field: color"
            );
        }

        if (!itr1->value["color"].IsString()) {
            throw std::runtime_error(
                "Place " + place_ptr->place_name + " field color must be string"
            );
        }

        place_ptr->color = TokenAttribute::string_to_place_color(
            itr1->value["color"].GetString()
        );

        for (int i = 0; i < itr1->value["pre_arcs"].GetArray().Size(); i++) {
            place_ptr->pre_arcs.emplace_back(itr1->value["pre_arcs"][i].GetString());
        }
        for (int i = 0; i < itr1->value["post_arcs"].GetArray().Size(); i++) {
            place_ptr->post_arcs.emplace_back(itr1->value["post_arcs"][i].GetString());
        }
        petrinet.places.emplace(place_ptr->place_name, place_ptr);

        /*std::cout << " name " << place_ptr->place_name << " capacity " << place_ptr->capacity << " is_control_place " << place_ptr->is_control_place << " proficiency " << place_ptr->proficiency << std::endl;*/
    }
}
//读取json文件，初始化变迁信息
inline void read_trans_json(Petrinet& petrinet) {
    //流文件打开json文件
    std::ifstream ifs(Transition_path);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
    }
    // 读取JSON数据
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    //是否解析成功
    if (doc.HasParseError()) {
        std::cerr << "Failed to parse JSON" << std::endl;
    }
    // 遍历每个对象
    for (auto itr1 = doc.MemberBegin(); itr1 != doc.MemberEnd(); ++itr1) {
        auto trans_ptr = make_shared<Transition>();
        trans_ptr->trans_name = itr1->name.GetString();
        trans_ptr->place_check = itr1->value["place_check"].GetBool();
        trans_ptr->place_lock = itr1->value["place_lock"].GetBool();

        auto test = itr1->value.GetObject();
        auto test1 = test["pre_arcs"].GetArray().begin();
        if (itr1->value.HasMember("pre_arcs") && itr1->value["pre_arcs"].IsArray()) {
            // 遍历 pre_arcs 数组中的每个元素
            vector<string> arcs_temp;
            for (rapidjson::SizeType i = 0; i < itr1->value["pre_arcs"].Size(); i++) {
                arcs_temp.emplace_back(itr1->value["pre_arcs"][i].GetString());  // 获取 pre_arcs 数组中的字符串并加入到 arcs_temp
            }
            // 将 pre_arcs 对应的键添加到 pre_places
            for (int i = 0; i < arcs_temp.size(); ++i) {
                trans_ptr->pre_places.emplace_back(arcs_temp[i]);
            }

            // 可以对 arcs_temp 进行进一步的处理
        }

        // 读取 post_arcs 字段
        if (itr1->value.HasMember("post_arcs") && itr1->value["post_arcs"].IsArray()) {
            // 遍历 post_arcs 数组中的每个元素
            vector<string> arcs_temp;
            for (rapidjson::SizeType i = 0; i < itr1->value["post_arcs"].Size(); i++) {
                arcs_temp.emplace_back(itr1->value["post_arcs"][i].GetString());  // 获取 post_arcs 数组中的字符串并加入到 arcs_temp
            }
            // 将 post_arcs 对应的键添加到 post_places
            for (int i = 0; i < arcs_temp.size(); ++i) {
                trans_ptr->post_places.emplace_back(arcs_temp[i]);
            }

            // 可以对 arcs_temp 进行进一步的处理
        }
        petrinet.transitions.emplace(trans_ptr->trans_name, trans_ptr);
        //std::cout << " name " << trans_ptr->trans_name << " trans_ptr->place_check " << trans_ptr->place_check << std::endl;
    }
}

//读取json文件，初始化变迁信息
inline void read_tokens_json(Petrinet& petrinet) {
    //流文件打开json文件
    std::ifstream ifs(Token_path);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
    }
    // 读取JSON数据
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    //是否解析成功
    if (doc.HasParseError()) {
        std::cerr << "Failed to parse JSON" << std::endl;
    }

    door_pre_time.clear();
    door_work_time.clear();
    grid_pre_time.clear();
    grid_work_time.clear();
    assembly_work_time.clear();
    series.clear();

    //auto test = doc["original_state"].GetObject().begin()->value["inplace"].GetString();
    for (auto itr1 = doc["original_state"].GetObject().begin(); itr1 != doc["original_state"].GetObject().end(); ++itr1) {
        
        auto workpiece_ptr = make_shared<Workpiece>();
        workpiece_ptr->ID = itr1->name.GetString();
        workpiece_ptr->numid = itr1->value["ID"].GetInt();
        workpiece_ptr->order_id = itr1->value["orderID"].GetString();//新增订单号
        workpiece_ptr->serial_id = itr1->value["serialID"].GetString();//新增序列号
        rapidjson::Value::Array arr = itr1->value["per_worktime"].GetArray();
        for (const auto& elem : arr) {
            int val = elem.IsInt() ? elem.GetInt() : 0;
            workpiece_ptr->per_worktime.push_back(val);
        }

        door_pre_time.push_back(workpiece_ptr->per_worktime[1]);
        door_work_time.push_back(workpiece_ptr->per_worktime[2]);
        grid_pre_time.push_back(workpiece_ptr->per_worktime[3]);
        grid_work_time.push_back(workpiece_ptr->per_worktime[4]);
        assembly_work_time.push_back(workpiece_ptr->per_worktime[8]);
        series.push_back(workpiece_ptr->serial_id);


        petrinet.workpiece.emplace(workpiece_ptr->ID, workpiece_ptr);
        petrinet.workpiece_id.push_back(workpiece_ptr);


    }
    N = assembly_work_time.size();
    //for (auto& [name, ptr] : petrinet.workpiece_id) {
    //    cout << "工件名" << name << "指针" << ptr << endl;
    //}
    //if (petrinet.workpiece.empty())cout << "表为空" << endl;

}


