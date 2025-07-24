# Godot C++ 绳索物理模拟器项目

## 项目概述

这是一个基于 Godot 4 和 C++ 的高级绳索物理模拟器项目，使用 GDExtension 技术实现了高性能的柔体物理模拟。
# 编译

```bash
#清理缓存
scons -c
#编译（默认编译debug single）
scons
```
### release版本
```bash
scons target=template_release precision=single
scons target=template_release precision=double

```

### debug版本
```bash
scons target=template_debug precision=single
scons target=template_debug precision=double
```

  


## 核心特性

### 🎯 物理模拟
- **Verlet 积分算法** - 稳定的物理计算
- **约束求解系统** - 精确的距离约束
- **重力和外力支持** - 真实的物理效果
- **质量和阻尼控制** - 可调节的物理参数

### ⚙️ 弹性控制
- **多种预设模式**：刚性、弹性、柔软、弹跳
- **实时参数调整**：刚度、阻尼、约束强度
- **高级弹性配置**：拉伸/压缩阻力独立控制
- **约束迭代控制**：精度与性能平衡

### 🎮 交互功能
- **节点拖拽** - 鼠标实时操作
- **节点锁定/解锁** - 右键固定节点
- **绳索剪切** - 双击剪断绳索
- **节点移除** - 动态调整绳索结构
- **批量节点操作** - 范围移动和变换

### 🔧 开发工具
- **调试绘制** - 可视化绳索状态
- **键盘快捷键** - 快速切换模式
- **参数实时调整** - 开发时便捷调试
- **完整的 Godot 文档** - 函数提示和说明

## 项目结构

```
Godot_c++/
├── src/                    # C++ 源代码
│   ├── example_class.h     # 绳索类头文件
│   ├── example_class.cpp   # 绳索类实现
│   └── register_types.cpp  # Godot 类型注册
├── demo/                   # 演示项目
│   ├── rope_demo.gd        # GDScript 演示脚本
│   └── example.tscn        # 演示场景
├── doc_classes/            # Godot 文档
│   └── ExampleClass.xml    # 类文档定义
└── bin/                    # 编译输出
    └── windows/            # 平台特定库文件
```

- **Godot 4.x** - 游戏引擎
- **C++17** - 核心语言
- **GDExtension** - Godot 扩展接口
- **CMake/SCons** - 构建系统
- **Verlet 积分** - 物理算法

## 主要类和方法

### ExampleClass (绳索模拟器)

**核心方法：**
- `setElasticity()` - 设置弹性参数
- `setElasticityPreset()` - 使用预设模式
- `update_physics()` - 物理更新
- `apply_force()` - 施加外力

**节点操作：**
- `set_node_position()` - 设置节点位置
- `setNodeLocked()` - 锁定/解锁节点
- `remove_node()` - 移除节点
- `cut_rope_at()` - 剪断绳索

**属性控制：**
- `node_count` - 节点数量 (2-100)
- `rope_length` - 绳索长度
- `gravity` - 重力向量
- `debug_draw` - 调试绘制


## 应用场景

- 🎮 **游戏开发** - 绳索、链条、藤蔓等
- 🎨 **动画制作** - 柔体动画效果
- 🔬 **物理模拟** - 教育和研究用途
- 🛠️ **工具开发** - 物理编辑器组件

## 性能特点

- ✅ **高性能 C++ 实现** - 适合实时应用
- ✅ **可调节精度** - 迭代次数控制
- ✅ **内存友好** - 高效的数据结构
- ✅ **跨平台支持** - Windows/Linux/macOS

## 开发状态

- 🔄 持续优化和功能扩展


        
