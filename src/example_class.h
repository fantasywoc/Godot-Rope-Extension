#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"

#include "godot_cpp/variant/array.hpp"      // 新增：支持 Array 类型
#include "godot_cpp/variant/vector2.hpp"    // 新增：支持 Vector2 类型

#include "godot_cpp/variant/variant.hpp"

using namespace godot;

class ExampleClass : public RefCounted {
	GDCLASS(ExampleClass, RefCounted)

protected:
	static void _bind_methods();

public:
	ExampleClass() = default;
	~ExampleClass() override = default;

	void print_type(const Variant &p_variant) const;
	 // 新增函数：输入点数组，返回坐标乘以2后的新数组
    Array double_points(const Array &points) const; 
};
