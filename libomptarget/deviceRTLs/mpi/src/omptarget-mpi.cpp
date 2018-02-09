//===--- omptarget-nvptx.cu - NVPTX OpenMP GPU initialization ---- CUDA -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the initialization code for the GPU
//
//===----------------------------------------------------------------------===//

#include "stdio.h"

extern "C" void __kmpc_kernel_init(int ThreadLimit,
                                   int RequiresOMPRuntime) {
  printf("Imprimindo teste 2: __kmpc_kernel_init");
}
