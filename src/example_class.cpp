#include "example_class.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/core/math.hpp>  // 修正路径

using namespace godot;

// 构造函数
ExampleClass::ExampleClass() {
    update_simulator();
}

ExampleClass::~ExampleClass() {}

// 物理模拟方法实现
void ExampleClass::initializeRope(int numNodes, float totalLength, Vector2 startPos) {
    nodes.clear();
    segmentLength = totalLength / (numNodes - 1);
    
    for (int i = 0; i < numNodes; ++i) {
        Vector2 pos = startPos + Vector2(i * segmentLength, 0);
        bool locked = (i == 0);
        nodes.emplace_back(pos, 1.0f, locked);
    }
}

void ExampleClass::setNodeMass(int index, float mass) {
    if (index >= 0 && index < static_cast<int>(nodes.size())) {
        nodes[index].mass = mass;
    }
}

void ExampleClass::setNodeLocked(int index, bool locked) {
    if (index >= 0 && index < static_cast<int>(nodes.size())) {
        nodes[index].locked = locked;
    }
}

void ExampleClass::applyForce(int nodeIndex, Vector2 force, float deltaTime) {
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes.size())) return;
    RopeNode& node = nodes[nodeIndex];
    if (node.locked) return;

    Vector2 adjustment = (force * (deltaTime * deltaTime)) / node.mass;
    node.oldPosition = node.oldPosition - adjustment;
}

void ExampleClass::update_physics(float deltaTime) {
    if (!isInitialized) {
        isInitialized = true;
        return;
    }

    // 韦尔莱积分更新位置
    for (auto& node : nodes) {
        if (node.locked) continue;
        
        Vector2 velocity = node.position - node.oldPosition;
        node.oldPosition = node.position;
        node.position += velocity + physics_gravity * deltaTime * deltaTime;
    }

    // 约束迭代
    const int iterations = 5;
    for (int iter = 0; iter < iterations; ++iter) {
        for (int i = 0; i < static_cast<int>(nodes.size()) - 1; ++i) {
            RopeNode& nodeA = nodes[i];
            RopeNode& nodeB = nodes[i + 1];
            
            Vector2 delta = nodeB.position - nodeA.position;
            float currentLength = delta.length();
            if (currentLength < 1e-6f) continue;
            
            float error = currentLength - segmentLength;
            Vector2 correction = delta.normalized() * error;

            if (!nodeA.locked && !nodeB.locked) {
                float totalMass = nodeA.mass + nodeB.mass;
                nodeA.position += correction * (0.5f * nodeB.mass / totalMass);
                nodeB.position -= correction * (0.5f * nodeA.mass / totalMass);
            } else if (!nodeA.locked) {
                nodeA.position += correction * 0.5f;
            } else if (!nodeB.locked) {
                nodeB.position -= correction * 0.5f;
            }
        }
    }
}

// Godot绑定方法
void ExampleClass::_bind_methods() {
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
    
    // Add this line to bind the setNodeLocked function
    ClassDB::bind_method(D_METHOD("setNodeLocked", "index", "locked"), &ExampleClass::setNodeLocked);
    // Add this line as well if you want to use setNodeMass
    ClassDB::bind_method(D_METHOD("setNodeMass", "index", "mass"), &ExampleClass::setNodeMass);
    
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

// Godot生命周期方法（修正后）
void ExampleClass::_ready() {
    update_simulator();
}

void ExampleClass::_process(double delta) {
    update_physics(static_cast<float>(delta));  // 修正：使用类的方法
}

void ExampleClass::_draw() {
    if (!debug_draw) return;
    
    // 修正：直接使用类的nodes成员
    for (size_t i = 0; i < nodes.size() - 1; ++i) {
        UtilityFunctions::print("Drawing rope segment from ", nodes[i].position, " to ", nodes[i+1].position);
    }
}

// 属性访问器方法
// 使用Math::clamp的方法
void ExampleClass::set_node_count(int count) {
    node_count = Math::clamp(count, 2, 100);  // 现在可以正确使用Math::clamp
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