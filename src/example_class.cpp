#include "example_class.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

// === 核心弹性控制函数实现 ===

void ExampleClass::setElasticity(float stiffness, float damping, int iterations, 
                                float constraint_strength, float stretch_resistance,
                                float compression_resistance) {
    // 设置刚度
    elasticity.stiffness = Math::clamp(stiffness, 0.1f, 3.0f);
    
    // 设置阻尼
    elasticity.damping = Math::clamp(damping, 0.7f, 1.0f);
    
    // 可选参数设置
    if (iterations > 0) {
        elasticity.constraint_iterations = Math::clamp(iterations, 1, 20);
    }
    
    if (constraint_strength >= 0.0f) {
        elasticity.constraint_strength = Math::clamp(constraint_strength, 0.1f, 1.0f);
    }
    
    if (stretch_resistance >= 0.0f) {
        elasticity.stretch_resistance = Math::clamp(stretch_resistance, 0.5f, 2.0f);
    }
    
    if (compression_resistance >= 0.0f) {
        elasticity.compression_resistance = Math::clamp(compression_resistance, 0.5f, 2.0f);
    }
    
    UtilityFunctions::print("弹性参数已更新 - 刚度:", elasticity.stiffness, 
                           " 阻尼:", elasticity.damping, 
                           " 迭代:", elasticity.constraint_iterations);
}

void ExampleClass::setElasticityPreset(const String& preset_name) {
    if (preset_name == "rigid") {
        elasticity.setRigid();
        UtilityFunctions::print("已设置为刚性模式");
    } else if (preset_name == "elastic") {
        elasticity.setElastic();
        UtilityFunctions::print("已设置为弹性模式");
    } else if (preset_name == "soft") {
        elasticity.setSoft();
        UtilityFunctions::print("已设置为柔软模式");
    } else if (preset_name == "bouncy") {
        elasticity.setBouncy();
        UtilityFunctions::print("已设置为弹跳模式");
    } else {
        UtilityFunctions::print("未知的弹性预设:", preset_name);
    }
}

void ExampleClass::setAdvancedElasticity(const Dictionary& params) {
    if (params.has("stiffness")) {
        elasticity.stiffness = Math::clamp(float(params["stiffness"]), 0.1f, 3.0f);
    }
    if (params.has("damping")) {
        elasticity.damping = Math::clamp(float(params["damping"]), 0.7f, 1.0f);
    }
    if (params.has("iterations")) {
        elasticity.constraint_iterations = Math::clamp(int(params["iterations"]), 1, 20);
    }
    if (params.has("constraint_strength")) {
        elasticity.constraint_strength = Math::clamp(float(params["constraint_strength"]), 0.1f, 1.0f);
    }
    if (params.has("stretch_resistance")) {
        elasticity.stretch_resistance = Math::clamp(float(params["stretch_resistance"]), 0.5f, 2.0f);
    }
    if (params.has("compression_resistance")) {
        elasticity.compression_resistance = Math::clamp(float(params["compression_resistance"]), 0.5f, 2.0f);
    }
    
    UtilityFunctions::print("高级弹性参数已更新");
}

Dictionary ExampleClass::getElasticityParams() const {
    Dictionary params;
    params["stiffness"] = elasticity.stiffness;
    params["damping"] = elasticity.damping;
    params["iterations"] = elasticity.constraint_iterations;
    params["constraint_strength"] = elasticity.constraint_strength;
    params["stretch_resistance"] = elasticity.stretch_resistance;
    params["compression_resistance"] = elasticity.compression_resistance;
    return params;
}

// 替换现有的中文输出
void ExampleClass::adjustStiffness(float delta) {
    elasticity.stiffness = Math::clamp(elasticity.stiffness + delta, 0.1f, 3.0f);
    // 使用u8字符串字面量确保UTF-8编码
    UtilityFunctions::print(u8"刚度调整为:", elasticity.stiffness);
}

void ExampleClass::adjustDamping(float delta) {
    elasticity.damping = Math::clamp(elasticity.damping + delta, 0.7f, 1.0f);
    UtilityFunctions::print(u8"阻尼调整为:", elasticity.damping);
}

void ExampleClass::adjustConstraintStrength(float delta) {
    elasticity.constraint_strength = Math::clamp(elasticity.constraint_strength + delta, 0.1f, 1.0f);
    UtilityFunctions::print(u8"约束强度调整为:", elasticity.constraint_strength);
}

// === 优化的物理更新函数 ===

