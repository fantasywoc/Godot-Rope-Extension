#!/usr/bin/env python
import os
import sys

from methods import print_error

# 库文件名
libname = "EXTENSION-NAME"
# 项目路径
projectdir = "demo"

localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
# 使用godot-cpp兼容的参数名
opts.Add(EnumVariable('precision', 'Floating point precision (single or double)', 'single', 
                      allowed_values=('single', 'double')))
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print_error("""godot-cpp is not available within this folder, as Git submodules haven't been initialized.
Run the following command to download godot-cpp:

    git submodule update --init --recursive""")
    sys.exit(1)

# 确保precision参数传递给godot-cpp
env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

# 移除自定义的精度宏定义，使用godot-cpp的标准配置
precision = env['precision']
if precision == 'double':
    print("Building with DOUBLE precision")
else:
    print("Building with SINGLE precision")

# 添加构建类型配置
if env["target"] == "template_release":
    env.Append(CCFLAGS=['-O2'])
    print("Building RELEASE version")
else:
    env.Append(CCFLAGS=['-g'])
    print("Building DEBUG version")

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

# 使用godot-cpp的标准suffix，它已经包含了精度信息
suffix = env['suffix'].replace(".dev", "").replace(".universal", "")

lib_filename = "{}{}{}{}".format(env.subst('$SHLIBPREFIX'), libname, suffix, env.subst('$SHLIBSUFFIX'))

library = env.SharedLibrary(
    "bin/{}/{}".format(env['platform'], lib_filename),
    source=sources,
)

copy = env.Install("{}/bin/{}/".format(projectdir, env["platform"]), library)

default_args = [library, copy]
Default(*default_args)