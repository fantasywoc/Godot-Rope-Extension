#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/vector2.hpp"
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

// 弹性参数结构体
struct ElasticityParams {
    int constraint_iterations = 5;        // 约束迭代次数 (1-20)
    float stiffness = 1.0f;              // 刚度系数 (0.1-3.0)
    float damping = 0.99f;               // 阻尼系数 (0.7-1.0)
    float constraint_strength = 0.5f;     // 约束强度 (0.1-1.0)
    float stretch_resistance = 1.0f;      // 拉伸阻力 (0.5-2.0)
    float compression_resistance = 1.0f;  // 压缩阻力 (0.5-2.0)
    
    // 预设模式
    void setRigid() {
        constraint_iterations = 12;
        stiffness = 0.8f;
        damping = 0.98f;
        constraint_strength = 1.0f;
        stretch_resistance = 0.8f;
        compression_resistance = 1.2f;
    }
    
    void setElastic() {
        constraint_iterations = 6;
        stiffness = 1.2f;
        damping = 0.95f;
        constraint_strength = 0.7f;
        stretch_resistance = 1.2f;
        compression_resistance = 1.0f;
    }
    
    void setSoft() {
        constraint_iterations = 3;
        stiffness = 0.6f;
        damping = 0.92f;
        constraint_strength = 0.4f;
        stretch_resistance = 0.8f;
        compression_resistance = 0.6f;
    }
    
    void setBouncy() {
        constraint_iterations = 8;
        stiffness = 1.8f;
        damping = 0.98f;
        constraint_strength = 0.8f;
        stretch_resistance = 1.5f;
        compression_resistance = 1.2f;
    }
};

class ExampleClass : public RefCounted {
    GDCLASS(ExampleClass, RefCounted)

private:
    // 物理模拟数据
    std::vector<RopeNode> nodes;
    Vector2 physics_gravity = Vector2(0, 9.8f);
    bool isInitialized = false;
    float segmentLength = 0.1f;
    
    // 弹性控制参数
    ElasticityParams elasticity;
    
    // Godot接口数据
    bool debug_draw = true;
    Color rope_color = Color(0.8, 0.2, 0.2);
    
    // 导出属性
    int node_count = 10;
    float rope_length = 5.0;
    Vector2 gravity = Vector2(0, 9.8);
    float default_node_mass = 1.0f;
    
    // 添加缺失的私有函数声明
    void update_simulator();
    void initializeRope(int numNodes, float totalLength, Vector2 startPos);
    void freezeInitialState();
    void applyConstraints(float deltaTime);
    void applyForce(int node_index, Vector2 force, float deltaTime);

protected:
    static void _bind_methods();

public:
    ExampleClass();
    ~ExampleClass();
    
    // 添加默认节点质量访问器
    void set_default_node_mass(float mass);
    float get_default_node_mass() const;
    
    // === 核心弹性控制函数 ===
    void setElasticity(float stiffness, float damping, int iterations = -1, 
                      float constraint_strength = -1.0f, 
                      float stretch_resistance = -1.0f,
                      float compression_resistance = -1.0f);
    
    void setElasticityPreset(const String& preset_name);
    
    // 高级弹性控制
    void setAdvancedElasticity(const Dictionary& params);
    Dictionary getElasticityParams() const;
    
    // 实时弹性调整
    void adjustStiffness(float delta);
    void adjustDamping(float delta);
    void adjustConstraintStrength(float delta);
    
    // 弹性参数访问器
    void set_stiffness(float value) { elasticity.stiffness = Math::clamp(value, 0.1f, 3.0f); }
    float get_stiffness() const { return elasticity.stiffness; }
    
    void set_damping(float value) { elasticity.damping = Math::clamp(value, 0.7f, 1.0f); }
    float get_damping() const { return elasticity.damping; }
    
    void set_constraint_iterations(int value) { elasticity.constraint_iterations = Math::clamp(value, 1, 20); }
    int get_constraint_iterations() const { return elasticity.constraint_iterations; }
    
    void set_constraint_strength(float value) { elasticity.constraint_strength = Math::clamp(value, 0.1f, 1.0f); }
    float get_constraint_strength() const { return elasticity.constraint_strength; }
    
    // 调试相关函数
    void set_debug_draw(bool enabled);
    bool is_debug_drawing() const;
    
    // 节点操作函数
    void remove_node(int index);
    void cut_rope_at(int index);
    int get_current_node_count() const;
    
    // 新增：节点增加函数
    void add_node_at(int index, Vector2 position = Vector2(0, 0));
    void add_node_to_end();
    void add_nodes_to_end(int count);
    void insert_node_between(int index1, int index2);
    
    // 新增：批量节点操作
    void extend_rope_by_length(float additional_length);
    void extend_rope_by_nodes(int additional_nodes);
    
    // 节点位置操作函数
    void set_node_position(int index, Vector2 position);
    void move_node(int index, Vector2 displacement);
    Vector2 get_node_position(int index) const;
    void translate_all_nodes(Vector2 displacement);
    void translate_nodes_range(int start_index, int end_index, Vector2 displacement);
    
    // 节点状态查询
    bool get_node_locked(int index) const;
    
    // 速度控制函数
    void add_velocity_damping(int index, float damping_factor);
    void set_node_velocity(int index, Vector2 velocity);
    Vector2 get_node_velocity(int index) const;
    
    // 其他现有方法
    void update_physics(float deltaTime);
    Array get_node_positions() const;
    void setNodeMass(int index, float mass);
    void setNodeLocked(int index, bool locked);
    void apply_force(int node_index, Vector2 force);
    void reset_rope();
    
    // 属性访问器
    void set_node_count(int count);
    int get_node_count() const { return node_count; }
    void set_rope_length(float length);
    float get_rope_length() const { return rope_length; }
    void set_gravity(Vector2 g);
    Vector2 get_gravity() const { return gravity; }
    
    // 生命周期方法
    void _ready();
    void _process(double delta);
    void _draw();
};
