#ifndef DEBUG_TOOL_H
#define DEBUG_TOOL_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

// 前置声明（如果你的Place/Transition类在其他头文件，替换为对应的#include）
class Place;
class Transition;

/**
 * @brief Petri网调试工具类
 * 用于打印库所、变迁信息，验证连接关系一致性
 */
class PetriNetDebugTool {
public:
    /**
     * @brief 打印所有库所的详细信息
     * @param places 库所容器（key：库所ID，value：库所对象指针）
     */
    static void printAllPlaces(const std::unordered_map<std::string, std::shared_ptr<Place>>& places);

    /**
     * @brief 打印所有变迁的详细信息
     * @param transitions 变迁容器（key：变迁ID，value：变迁对象指针）
     */
    static void printAllTransitions(const std::unordered_map<std::string, std::shared_ptr<Transition>>& transitions);

    /**
     * @brief 验证库所-变迁连接关系的一致性
     * @param places 库所容器
     * @param transitions 变迁容器
     * @return bool 一致性结果（true：无错误，false：存在错误）
     */
    static bool verifyConnectionConsistency(
        const std::unordered_map<std::string, std::shared_ptr<Place>>& places,
        const std::unordered_map<std::string, std::shared_ptr<Transition>>& transitions
    );

    /**
     * @brief 一键打印所有Petri网信息并验证一致性（推荐主入口）
     * @param places 库所容器
     * @param transitions 变迁容器
     */
    static void printPetriNetInfo(
        const std::unordered_map<std::string, std::shared_ptr<Place>>& places,
        const std::unordered_map<std::string, std::shared_ptr<Transition>>& transitions
    );

private:
    /**
     * @brief 辅助函数：打印字符串向量
     * @param vec 待打印的向量
     * @param prefix 前缀（用于格式化）
     */
    static void printStringVec(const std::vector<std::string>& vec, const std::string& prefix = "");
};

#endif // DEBUG_TOOL_H
