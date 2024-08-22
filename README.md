# compiler

compiler为2024年“华为毕昇杯”编译系统设计赛-编译系统实现赛，“正道的光”队的参赛作品。本作品获得ARM赛道二等奖。

## 主要功能

命令格式:
compiler [选项] 文件

选项-S 编译到汇编语言，不进行汇编和链接

选项-o <文件> 输出到 <文件>

选项-O<N> 将优化等级设为 N

选项-g 在汇编代码中生成详细的调试信息

## 前端

函数名|描述
:-:|:-:
ParseFile|解析源文件
ParseString|解析字符串

## 机器无关优化

名称|函数名|描述
:-:|:-:|:-:
常量折叠|FoldFltUnop、FoldFltBinop、FoldIntBinop、FoldIntUnop|
循环倒置|copy_loop_headers|
内联替换|inline_transform|
SSA形式的构建|build_ssa|
稀疏条件常量传播|SparseCondConstProp|
全局值编号|GlobalValueNumbering|
SSA形式的消去|remove_ssa_form|
SSA形式的消去|rewrite_out_of_ssa|
懒惰代码移动|LazyCodeMotion|
复写传播|copyprop|
复写传播（未使用）|CopyPropPass|
局部复写传播|local_copyprop_pass|
死代码消除|perform_ssa_dce|
弱强度削减|StraightLineStrengthReduce|
消除无用控制流|cleanup_cfg|
全局变量局部化|GlobalVariableLocalization|
运算符强度削减|OSR|
循环展开|unroll_loops|
树高平衡|treeheight|

## 后端

名称|函数名|描述
:-:|:-:|:-:
指令选择|InstSelectorArm32|算法：树模式匹配
寄存器分配|LinearScanAllocator|算法：线性扫描寄存器分配
寄存器分配（未使用）|regallocArm32|算法：图着色寄存器分配
条件指令|if_convertArm32|

## 要求

在开始使用compiler之前，请先查看下面给出的要求。提前了解所需的硬件和软件可能会为您省去一些麻烦。

### 硬件

compiler已知可以在以下主机平台上运行：
OS|Arch|Compilers
:-:|:-:|:-:
Linux|x86|GCC, Clang
Linux|amd64|GCC, Clang
Linux|ARM|GCC, Clang
Windows|x86|Visual Studio
Windows x64|x86-64|Visual Studio

### 软件

编译compiler需要安装多个软件包。下表列出了所需的软件包。Package列是compiler所依赖的软件包的常用名称。Version列提供该软件包的“已知可用”版本。Notes列描述了compiler如何使用该软件包并提供其他详细信息。

Package|Version|Notes
:-:|:-:|:-:
[CMake](https://cmake.org/)|>=3.20.0|Makefile/workspace generator
GNU Make|3.79, 3.79.1|Makefile/build processor
flex|2.6.4|词法分析
bison|3.8.2|语法分析
[iburg](https://github.com/drh/iburg)|未知|树模式匹配
graphviz|11.0.0|图形输出


版权所有 (c) 2024 西北工业大学正道的光编译器开发小组。保留所有权利。

作者：苗潼超。

感谢谭钰蓁、袁竟程同学提供的技术支持。

特别感谢曾雷杰、林奕老师提供的理论、技术指导。

