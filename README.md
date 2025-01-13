# compiler

compiler for SysY2022 language.

## Usage

Usage:
compiler [options] file

Options:
  -?                       Display this information.
  -S                       Compile only; do not assemble or link.
  -o <file>                Place the output into <file>.

The following options control optimizations:
  -O<number>    Set optimization level to <number>.

## Frontend

Function name|Description
:-:|:-:
ParseFile|Parse source file
ParseString|Parse String

## Machine-independent Optimizations

Name|Function name|Description
:-:|:-:|:-:
Constant Folding|FoldFltUnop、FoldFltBinop、FoldIntBinop、FoldIntUnop|
Loop Inversion|copy_loop_headers|
Inline Transform|inline_transform|
SSA Form Construction|build_ssa|
 Sparse Conditional Constant Propagation |SparseCondConstProp|
Global Value Numbering|GlobalValueNumbering|
SSA Form Elimination|remove_ssa_form|
SSA Form Elimination|rewrite_out_of_ssa|
Lazy Code Motion|LazyCodeMotion|
Copy Propagation|copyprop|
Local Copy Propagation|local_copyprop_pass|
Dead Code Elimination|perform_ssa_dce|
Strength Reduction|StraightLineStrengthReduce|
Elimination of Useless Control Flow|cleanup_cfg|
Global Variable Localization|GlobalVariableLocalization|
Operator Strength Reduction|OSR|
Loop Unrolling|unroll_loops|
Tree Height Reducing|treeheight|

## Backend

Name|Function name|Description
:-:|:-:|:-:
Instruction Selection and Scheduling|InstSelectorArm32|Algorithm: TreeSeq
Register Allocation|LinearScanAllocator|Algorithm: Linear Scan
Register Allocation|ra_colorize_graph|Algorithm: Graph Coloring
if convert|if_convertArm32|

## Requirements

Before you start using the compiler, please review the requirements listed below. Familiarizing yourself with the necessary hardware and software in advance may save you some trouble.

### Hardware 

compiler is known to run on the following host platforms:
OS|Arch|Compilers
:-:|:-:|:-:
Linux|x86|GCC, Clang
Linux|amd64|GCC, Clang
Linux|ARM|GCC, Clang
Windows|x86|Visual Studio
Windows x64|x86-64|Visual Studio

### Software 

To compile compiler, you need to install several packages. The following table lists the required packages. The `Package` column is the common name of the package on which the compiler depends. The `Version` column provides the "known available" version of the package. The `Notes` column describes how the compiler uses the package and provides other details.

Package|Version|Notes
:-:|:-:|:-:
[CMake](https://cmake.org/)|>=3.20.0|Makefile/workspace generator
GNU Make|3.79, 3.79.1|Makefile/build processor
flex|2.6.4|Lexical analysis
bison|3.8.2|Syntax analysis
[iburg](https://github.com/drh/iburg)|未知|Tree pattern matching
graphviz|11.0.0|Graphical output


Copyright (c) 2025 Anonymous
