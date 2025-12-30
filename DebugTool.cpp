#include "DebugTool.h"
// 引入你的Place和Transition类头文件（替换为实际路径）
#include "Place.h"
#include "Transition.h"


// -------------- 临时定义结束 --------------

void PetriNetDebugTool::printStringVec(const std::vector<std::string>& vec, const std::string& prefix) {
    std::cout << prefix;
    if (vec.empty()) {
        std::cout << "空" << std::endl;
        return;
    }
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << vec[i];
    }
    std::cout << std::endl;
}

void PetriNetDebugTool::printAllPlaces(const std::unordered_map<std::string, std::shared_ptr<Place>>& places) {
    std::cout << "==================================== 所有库所（Place）信息 ====================================" << std::endl;
    if (places.empty()) {
        std::cout << "??  无任何库所数据" << std::endl;
        return;
    }

    for (const auto& pair : places) {
        const std::string& place_id = pair.first;
        const std::shared_ptr<Place>& place = pair.second;

        std::cout << "\n【库所 ID】: " << place_id << std::endl;
        std::cout << "  库所名称: " << place->place_name << std::endl;
        std::cout << "  容量: " << place->capacity << std::endl;
        std::cout << "  是否赋时库所: " << (place->is_time_place ? "是" : "否") << std::endl;
        std::cout << "  人员熟练度: " << place->proficiency << std::endl;
        std::cout << "  对应工序: " << place->stage << std::endl;
        std::cout << "  前置变迁（pre_arcs）: ";
        printStringVec(place->pre_arcs, "    ");
        std::cout << "  后置变迁（post_arcs）: ";
        printStringVec(place->post_arcs, "    ");
        std::cout << "----------------------------------------" << std::endl;
    }
}

void PetriNetDebugTool::printAllTransitions(const std::unordered_map<std::string, std::shared_ptr<Transition>>& transitions) {
    std::cout << "\n==================================== 所有变迁（Transition）信息 ====================================" << std::endl;
    if (transitions.empty()) {
        std::cout << "??  无任何变迁数据" << std::endl;
        return;
    }

    for (const auto& pair : transitions) {
        const std::string& trans_id = pair.first;
        const std::shared_ptr<Transition>& trans = pair.second;

        std::cout << "\n【变迁 ID】: " << trans_id << std::endl;
        std::cout << "  变迁名称: " << trans->trans_name << std::endl;
        std::cout << "  前置库所（pre_places）: ";
        printStringVec(trans->pre_places, "    ");
        std::cout << "  后置库所（post_places）: ";
        printStringVec(trans->post_places, "    ");
        std::cout << "  库位锁状态: " << (trans->place_lock ? "上锁" : "未上锁") << std::endl;
        std::cout << "  库所检查状态: " << (trans->place_check ? "需检查" : "无检查") << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }
}

bool PetriNetDebugTool::verifyConnectionConsistency(
    const std::unordered_map<std::string, std::shared_ptr<Place>>& places,
    const std::unordered_map<std::string, std::shared_ptr<Transition>>& transitions
) {
    std::cout << "\n==================================== 连接关系一致性验证 ====================================" << std::endl;
    bool has_error = false;

    // 验证：变迁的前置库所 → 库所的后置变迁 必须包含该变迁
    for (const auto& trans_pair : transitions) {
        const std::string& trans_id = trans_pair.first;
        const std::shared_ptr<Transition>& trans = trans_pair.second;

        // 检查前置库所关联
        for (const std::string& pre_place_id : trans->pre_places) {
            auto place_it = places.find(pre_place_id);
            if (place_it == places.end()) {
                std::cout << "? 错误：变迁[" << trans_id << "]的前置库所[" << pre_place_id << "]不存在！" << std::endl;
                has_error = true;
                continue;
            }
            const auto& post_arcs = place_it->second->post_arcs;
            if (std::find(post_arcs.begin(), post_arcs.end(), trans_id) == post_arcs.end()) {
                std::cout << "? 错误：变迁[" << trans_id << "]的前置库所[" << pre_place_id << "]，其后置变迁未包含该变迁！" << std::endl;
                has_error = true;
            }
        }

        // 检查后置库所关联
        for (const std::string& post_place_id : trans->post_places) {
            auto place_it = places.find(post_place_id);
            if (place_it == places.end()) {
                std::cout << "? 错误：变迁[" << trans_id << "]的后置库所[" << post_place_id << "]不存在！" << std::endl;
                has_error = true;
                continue;
            }
            const auto& pre_arcs = place_it->second->pre_arcs;
            if (std::find(pre_arcs.begin(), pre_arcs.end(), trans_id) == pre_arcs.end()) {
                std::cout << "? 错误：变迁[" << trans_id << "]的后置库所[" << post_place_id << "]，其前置变迁未包含该变迁！" << std::endl;
                has_error = true;
            }
        }
    }

    if (!has_error) {
        std::cout << "? 所有连接关系一致，无错误！" << std::endl;
    }
    return !has_error;
}

void PetriNetDebugTool::printPetriNetInfo(
    const std::unordered_map<std::string, std::shared_ptr<Place>>& places,
    const std::unordered_map<std::string, std::shared_ptr<Transition>>& transitions
) {
    printAllPlaces(places);
    printAllTransitions(transitions);
    verifyConnectionConsistency(places, transitions);
}