void ExampleClass::update_physics(float deltaTime) {
    if (!isInitialized) {
        isInitialized = true;
        return;
    }

    // 韦尔莱积分更新位置（应用阻尼）
    for (auto& node : nodes) {
        if (node.locked) continue;
        
        Vector2 velocity = node.position - node.oldPosition;
        // 应用阻尼
        velocity *= elasticity.damping;
        
        node.oldPosition = node.position;
        node.position += velocity + physics_gravity * deltaTime * deltaTime;
    }

    // 应用约束
    applyConstraints(deltaTime);
}

void ExampleClass::applyConstraints(float deltaTime) {
    // 可调节的约束迭代
    for (int iter = 0; iter < elasticity.constraint_iterations; ++iter) {
        for (int i = 0; i < static_cast<int>(nodes.size()) - 1; ++i) {
            RopeNode& nodeA = nodes[i];
            RopeNode& nodeB = nodes[i + 1];
            
            Vector2 delta = nodeB.position - nodeA.position;
            float currentLength = delta.length();
            if (currentLength < 1e-6f) continue;
            
            float error = currentLength - segmentLength;
            Vector2 correction = delta.normalized() * error;
            
            // 根据拉伸/压缩应用不同的阻力
            float resistance = (error > 0) ? elasticity.stretch_resistance : elasticity.compression_resistance;
            
            // 应用刚度、约束强度和阻力
            correction *= elasticity.stiffness * elasticity.constraint_strength * resistance;
            
            // 根据迭代次数调整修正强度（后期迭代更精细）
            float iteration_factor = 1.0f - (float(iter) / float(elasticity.constraint_iterations)) * 0.3f;
            correction *= iteration_factor;

            if (!nodeA.locked && !nodeB.locked) {
                float totalMass = nodeA.mass + nodeB.mass;
                float ratioA = nodeB.mass / totalMass;
                float ratioB = nodeA.mass / totalMass;
                
                nodeA.position += correction * (0.5f * ratioA);
                nodeB.position -= correction * (0.5f * ratioB);
            } else if (!nodeA.locked) {
                nodeA.position += correction * 0.5f;
            } else if (!nodeB.locked) {
                nodeB.position -= correction * 0.5f;
            }
        }
    }
}

// === Godot 方法绑定 ===

