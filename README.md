# godot-cpp template
This repository serves as a quickstart template for GDExtension development with Godot 4.0+.

# godot-cpp-extension
godot C++ 扩展
# 编译

```bash
#清理缓存
scons -c
#编译（默认编译debug single）
scons
```
### release版本
```scons target=template_release precision=single```
```scons target=template_release precision=double```

### debug版本
```scons target=template_debug precision=single```
```scons target=template_debug precision=double```



## 构建扩展代码
 1. 将src/example_class.cpp 和 src/example_class.h 替换为你的代码，并进行函数绑定```_bind_methods()```
 2. 修改注册代码src/register_types.cpp -> ```GDREGISTER_CLASS(ExampleClass)```匹配你的类名  	

## 修改库文件名配置
 假设你的库文件名为  ```Your-lib-Name```
1. /CMakeLists.txt 
   ```set(LIBNAME "EXTENSION-NAMET" CACHE STRING "The name of the library")```
   将```EXTENSION-NAMET```修改为```Your-lib-Name```
2. ./EXTENSION-VERLET
   ```libname = "EXTENSION-NAMET" ```
   将```EXTENSION-NAMET```修改为```Your-lib-Name```




## `example.gdextension` 文件配置
   匹配你编译的库文件版本，例如windows编译。
   ```windows.x86_64.single.debug = "./windows/EXTENSION-NAME.windows.template_debug.x86_64.dll"```
   将 ```EXTENSION-NAME.windows.template_debug.x86_64.dll``` 替换为 ```Your-lib-Name.windows.template_debug.x86_64.dll```

# 扩展的使用方法

将编译好的文件放到godot项目对应的bin/目录下
```
   ./bin
      ├─android
      ├─linux
      ├─macos
      ├─windows
      └─example.gdextension
```
启动demo(你的项目)即可使用函数
```	
   var example := ExampleClass.new()
	example.print_type(example)
```
## 扩展使用注意
单精度库需要单精度godot编辑器

### Debug版本 (template_debug)
- 开发阶段 ：在Godot编辑器中运行项目时
- 调试模式 ：使用"Play"按钮运行项目时
- 特点 ：包含调试信息，性能较慢，但便于调试
### Release版本 (template_release)
- 最终发布 ：当你导出/打包项目为最终产品时
- 生产环境 ：分发给最终用户的版本
- 特点 ：优化编译，性能更好，体积更小




`.gdextension` 文件是 Godot 4 引入的**手动创建的配置文件**，用于替代 Godot 3 的 `.gdnlib` 文件。

## 文件创建方式

### 手动创建步骤：
1. 在文本编辑器中创建新文件
2. 保存为 `your_extension.gdextension`（名称与你的扩展相关）
3. 使用以下基本结构填充内容：

```ini
[configuration]
entry_symbol = "gdextension_library_init"
compatibility_minimum = "4.1"

[libraries]
windows.x86_64 = "res://bin/libgdexample.dll"
linux.x86_64 = "res://bin/libgdexample.so"
macos = "res://bin/libgdexample.dylib"
```

### 关键配置项说明：

| 节           | 键                  | 描述                                                                 |
|--------------|---------------------|----------------------------------------------------------------------|
| `[configuration]` | `entry_symbol`        | **必须与 C++ 代码中的函数名匹配**，通常是 `gdextension_library_init` |
|              | `compatibility_minimum` | 最低支持的 Godot 版本 (如 "4.0", "4.1")                             |
| `[libraries]`    | `windows.x86_64`       | Windows 平台库文件路径                                                |
|              | `linux.x86_64`         | Linux 平台库文件路径                                                  |
|              | `macos`                | macOS 平台库文件路径                                                  |
| `[dependencies]` | 平台标识              | 依赖的附加库 (可选)                                                  |

## 与代码的关联

### C++ 入口点示例
`.gdextension` 文件中的 `entry_symbol` 必须与 C++ 代码中的初始化函数匹配：

```cpp
// 必须与 .gdextension 中的 entry_symbol 完全一致
extern "C" {
GDE_EXPORT GDExtensionBool GDE_EXPORT gdextension_library_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization *r_initialization
) {
    // 初始化代码
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
    init_obj.register_initializer(initialize_module);
    init_obj.register_terminator(uninitialize_module);
    return init_obj.init();
}
}
```

## 配置示例

1. **文件位置**：
   - 通常放在项目根目录的 `bin/` 或 `addons/your_extension/` 目录
   - 确保路径与库文件实际位置匹配

2. **多平台支持**：
   ```ini
   [libraries]
   # Windows
   windows.x86_64 = "res://bin/windows/libgdexample.dll"
   
   # Linux
   linux.x86_64 = "res://bin/linux/libgdexample.so"
   
   # macOS (通用二进制)
   macos = "res://bin/macos/libgdexample.dylib"
   ```

3. **版本控制**：
   ```ini
   [configuration]
   compatibility_minimum = "4.1"
   reloadable = true
   ```

4. **依赖管理** (如果需要)：
   ```ini
   [dependencies]
   windows.x86_64 = [ "res://bin/windows/dependency.dll" ]
   linux.x86_64 = [ "res://bin/linux/libdependency.so" ]
   ```

## 工作原理

当 Godot 启动时：
1. 扫描项目目录查找 `.gdextension` 文件
2. 根据当前运行平台加载对应的库文件
3. 通过 `entry_symbol` 查找并调用初始化函数
4. 注册扩展中定义的类和方法



