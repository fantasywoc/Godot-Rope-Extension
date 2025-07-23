#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"

#include "godot_cpp/variant/array.hpp"      // 新增：支持 Array 类型
#include "godot_cpp/variant/vector2.hpp"    // 新增：支持 Vector2 类型

#include "godot_cpp/variant/variant.hpp"
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/method_bind.hpp>
#include <vector>

using namespace godot;

// 绳索节点结构体
struct RopeNode {
    Vector2 position;      // 当前位置
    Vector2 oldPosition;   // 上一帧位置
    float mass;            // 节点质量
    bool locked;           // 是否固定

    RopeNode(Vector2 pos, float m = 1.0f, bool isLocked = false)
        : position(pos), oldPosition(pos), mass(m), locked(isLocked) {}
};

// 绳索模拟器类
class RopeSimulator {
public:
    RopeSimulator() = default;
    RopeSimulator(int numNodes, float totalLength, Vector2 startPos, Vector2 gravity);
    
    void initializeRope(int numNodes, float totalLength, Vector2 startPos);
    void setNodeMass(int index, float mass);
    void setNodeLocked(int index, bool locked);
    void applyForce(int nodeIndex, Vector2 force, float deltaTime);
    void update(float deltaTime);
    void freezeInitialState() { isInitialized = false; }
    
    const std::vector<RopeNode>& getNodes() const { return nodes; }
    
private:
    std::vector<RopeNode> nodes;
    Vector2 gravity = Vector2(0, 9.8f);
    bool isInitialized = false;
    float segmentLength = 0.1f;
};

// 统一的绳索模拟器类（Godot扩展 + 物理模拟）
class ExampleClass : public RefCounted {
    GDCLASS(ExampleClass, RefCounted)

private:
    // 物理模拟数据（原RopeSimulator的私有成员）
    std::vector<RopeNode> nodes;
    Vector2 physics_gravity = Vector2(0, 9.8f);
    bool isInitialized = false;
    float segmentLength = 0.1f;
    
    // Godot接口数据
    bool debug_draw = true;
    Color rope_color = Color(0.8, 0.2, 0.2);
    
    // 导出属性
    int node_count = 10;
    float rope_length = 5.0;
    Vector2 gravity = Vector2(0, 9.8);
    
    // 内部方法
    void update_simulator();
    void initializeRope(int numNodes, float totalLength, Vector2 startPos);

protected:
    static void _bind_methods();

public:
    ExampleClass();
    ~ExampleClass();
    
    // Godot生命周期方法（新增）
    void _ready();
    void _process(double delta);
    void _draw();
    
    // 物理模拟方法（原RopeSimulator的公共方法）
    void setNodeMass(int index, float mass);
    void setNodeLocked(int index, bool locked);
    void applyForce(int nodeIndex, Vector2 force, float deltaTime);
    void update_physics(float deltaTime);
    void freezeInitialState() { isInitialized = false; }
    
    // 数据访问
    Array get_node_positions() const;
    
    // Godot属性访问器
    void set_node_count(int count);
    int get_node_count() const { return node_count; }
    
    void set_rope_length(float length);
    float get_rope_length() const { return rope_length; }
    
    void set_gravity(Vector2 g);
    Vector2 get_gravity() const { return gravity; }
    
    void set_debug_draw(bool enabled) { debug_draw = enabled; }
    bool is_debug_drawing() const { return debug_draw; }
    
    // 物理交互接口
    void apply_force(int node_index, Vector2 force);
    void reset_rope();
};