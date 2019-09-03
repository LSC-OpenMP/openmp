#ifndef SRC_SDX_RTL_KERNEL_WIZARD_SDX_KERNEL_WIZARD_0_EX_SDX_IMPORTS_UTILS_H_
#define SRC_SDX_RTL_KERNEL_WIZARD_SDX_KERNEL_WIZARD_0_EX_SDX_IMPORTS_UTILS_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
// #include <CL/opencl.h>
#include <vector>

#define NUM_WORKGROUPS (1)
#define WORKGROUP_SIZE (256)
#define MAX_LENGTH 8192

#if defined(SDX_PLATFORM) && !defined(TARGET_DEVICE)
#define STR_VALUE(arg)      #arg
#define GET_STRING(name) STR_VALUE(name)
#define TARGET_DEVICE GET_STRING(SDX_PLATFORM)
#endif

void init_util();
void* data_alloc(int size);
void data_submit(void *tgt_ptr, void *hst_ptr, int64_t size);
void run_target();
void data_retrieve(void *hst_ptr, void *tgt_ptr, int size);
void data_delete(void *tgt_ptr);
void cleanup();

#endif /* SRC_SDX_RTL_KERNEL_WIZARD_SDX_KERNEL_WIZARD_0_EX_SDX_IMPORTS_UTILS_H_ */

