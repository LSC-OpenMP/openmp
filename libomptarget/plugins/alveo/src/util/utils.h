#ifndef UTILS_H_
#define UTILS_H_

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
#include <CL/opencl.h>
#include <CL/cl_ext.h>
#include <vector>

#include "omptargetplugin.h"

#define NUM_WORKGROUPS (1)
#define WORKGROUP_SIZE (256)
#define MAX_LENGTH 8192
#define MEM_ALIGNMENT 4096

#if defined(SDX_PLATFORM) && !defined(TARGET_DEVICE)
#define STR_VALUE(arg)      #arg
#define GET_STRING(name) STR_VALUE(name)
#define TARGET_DEVICE GET_STRING(SDX_PLATFORM)
#endif

int init_util(const char *xclbin);
void* data_alloc(int size);
void data_submit(void *tgt_ptr, void *hst_ptr, int64_t size);
int run_target();
void data_retrieve(void *hst_ptr, void *tgt_ptr, int size);
void data_delete(void *tgt_ptr);
void cleanup();

#endif // UTILS_H_

