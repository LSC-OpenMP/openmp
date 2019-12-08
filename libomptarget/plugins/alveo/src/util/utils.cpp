#include "utils.h"

int err;                       // error code returned from api calls

cl_platform_id   platform_id;  // platform id
cl_device_id     device_id;    // compute device id
cl_context       context;      // compute context
cl_command_queue commands;     // compute command queue
cl_program       program;      // compute programs
cl_kernel        kernel;       // compute kernel
cl_mem_ext_ptr_t mem_ext;      // define memory bank

int counter_mem_flags;

char cl_platform_vendor[1001];
char target_device_name[1001] = TARGET_DEVICE;

std::vector<cl_mem>  clmem_ptrs;     // buffers pointers
std::vector<cl_uint> scalars_values; // buffers sizes

int load_file_to_memory(const char *filename, char **result) {
  int size = 0;
  FILE *f = fopen(filename, "rb");

  if (f == NULL) {
    *result = NULL;

    return -1; // file opening fail
  }

  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fseek(f, 0, SEEK_SET);

  *result = (char *)malloc(size + 1);

  if (size != fread(*result, sizeof(char), size, f)) {
    free(*result);

    return -2; // file reading fail
  }

  fclose(f);

  (*result)[size] = 0;

  return size;
}

int init_util(const char *xclbin) {
  // get all platforms and then select xilinx platform
  cl_platform_id platforms[16];
  cl_uint platform_count;
  int platform_found = 0;

  err = clGetPlatformIDs(16, platforms, &platform_count);

  if (err != CL_SUCCESS) {
    printf("Error: Failed to find an OpenCL platform!\n");
    printf("Test failed\n");

    return OFFLOAD_FAIL;
  }

  printf("INFO: Found %d platforms\n", platform_count);

  // find xilinx plaftorm
  for (unsigned int iplat = 0; iplat < platform_count; iplat++) {
    err = clGetPlatformInfo(platforms[iplat], CL_PLATFORM_VENDOR, 1000, (void *)cl_platform_vendor,NULL);

    if (err != CL_SUCCESS) {
      printf("Error: clGetPlatformInfo(CL_PLATFORM_VENDOR) failed!\n");
      printf("Test failed\n");

      return OFFLOAD_FAIL;
    }

    if (strcmp(cl_platform_vendor, "Xilinx") == 0) {
      printf("INFO: Selected platform %d from %s\n", iplat, cl_platform_vendor);
      platform_id = platforms[iplat];
      platform_found = 1;
    }
  }

  if (!platform_found) {
    printf("ERROR: Platform Xilinx not found. Exit.\n");

    return OFFLOAD_FAIL;
  }

  // get accelerator compute device
  cl_uint num_devices;
  unsigned int device_found = 0;
  cl_device_id devices[16];  // compute device id
  char cl_device_name[1001];

  err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ACCELERATOR, 16, devices, &num_devices);

  printf("INFO: Found %d devices\n", num_devices);

  if (err != CL_SUCCESS) {
    printf("ERROR: Failed to create a device group!\n");
    printf("ERROR: Test failed\n");

    return OFFLOAD_FAIL;
  }

  // iterate all devices to select the target device.
  for (uint i = 0; i < num_devices; i++) {
    err = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 1024, cl_device_name, 0);

    if (err != CL_SUCCESS) {
      printf("Error: Failed to get device name for device %d!\n", i);
      printf("Test failed\n");

      return OFFLOAD_FAIL;
    }

    printf("CL_DEVICE_NAME %s\n", cl_device_name);

    if(strcmp(cl_device_name, target_device_name) == 0) {
      device_id = devices[i];
      device_found = 1;
      printf("Selected %s as the target device\n", cl_device_name);
    }
  }

  if (!device_found) {
    printf("Target device %s not found. Exit.\n", target_device_name);

    return OFFLOAD_FAIL;
  }

  // create a compute context
  context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

  if (!context) {
    printf("Error: Failed to create a compute context!\n");
    printf("Test failed\n");

    return OFFLOAD_FAIL;
  }

  commands = clCreateCommandQueue(context, device_id, 0, &err);

  if (!commands) {
    printf("Error: Failed to create commands!\n");
    printf("Error: code %i\n",err);
    printf("Test failed\n");

    return OFFLOAD_FAIL;
  }

  int status;

  // create program objects
  // load binary from disk
  unsigned char *kernelbinary;

  //------------------------------------------------------------------------------
  // xclbin
  //------------------------------------------------------------------------------
  const char *extension = ".xclbin";

  char *name_binary;

  name_binary = (char*) malloc (strlen(xclbin) + strlen(extension) + 1);

  strcpy(name_binary, xclbin);
  strcat(name_binary, extension);

  printf("INFO: loading xclbin %s\n", name_binary);

  // int n_i0 = load_file_to_memory("loopback.xclbin", (char **) &kernelbinary);
  int n_i0 = load_file_to_memory(name_binary, (char **) &kernelbinary);

  if (n_i0 < 0) {
    printf("failed to load kernel from xclbin: %s\n", name_binary);
    printf("Test failed\n");

    return OFFLOAD_FAIL;
  }

  free(name_binary);

  size_t n0 = n_i0;

  // create the compute program from offline

  program = clCreateProgramWithBinary(context, 1, &device_id, &n0,
      (const unsigned char **) &kernelbinary, &status, &err);

  if ((!program) || (err != CL_SUCCESS)) {
    printf("Error: Failed to create compute program from binary %d!\n", err);
    printf("Test failed\n");

    return OFFLOAD_FAIL;
  }

  // build the program executable
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

  if (err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];

    printf("Error: Failed to build program executable!\n");

    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);

    printf("%s\n", buffer);
    printf("Test failed\n");

    return OFFLOAD_FAIL;
  }

  // create the compute kernel in the program we wish to run
  kernel = clCreateKernel(program, "hardcloud_top", &err);

  if (!kernel || err != CL_SUCCESS) {
    printf("Error: Failed to create compute kernel!\n");
    printf("Test failed\n");

    return OFFLOAD_FAIL;
  }

  mem_ext.obj = NULL;
  mem_ext.param = kernel;

  counter_mem_flags = 3;

  return OFFLOAD_SUCCESS;
}

