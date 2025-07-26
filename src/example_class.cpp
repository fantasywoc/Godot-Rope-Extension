#include "example_class.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;


void ExampleClass::setElasticity(float stiffness, float damping, int iterations, 
                                float constraint_strength, float stretch_resistance,
                                float compression_resistance) {
    // 刚度
    elasticity.stiffness = Math::clamp(stiffness, 0.1f, 3.0f);
    
    // 阻尼
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
    //godot 输出函数
    UtilityFunctions::print("Elasticity parameters updated - Stiffness:", elasticity.stiffness, 
                           " Damping:", elasticity.damping, 
                           " Iterations:", elasticity.constraint_iterations);
}
// 预设属性
void ExampleClass::setElasticityPreset(const String& preset_name) {
    if (preset_name == "rigid") {
        elasticity.setRigid();
        UtilityFunctions::print("Set to rigid mode");
    } else if (preset_name == "elastic") {
        elasticity.setElastic();
        UtilityFunctions::print("Set to elastic mode");
    } else if (preset_name == "soft") {
        elasticity.setSoft();
        UtilityFunctions::print("Set to soft mode");
    } else if (preset_name == "bouncy") {
        elasticity.setBouncy();
        UtilityFunctions::print("Set to bouncy mode");
    } else {
        UtilityFunctions::print("Unknown elasticity preset:", preset_name);
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
    
    UtilityFunctions::print("Advanced elasticity parameters updated");
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


void ExampleClass::adjustStiffness(float delta) {
    elasticity.stiffness = Math::clamp(elasticity.stiffness + delta, 0.1f, 3.0f);
    UtilityFunctions::print("Stiffness adjusted to:", elasticity.stiffness);
}

void ExampleClass::adjustDamping(float delta) {
    elasticity.damping = Math::clamp(elasticity.damping + delta, 0.7f, 1.0f);
    UtilityFunctions::print("Damping adjusted to:", elasticity.damping);
}

void ExampleClass::adjustConstraintStrength(float delta) {
    elasticity.constraint_strength = Math::clamp(elasticity.constraint_strength + delta, 0.1f, 1.0f);
    UtilityFunctions::print("Constraint strength adjusted to:", elasticity.constraint_strength);
}

// 更新函数 

void ExampleClass::update_physics(float deltaTime) {
    if (!isInitialized) {
        isInitialized = true;
        return;
    }

    // // 重力影响的时间步长
    // gravity_multiplier = 10.0f;  // 增加这个值可以增强重力对绳子的影响
    
        // 韦尔莱积分更新位置
    for (auto& node : nodes) {
        if (node.locked) continue;
        
        // 计算当前速度：v(t) = (x(t) - x(t-Δt))/Δt
        // 这里没有除以deltaTime，因为后续位置更新会乘以deltaTime
        Vector2 velocity = node.position - node.oldPosition;
        
        // 阻尼效果其实是就是一个减速的效果，类似风阻
        velocity *= elasticity.damping;
        
        // 更新位置
        node.oldPosition = node.position;
        
        // 维尔莱积分位置更新：x(t+Δt) = x(t) + v(t)*Δt + a(t)*(Δt)²
        // 其中：
        // - velocity 等价于 v(t)*Δt
        // - physics_gravity * deltaTime * deltaTime * gravity_multiplier 等价于 a(t)*(Δt)²
        // - gravity_multiplier 是一个系数，用于调整重力的影响强度
        node.position += velocity + (physics_gravity * gravity_multiplier ) * deltaTime * deltaTime;
    }

    applyConstraints(deltaTime);
}

void ExampleClass::applyConstraints(float deltaTime) {
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
            
            // 根据迭代次数调整修正强度
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

/////////////////////////// Godot 绑定 C++扩展类///////////////////////////
void ExampleClass::_bind_methods() {
  
    ClassDB::bind_method(D_METHOD("setElasticity", "stiffness", "damping", "iterations", "constraint_strength", "stretch_resistance", "compression_resistance"), 
                        &ExampleClass::setElasticity, DEFVAL(-1), DEFVAL(-1.0f), DEFVAL(-1.0f), DEFVAL(-1.0f));
    
    ClassDB::bind_method(D_METHOD("setElasticityPreset", "preset_name"), &ExampleClass::setElasticityPreset);
    ClassDB::bind_method(D_METHOD("setAdvancedElasticity", "params"), &ExampleClass::setAdvancedElasticity);
    ClassDB::bind_method(D_METHOD("getElasticityParams"), &ExampleClass::getElasticityParams);
    
    ClassDB::bind_method(D_METHOD("adjustStiffness", "delta"), &ExampleClass::adjustStiffness);
    ClassDB::bind_method(D_METHOD("adjustDamping", "delta"), &ExampleClass::adjustDamping);
    ClassDB::bind_method(D_METHOD("adjustConstraintStrength", "delta"), &ExampleClass::adjustConstraintStrength);
    
    ClassDB::bind_method(D_METHOD("set_stiffness", "value"), &ExampleClass::set_stiffness);
    ClassDB::bind_method(D_METHOD("get_stiffness"), &ExampleClass::get_stiffness);
    ClassDB::bind_method(D_METHOD("set_damping", "value"), &ExampleClass::set_damping);
    ClassDB::bind_method(D_METHOD("get_damping"), &ExampleClass::get_damping);
    ClassDB::bind_method(D_METHOD("set_constraint_iterations", "value"), &ExampleClass::set_constraint_iterations);
    ClassDB::bind_method(D_METHOD("get_constraint_iterations"), &ExampleClass::get_constraint_iterations);
    ClassDB::bind_method(D_METHOD("set_constraint_strength", "value"), &ExampleClass::set_constraint_strength);
    ClassDB::bind_method(D_METHOD("get_constraint_strength"), &ExampleClass::get_constraint_strength);

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
    
    //增加绳子长度相关函数
    ClassDB::bind_method(D_METHOD("add_node_at", "index", "position"), &ExampleClass::add_node_at, DEFVAL(Vector2(0, 0)));
    ClassDB::bind_method(D_METHOD("add_node_to_end"), &ExampleClass::add_node_to_end);
    ClassDB::bind_method(D_METHOD("add_nodes_to_end", "count"), &ExampleClass::add_nodes_to_end);
    ClassDB::bind_method(D_METHOD("insert_node_between", "index1", "index2"), &ExampleClass::insert_node_between);
    
    
    ClassDB::bind_method(D_METHOD("extend_rope_by_length", "additional_length"), &ExampleClass::extend_rope_by_length);
    ClassDB::bind_method(D_METHOD("extend_rope_by_nodes", "additional_nodes"), &ExampleClass::extend_rope_by_nodes);
    
    //质量设置
    ClassDB::bind_method(D_METHOD("set_default_node_mass", "mass"), &ExampleClass::set_default_node_mass);
    ClassDB::bind_method(D_METHOD("get_default_node_mass"), &ExampleClass::get_default_node_mass);
    ClassDB::bind_method(D_METHOD("set_gravity_multiplier", "new_gravity_multiplier"), &ExampleClass::set_gravity_multiplier);
    ClassDB::bind_method(D_METHOD("get_gravity_multiplier"), &ExampleClass::get_gravity_multiplier);

    // 节点位移功能绑定
    ClassDB::bind_method(D_METHOD("set_node_position", "index", "position"), &ExampleClass::set_node_position);
    ClassDB::bind_method(D_METHOD("move_node", "index", "displacement"), &ExampleClass::move_node);
    ClassDB::bind_method(D_METHOD("get_node_position", "index"), &ExampleClass::get_node_position);
    ClassDB::bind_method(D_METHOD("translate_all_nodes", "displacement"), &ExampleClass::translate_all_nodes);
    ClassDB::bind_method(D_METHOD("translate_nodes_range", "start_index", "end_index", "displacement"), &ExampleClass::translate_nodes_range);
    // 注册属性
    ADD_PROPERTY(PropertyInfo(Variant::INT, "node_count", PROPERTY_HINT_RANGE, "2,100"), "set_node_count", "get_node_count");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rope_length", PROPERTY_HINT_RANGE, "0.1,50.0"), "set_rope_length", "get_rope_length");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "gravity"), "set_gravity", "get_gravity");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_draw"), "set_debug_draw", "is_debug_drawing");
    


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
    applyForce(node_index, force, 0.016f); // 默认60FPS
}

void ExampleClass::reset_rope() {
    update_simulator();
}


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

    node_count = static_cast<int>(nodes.size());
    
  
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
    
    // UtilityFunctions::print("Cut rope at node ", index, ", new length: ", rope_length, ", new node count: ", nodes.size());
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
    
    // UtilityFunctions::print("Set node ", index, " position to: ", position);
}