void ExampleClass::_bind_methods() {
    // 核心弹性控制函数
    ClassDB::bind_method(D_METHOD("setElasticity", "stiffness", "damping", "iterations", "constraint_strength", "stretch_resistance", "compression_resistance"), 
                        &ExampleClass::setElasticity, DEFVAL(-1), DEFVAL(-1.0f), DEFVAL(-1.0f), DEFVAL(-1.0f));
    
    ClassDB::bind_method(D_METHOD("setElasticityPreset", "preset_name"), &ExampleClass::setElasticityPreset);
    ClassDB::bind_method(D_METHOD("setAdvancedElasticity", "params"), &ExampleClass::setAdvancedElasticity);
    ClassDB::bind_method(D_METHOD("getElasticityParams"), &ExampleClass::getElasticityParams);
    
    // 实时调整函数
    ClassDB::bind_method(D_METHOD("adjustStiffness", "delta"), &ExampleClass::adjustStiffness);
    ClassDB::bind_method(D_METHOD("adjustDamping", "delta"), &ExampleClass::adjustDamping);
    ClassDB::bind_method(D_METHOD("adjustConstraintStrength", "delta"), &ExampleClass::adjustConstraintStrength);
    
    // 弹性参数访问器
    ClassDB::bind_method(D_METHOD("set_stiffness", "value"), &ExampleClass::set_stiffness);
    ClassDB::bind_method(D_METHOD("get_stiffness"), &ExampleClass::get_stiffness);
    ClassDB::bind_method(D_METHOD("set_damping", "value"), &ExampleClass::set_damping);
    ClassDB::bind_method(D_METHOD("get_damping"), &ExampleClass::get_damping);
    ClassDB::bind_method(D_METHOD("set_constraint_iterations", "value"), &ExampleClass::set_constraint_iterations);
    ClassDB::bind_method(D_METHOD("get_constraint_iterations"), &ExampleClass::get_constraint_iterations);
    ClassDB::bind_method(D_METHOD("set_constraint_strength", "value"), &ExampleClass::set_constraint_strength);
    ClassDB::bind_method(D_METHOD("get_constraint_strength"), &ExampleClass::get_constraint_strength);
    
    // 现有方法绑定...
    ClassDB::bind_method(D_METHOD("update_physics", "delta"), &ExampleClass::update_physics);
    ClassDB::bind_method(D_METHOD("get_node_positions"), &ExampleClass::get_node_positions);
    ClassDB::bind_method(D_METHOD("set_node_count", "count"), &ExampleClass::set_node_count);
    ClassDB::bind_method(D_METHOD("get_node_count"), &ExampleClass::get_node_count);
    ClassDB::bind_method(D_METHOD("set_rope_length", "length"), &ExampleClass::set_rope_length);
    ClassDB::bind_method(D_METHOD("get_rope_length"), &ExampleClass::get_rope_length);
    ClassDB::bind_method(D_METHOD("set_gravity", "gravity"), &ExampleClass::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"), &ExampleClass::get_gravity);
    ClassDB::bind_method(D_METHOD("set_debug_draw", "enabled"), &ExampleClass::set_debug_draw);
    ClassDB::bind_method(D_METHOD("is_debug_drawing"), &ExampleClass::is_debug_drawing);
    ClassDB::bind_method(D_METHOD("apply_force", "node_index", "force"), &ExampleClass::apply_force);
    ClassDB::bind_method(D_METHOD("reset_rope"), &ExampleClass::reset_rope);
    ClassDB::bind_method(D_METHOD("setNodeLocked", "index", "locked"), &ExampleClass::setNodeLocked);
    ClassDB::bind_method(D_METHOD("setNodeMass", "index", "mass"), &ExampleClass::setNodeMass);
    ClassDB::bind_method(D_METHOD("remove_node", "index"), &ExampleClass::remove_node);
    ClassDB::bind_method(D_METHOD("cut_rope_at", "index"), &ExampleClass::cut_rope_at);
    ClassDB::bind_method(D_METHOD("get_current_node_count"), &ExampleClass::get_current_node_count);
    
    // 注册属性
    ADD_PROPERTY(PropertyInfo(Variant::INT, "node_count", PROPERTY_HINT_RANGE, "2,100"), "set_node_count", "get_node_count");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rope_length", PROPERTY_HINT_RANGE, "0.1,50.0"), "set_rope_length", "get_rope_length");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "gravity"), "set_gravity", "get_gravity");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_draw"), "set_debug_draw", "is_debug_drawing");
    
    // 节点位移功能绑定
    ClassDB::bind_method(D_METHOD("set_node_position", "index", "position"), &ExampleClass::set_node_position);
    ClassDB::bind_method(D_METHOD("move_node", "index", "displacement"), &ExampleClass::move_node);
    ClassDB::bind_method(D_METHOD("get_node_position", "index"), &ExampleClass::get_node_position);
    ClassDB::bind_method(D_METHOD("translate_all_nodes", "displacement"), &ExampleClass::translate_all_nodes);
    ClassDB::bind_method(D_METHOD("translate_nodes_range", "start_index", "end_index", "displacement"), &ExampleClass::translate_nodes_range);
}

void ExampleClass::update_simulator() {
    physics_gravity = gravity;
    initializeRope(node_count, rope_length, Vector2(0, 0));
    freezeInitialState();
}

Array ExampleClass::get_node_positions() const {
    Array positions;
    for (const auto& node : nodes) {
        positions.push_back(node.position);
    }
    return positions;
}

// Godot生命周期方法
void ExampleClass::_ready() {
    update_simulator();
}

void ExampleClass::_process(double delta) {
    update_physics(static_cast<float>(delta));
}

void ExampleClass::_draw() {
    if (!debug_draw) return;
    
    for (size_t i = 0; i < nodes.size() - 1; ++i) {
        UtilityFunctions::print("Drawing rope segment from ", nodes[i].position, " to ", nodes[i+1].position);
    }
}

// 属性访问器方法
void ExampleClass::set_node_count(int count) {
    node_count = Math::clamp(count, 2, 100);
    update_simulator();
}

void ExampleClass::set_rope_length(float length) {
    rope_length = Math::clamp(length, 0.1f, 50.0f);
    update_simulator();
}

void ExampleClass::set_gravity(Vector2 g) {
    gravity = g;
    update_simulator();
}

void ExampleClass::apply_force(int node_index, Vector2 force) {
    applyForce(node_index, force, 0.016f); // 假设60FPS
}

void ExampleClass::reset_rope() {
    update_simulator();
}