void* data_alloc(int size) {
  mem_ext.flags = counter_mem_flags;

  counter_mem_flags = counter_mem_flags - 1;

  cl_mem dt = clCreateBuffer(context,  CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX,
      size, &mem_ext, NULL);

  if (!(dt)) {
    printf("Error: Failed to allocate device memory!\n");
    printf("Test failed\n");
  }

  clmem_ptrs.push_back(dt);
  scalars_values.push_back((cl_uint) size);

  return (void*) dt;
}

void data_submit(void *tgt_ptr, void *hst_ptr, int64_t size) {
  err = clEnqueueWriteBuffer(commands, (cl_mem) tgt_ptr, CL_TRUE, 0, sizeof(char) * size,
      hst_ptr, 0, NULL, NULL);

  if (err != CL_SUCCESS) {
    printf("Error: Failed to submit data. tgt_ptr %p, hst_ptr %p, size %d!\n", tgt_ptr, hst_ptr, size);
    printf("Test failed\n");
  }
}

int run_target() {
  cl_uint args_counter = 0;

  err = 0;

  for(auto scl : scalars_values) {
    err = clSetKernelArg(kernel, args_counter, sizeof(cl_uint), &scl);

    if(err != CL_SUCCESS){
      printf("Error: Failed to set kernel arguments! %d\n", err);
      printf("Test failed\n");

      return OFFLOAD_FAIL;
    }

    args_counter++;
  }

  args_counter = 3;
  for(auto ptrs : clmem_ptrs) {
    err = clSetKernelArg(kernel, args_counter, sizeof(cl_mem), &ptrs);

    if(err != CL_SUCCESS) {
      printf("Error: Failed to set kernel arguments! %d\n", err);
      printf("Test failed\n");

      return OFFLOAD_FAIL;
    }

    args_counter--;
  }

  // Execute the kernel over the entire range of our 1d input data set
  // using the maximum number of work group items for this device
  clFinish(commands);

  err = clEnqueueTask(commands, kernel, 0, NULL, NULL);

  if (err) {
    printf("Error: Failed to execute kernel! %d\n", err);
    printf("Test failed\n");

    return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

void data_retrieve(void *hst_ptr, void *tgt_ptr, int size) {
  cl_event readevent;

  clFinish(commands);

  err = 0;
  err |= clEnqueueReadBuffer(commands, (cl_mem) tgt_ptr, CL_TRUE, 0, size, hst_ptr, 0, NULL, &readevent);

  if (err != CL_SUCCESS) {
    printf("Error: Failed to read output array! %d\n", err);
    printf("Test failed\n");
  }

  clWaitForEvents(1, &readevent);
}

void data_delete(void *tgt_ptr) {
  printf("%s\n", "UTILS INFO: calling data_delete \n");

  clReleaseMemObject((cl_mem)tgt_ptr);
}

void cleanup() {
  printf("%s\n", "UTILS INFO: calling cleanup \n");

  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(commands);
  clReleaseContext(context);
}