void ExampleClass::move_node(int index, Vector2 displacement) {
    if (index < 0 || index >= static_cast<int>(nodes.size())) {
        UtilityFunctions::print("Error: Node index out of range");
        return;
    }
    
    nodes[index].position += displacement;
    nodes[index].oldPosition += displacement;
    
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
        node.oldPosition += displacement;
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
        nodes[i].oldPosition += displacement; 
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


ExampleClass::ExampleClass() {
    node_count = 20;
    rope_length = 1.0f;
    // gravity = Vector2(0, 9.8f);
    physics_gravity = gravity;
    segmentLength = 0.1f;
    debug_draw = true;
    isInitialized = false;
    default_node_mass = 1.0f;  
    
    // 弹性参数
    elasticity.constraint_iterations = 5;
    elasticity.stiffness = 1.0f;
    elasticity.damping = 0.99f;
    elasticity.constraint_strength = 0.5f;
    elasticity.stretch_resistance = 1.0f;
    elasticity.compression_resistance = 1.0f;
}

ExampleClass::~ExampleClass() {
    // 清理
    nodes.clear();
}

// 调试
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
    
    // 计算段长度
    segmentLength = totalLength / (numNodes - 1);
    
    // 创建节点
    for (int i = 0; i < numNodes; ++i) {
        Vector2 position = startPos + Vector2(0, i * segmentLength);
        nodes.emplace_back(position, default_node_mass, i == 0);
    }
    
    isInitialized = true;
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

// 在指定位置插入节点
void ExampleClass::add_node_at(int index, Vector2 position) {
    if (index < 0 || index > static_cast<int>(nodes.size())) {
        UtilityFunctions::print("Error: Invalid insertion index");
        return;
    }
    
    if (nodes.size() >= 100) {
        UtilityFunctions::print("Error: Maximum node count reached");
        return;
    }
    
    Vector2 insert_position;
    
    // 如果没有指定位置，计算插入位置
    if (position == Vector2(0, 0)) {
        if (index == 0) {
            // 在开头插入，位置在第一个节点之前
            insert_position = nodes[0].position - Vector2(0, segmentLength);
        } else if (index == static_cast<int>(nodes.size())) {
            // 在末尾插入，位置在最后一个节点之后
            insert_position = nodes.back().position + Vector2(0, segmentLength);
        } else {
            // 在中间插入，位置在两个节点的中点
            insert_position = (nodes[index - 1].position + nodes[index].position) * 0.5f;
        }
    } else {
        insert_position = position;
    }
    
    // 创建新节点
    RopeNode new_node(insert_position, default_node_mass, false);
    
    // 插入节点
    nodes.insert(nodes.begin() + index, new_node);
    
    // 更新节点数量
    node_count = static_cast<int>(nodes.size());
    
    // 重新计算段长度
    if (nodes.size() > 1) {
        float total_length = 0.0f;
        for (int i = 0; i < static_cast<int>(nodes.size()) - 1; ++i) {
            total_length += (nodes[i + 1].position - nodes[i].position).length();
        }
        segmentLength = total_length / (nodes.size() - 1);
        rope_length = total_length;
    }
    
    UtilityFunctions::print("Added node at index ", index, ", current nodes: ", nodes.size(), ", segment length: ", segmentLength);
}

// 在绳子末尾添加一个节点
void ExampleClass::add_node_to_end() {
    if (nodes.size() >= 100) {
        UtilityFunctions::print("Error: Maximum node count reached");
        return;
    }
    
    if (nodes.empty()) {
        UtilityFunctions::print("Error: No existing nodes to extend from");
        return;
    }
    
    // 计算新节点位置（在最后一个节点下方）
    Vector2 last_pos = nodes.back().position;
    Vector2 new_position;
    
    if (nodes.size() >= 2) {
        // 基于最后两个节点的方向延伸
        Vector2 direction = (nodes.back().position - nodes[nodes.size() - 2].position).normalized();
        new_position = last_pos + direction * segmentLength;
    } else {
        // 只有一个节点时，向下延伸
        new_position = last_pos + Vector2(0, segmentLength);
    }
    
    // 创建新节点
    RopeNode new_node(new_position, default_node_mass, false);
    nodes.push_back(new_node);
    
    // 更新参数
    node_count = static_cast<int>(nodes.size());
    rope_length = segmentLength * (node_count - 1);
    
    UtilityFunctions::print("Added node to end, current nodes: ", nodes.size(), ", new length: ", rope_length);
}

// 在绳子末尾添加多个节点
void ExampleClass::add_nodes_to_end(int count) {
    if (count <= 0) {
        UtilityFunctions::print("Error: Invalid node count");
        return;
    }
    
    if (nodes.size() + count > 100) {
        UtilityFunctions::print("Error: Would exceed maximum node count");
        return;
    }
    
    for (int i = 0; i < count; ++i) {
        add_node_to_end();
    }
    
    UtilityFunctions::print("Added ", count, " nodes to end, total nodes: ", nodes.size());
}

// 在两个节点之间插入一个节点
void ExampleClass::insert_node_between(int index1, int index2) {
    if (index1 < 0 || index2 < 0 || 
        index1 >= static_cast<int>(nodes.size()) || 
        index2 >= static_cast<int>(nodes.size()) ||
        abs(index1 - index2) != 1) {
        UtilityFunctions::print("Error: Invalid indices for insertion between nodes");
        return;
    }
    
    int insert_index = Math::max(index1, index2);
    Vector2 mid_position = (nodes[index1].position + nodes[index2].position) * 0.5f;
    
    add_node_at(insert_index, mid_position);
    
    UtilityFunctions::print("Inserted node between ", index1, " and ", index2);
}

// 通过增加长度来扩展绳子
void ExampleClass::extend_rope_by_length(float additional_length) {
    if (additional_length <= 0) {
        UtilityFunctions::print("Error: Invalid additional length");
        return;
    }
    
    // 计算需要添加的节点数
    int additional_nodes = static_cast<int>(additional_length / segmentLength);
    if (additional_nodes == 0) {
        additional_nodes = 1;  // 至少添加一个节点
    }
    
    add_nodes_to_end(additional_nodes);
    
    UtilityFunctions::print("Extended rope by ", additional_length, " units, added ", additional_nodes, " nodes");
}

// 通过增加节点数来扩展绳子
void ExampleClass::extend_rope_by_nodes(int additional_nodes) {
    if (additional_nodes <= 0) {
        UtilityFunctions::print("Error: Invalid additional node count");
        return;
    }
    
    add_nodes_to_end(additional_nodes);
    
    UtilityFunctions::print("Extended rope by ", additional_nodes, " nodes");
}

// 设置默认节点质量
void ExampleClass::set_default_node_mass(float mass) {
    default_node_mass = Math::clamp(mass, 0.01f, 100.0f);
    UtilityFunctions::print("Set default node mass to: ", default_node_mass);
}

float ExampleClass::get_default_node_mass() const {
    return default_node_mass;
}