// 改进的 remove_node 函数，避免过度弹性
void ExampleClass::remove_node(int index) {
    if (index < 0 || index >= static_cast<int>(nodes.size())) {
        UtilityFunctions::print("Error: Node index out of range");
        return;
    }
    
    if (index == 0) {
        UtilityFunctions::print("Error: Cannot remove start node");
        return;
    }
    
    if (nodes.size() <= 2) {
        UtilityFunctions::print("Error: Too few nodes to remove");
        return;
    }
    
    // 保存所有节点的状态（除了要删除的节点）
    std::vector<bool> locked_states;
    std::vector<float> mass_values;
    std::vector<Vector2> positions;
    std::vector<Vector2> old_positions;
    
    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        if (i != index) {
            locked_states.push_back(nodes[i].locked);
            mass_values.push_back(nodes[i].mass);
            positions.push_back(nodes[i].position);
            old_positions.push_back(nodes[i].oldPosition);
        }
    }
    
    // 移除节点
    nodes.erase(nodes.begin() + index);
    
    // 重新计算段长度，基于实际剩余节点的距离
    float total_length = 0.0f;
    for (int i = 0; i < static_cast<int>(positions.size()) - 1; ++i) {
        total_length += (positions[i + 1] - positions[i]).length();
    }
    segmentLength = total_length / (positions.size() - 1);
    
    // 恢复节点状态，保持原有位置和速度
    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        nodes[i].position = positions[i];
        nodes[i].oldPosition = old_positions[i];
        nodes[i].locked = locked_states[i];
        nodes[i].mass = mass_values[i];
        
        // 添加轻微的阻尼，减少弹性反应
        Vector2 velocity = nodes[i].position - nodes[i].oldPosition;
        nodes[i].oldPosition = nodes[i].position - velocity * 0.8f; // 20%的阻尼
    }
    
    UtilityFunctions::print("Removed node ", index, ", current nodes: ", nodes.size(), ", segment length: ", segmentLength);
}

// 修改 cut_rope_at 函数，保留锁定状态
void ExampleClass::cut_rope_at(int index) {
    if (index < 1 || index >= static_cast<int>(nodes.size())) {
        UtilityFunctions::print("Error: Invalid cut position index");
        return;
    }
    
    // 保存要保留的节点状态
    std::vector<bool> locked_states;
    std::vector<float> mass_values;
    for (int i = 0; i <= index; ++i) {
        locked_states.push_back(nodes[i].locked);
        mass_values.push_back(nodes[i].mass);
    }
    
    // 计算新的绳子长度（从起始点到切断点的实际距离）
    float new_length = 0.0f;
    for (int i = 0; i < index; ++i) {
        new_length += (nodes[i + 1].position - nodes[i].position).length();
    }
    
    // 移除切断点之后的所有节点
    nodes.erase(nodes.begin() + index + 1, nodes.end());
    
    // 更新绳子参数
    rope_length = new_length;
    node_count = nodes.size();
    segmentLength = rope_length / (nodes.size() - 1);
    
    // 恢复保留节点的状态
    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        nodes[i].locked = locked_states[i];
        nodes[i].mass = mass_values[i];
    }
    
    UtilityFunctions::print("Cut rope at node ", index, ", new length: ", rope_length, ", new node count: ", nodes.size());
}

// 获取当前实际节点数
int ExampleClass::get_current_node_count() const {
    return static_cast<int>(nodes.size());
}

// 新增：节点位移功能实现
void ExampleClass::set_node_position(int index, Vector2 position) {
    if (index < 0 || index >= static_cast<int>(nodes.size())) {
        UtilityFunctions::print("Error: Node index out of range");
        return;
    }
    
    nodes[index].position = position;
    nodes[index].oldPosition = position; // 重置速度
    
    UtilityFunctions::print("Set node ", index, " position to: ", position);
}

void ExampleClass::move_node(int index, Vector2 displacement) {
    if (index < 0 || index >= static_cast<int>(nodes.size())) {
        UtilityFunctions::print("Error: Node index out of range");
        return;
    }
    
    nodes[index].position += displacement;
    nodes[index].oldPosition += displacement; // 保持速度不变
    
    UtilityFunctions::print("Moved node ", index, " by: ", displacement, ", new position: ", nodes[index].position);
}

Vector2 ExampleClass::get_node_position(int index) const {
    if (index < 0 || index >= static_cast<int>(nodes.size())) {
        UtilityFunctions::print("Error: Node index out of range");
        return Vector2(0, 0);
    }
    
    return nodes[index].position;
}

