#include "example_class.h"

void ExampleClass::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("print_type", "variant"), &ExampleClass::print_type);
	    // 绑定新增函数：参数名为 points
    godot::ClassDB::bind_method(D_METHOD("double_points", "points"), &ExampleClass::double_points);
}

void ExampleClass::print_type(const Variant &p_variant) const {
	print_line(vformat("VerletRopeType: %d", p_variant.get_type()));
}


// 实现坐标点倍增功能
Array ExampleClass::double_points(const Array &points) const {
    Array result;
    for (int i = 0; i < points.size(); i++) {
        if (points[i].get_type() == Variant::VECTOR2) {
            Vector2 point = points[i]; // 提取 Vector2 坐标
            result.append(Vector2(point.x * 2, point.y * 2)); // 坐标乘以2
        } else {
            UtilityFunctions::printerr("Element at index ", i, " is not a Vector2. Skipping.");
        }
    }
    return result;
}