void ExampleClass::translate_all_nodes(Vector2 displacement) {
    for (auto& node : nodes) {
        node.position += displacement;
        node.oldPosition += displacement; // 保持速度不变
    }
    
    UtilityFunctions::print("Translated all nodes by: ", displacement);
}

void ExampleClass::translate_nodes_range(int start_index, int end_index, Vector2 displacement) {
    if (start_index < 0 || end_index >= static_cast<int>(nodes.size()) || start_index > end_index) {
        UtilityFunctions::print("Error: Invalid node range");
        return;
    }
    
    for (int i = start_index; i <= end_index; ++i) {
        nodes[i].position += displacement;
        nodes[i].oldPosition += displacement; // 保持速度不变
    }
    
    UtilityFunctions::print("Translated nodes ", start_index, " to ", end_index, " by: ", displacement);
}

bool ExampleClass::get_node_locked(int index) const {
    if (index >= 0 && index < static_cast<int>(nodes.size())) {
        return nodes[index].locked;
    }
    return false;
}

// 添加速度控制方法
void ExampleClass::add_velocity_damping(int index, float damping_factor) {
    if (index < 0 || index >= static_cast<int>(nodes.size())) {
        return;
    }
    
    Vector2 velocity = nodes[index].position - nodes[index].oldPosition;
    nodes[index].oldPosition = nodes[index].position - velocity * (1.0f - damping_factor);
}

void ExampleClass::set_node_velocity(int index, Vector2 velocity) {
    if (index < 0 || index >= static_cast<int>(nodes.size())) {
        return;
    }
    
    nodes[index].oldPosition = nodes[index].position - velocity;
}

Vector2 ExampleClass::get_node_velocity(int index) const {
    if (index < 0 || index >= static_cast<int>(nodes.size())) {
        return Vector2(0, 0);
    }
    
    return nodes[index].position - nodes[index].oldPosition;
}
// 在 #include 语句之后添加

ExampleClass::ExampleClass() {
    // 初始化默认值
    isInitialized = false;
    debug_draw = true;
    rope_color = Color(0.8, 0.2, 0.2);
    node_count = 10;
    rope_length = 5.0;
    gravity = Vector2(0, 9.8);
    physics_gravity = gravity;
    segmentLength = 0.1f;
}

ExampleClass::~ExampleClass() {
    // 清理资源
    nodes.clear();
}

// 添加调试相关函数
void ExampleClass::set_debug_draw(bool enabled) {
    debug_draw = enabled;
}

bool ExampleClass::is_debug_drawing() const {
    return debug_draw;
}

// 添加基础物理函数
void ExampleClass::freezeInitialState() {
    // 冻结初始状态，用于稳定绳索
    for (auto& node : nodes) {
        node.oldPosition = node.position;
    }
}

void ExampleClass::applyForce(int node_index, Vector2 force, float deltaTime) {
    if (node_index < 0 || node_index >= static_cast<int>(nodes.size())) {
        return;
    }
    
    if (nodes[node_index].locked) {
        return;
    }
    
    // 应用力到节点
    Vector2 acceleration = force / nodes[node_index].mass;
    nodes[node_index].position += acceleration * deltaTime * deltaTime;
}

// 添加绳索初始化函数
void ExampleClass::initializeRope(int numNodes, float totalLength, Vector2 startPos) {
    nodes.clear();
    nodes.reserve(numNodes);
    
    segmentLength = totalLength / (numNodes - 1);
    
    // 创建节点
    for (int i = 0; i < numNodes; ++i) {
        Vector2 position = startPos + Vector2(0, i * segmentLength);
        nodes.emplace_back(position, 1.0f, i == 0); // 第一个节点默认锁定
    }
    
    isInitialized = true;
    UtilityFunctions::print("绳索初始化完成，节点数：", numNodes, "，总长度：", totalLength);
}

// 添加缺失的函数实现
void ExampleClass::setNodeMass(int index, float mass) {
    if (index < 0 || index >= static_cast<int>(nodes.size())) {
        UtilityFunctions::print("Error: Node index out of range");
        return;
    }
    
    nodes[index].mass = Math::max(mass, 0.1f); // 确保质量不为零或负数
    UtilityFunctions::print("Set node ", index, " mass to: ", nodes[index].mass);
}

void ExampleClass::setNodeLocked(int index, bool locked) {
    if (index < 0 || index >= static_cast<int>(nodes.size())) {
        UtilityFunctions::print("Error: Node index out of range");
        return;
    }
    
    nodes[index].locked = locked;
    UtilityFunctions::print("Set node ", index, " locked state to: ", locked);
}