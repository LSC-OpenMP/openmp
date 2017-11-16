//===----RTLs/opencl/src/rtl.cpp - Target RTLs Implementation ------ C++
//-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// RTL for opencl drivers
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <list>
#include <vector>
#ifdef __APPLE__
#include <OpenCL/cl.h>
#elif __ALTERA__
#include <CL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "../../../src/omptargetplugin.h"

#ifndef TARGET_NAME
#define TARGET_NAME OPENCL
#endif

#define OFFLOADSECTIONNAME ".omp_offloading.entries"

/// Keep entries table per device.
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::vector<__tgt_offload_entry> Entries;
  std::vector<cl_kernel> Kernels;
};

void clErrorCode(cl_int status) {
  char *code;
  switch (status) {
  case CL_DEVICE_NOT_FOUND:
    code = "CL_DEVICE_NOT_FOUND";
    break;
  case CL_DEVICE_NOT_AVAILABLE:
    code = "CL_DEVICE_NOT_AVAILABLE";
    break;
  case CL_COMPILER_NOT_AVAILABLE:
    code = "CL_COMPILER_NOT_AVAILABLE";
    break;
  case CL_MEM_OBJECT_ALLOCATION_FAILURE:
    code = "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    break;
  case CL_OUT_OF_RESOURCES:
    code = "CL_OUT_OF_RESOURCES";
    break;
  case CL_OUT_OF_HOST_MEMORY:
    code = "CL_OUT_OF_HOST_MEMORY";
    break;
  case CL_PROFILING_INFO_NOT_AVAILABLE:
    code = "CL_PROFILING_INFO_NOT_AVAILABLE";
    break;
  case CL_MEM_COPY_OVERLAP:
    code = "CL_MEM_COPY_OVERLAP";
    break;
  case CL_IMAGE_FORMAT_MISMATCH:
    code = "CL_IMAGE_FORMAT_MISMATCH";
    break;
  case CL_IMAGE_FORMAT_NOT_SUPPORTED:
    code = "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    break;
  case CL_BUILD_PROGRAM_FAILURE:
    code = "CL_BUILD_PROGRAM_FAILURE";
    break;
  case CL_MAP_FAILURE:
    code = "CL_MAP_FAILURE";
    break;
  case CL_MISALIGNED_SUB_BUFFER_OFFSET:
    code = "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    break;
  case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
    code = "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    break;
  case CL_INVALID_VALUE:
    code = "CL_INVALID_VALUE";
    break;
  case CL_INVALID_DEVICE_TYPE:
    code = "CL_INVALID_DEVICE_TYPE";
    break;
  case CL_INVALID_PLATFORM:
    code = "CL_INVALID_PLATFORM";
    break;
  case CL_INVALID_DEVICE:
    code = "CL_INVALID_DEVICE";
    break;
  case CL_INVALID_CONTEXT:
    code = "CL_INVALID_CONTEXT";
    break;
  case CL_INVALID_QUEUE_PROPERTIES:
    code = "CL_INVALID_QUEUE_PROPERTIES";
    break;
  case CL_INVALID_COMMAND_QUEUE:
    code = "CL_INVALID_COMMAND_QUEUE";
    break;
  case CL_INVALID_HOST_PTR:
    code = "CL_INVALID_HOST_PTR";
    break;
  case CL_INVALID_MEM_OBJECT:
    code = "CL_INVALID_MEM_OBJECT";
    break;
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
    code = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    break;
  case CL_INVALID_IMAGE_SIZE:
    code = "CL_INVALID_IMAGE_SIZE";
    break;
  case CL_INVALID_SAMPLER:
    code = "CL_INVALID_SAMPLER";
    break;
  case CL_INVALID_BINARY:
    code = "CL_INVALID_BINARY";
    break;
  case CL_INVALID_BUILD_OPTIONS:
    code = "CL_INVALID_BUILD_OPTIONS";
    break;
  case CL_INVALID_PROGRAM:
    code = "CL_INVALID_PROGRAM";
    break;
  case CL_INVALID_PROGRAM_EXECUTABLE:
    code = "CL_INVALID_PROGRAM_EXECUTABLE";
    break;
  case CL_INVALID_KERNEL_NAME:
    code = "CL_INVALID_KERNEL_NAME";
    break;
  case CL_INVALID_KERNEL_DEFINITION:
    code = "CL_INVALID_KERNEL_DEFINITION";
    break;
  case CL_INVALID_KERNEL:
    code = "CL_INVALID_KERNEL";
    break;
  case CL_INVALID_ARG_INDEX:
    code = "CL_INVALID_ARG_INDEX";
    break;
  case CL_INVALID_ARG_VALUE:
    code = "CL_INVALID_ARG_VALUE";
    break;
  case CL_INVALID_ARG_SIZE:
    code = "CL_INVALID_ARG_SIZE";
    break;
  case CL_INVALID_KERNEL_ARGS:
    code = "CL_INVALID_KERNEL_ARGS";
    break;
  case CL_INVALID_WORK_DIMENSION:
    code = "CL_INVALID_WORK_DIMENSION";
    break;
  case CL_INVALID_WORK_GROUP_SIZE:
    code = "CL_INVALID_WORK_GROUP_SIZE";
    break;
  case CL_INVALID_WORK_ITEM_SIZE:
    code = "CL_INVALID_WORK_ITEM_SIZE";
    break;
  case CL_INVALID_GLOBAL_OFFSET:
    code = "CL_INVALID_GLOBAL_OFFSET";
    break;
  case CL_INVALID_EVENT_WAIT_LIST:
    code = "CL_INVALID_EVENT_WAIT_LIST";
    break;
  case CL_INVALID_EVENT:
    code = "CL_INVALID_EVENT";
    break;
  case CL_INVALID_OPERATION:
    code = "CL_INVALID_OPERATION";
    break;
  case CL_INVALID_GL_OBJECT:
    code = "CL_INVALID_GL_OBJECT";
    break;
  case CL_INVALID_BUFFER_SIZE:
    code = "CL_INVALID_BUFFER_SIZE";
    break;
  case CL_INVALID_MIP_LEVEL:
    code = "CL_INVALID_MIP_LEVEL";
    break;
  case CL_INVALID_GLOBAL_WORK_SIZE:
    code = "CL_INVALID_GLOBAL_WORK_SIZE";
    break;
  case CL_INVALID_PROPERTY:
    code = "CL_INVALID_PROPERTY";
  default:
    code = "CL_UNKNOWN_ERROR_CODE";
  }
  fprintf(stderr, "<rtl> Error Code: %s\n", code);
}

void deviceDetails(cl_device_id id, cl_device_info param_name,
                   const char *param_str) {
  cl_uint i;
  cl_int status = 0;
  size_t param_size = 0;

  status = clGetDeviceInfo(id, param_name, 0, NULL, &param_size);
  if (status != CL_SUCCESS) {
    fprintf(stderr, "<rtl> Unable to obtain device info for %s.\n", param_str);
    clErrorCode(status);
    return;
  }

  /* the cl_device_info are preprocessor directives defined in cl.h */
  switch (param_name) {
  case CL_DEVICE_TYPE: {
    cl_device_type *devType =
        (cl_device_type *)alloca(sizeof(cl_device_type) * param_size);
    status = clGetDeviceInfo(id, param_name, param_size, devType, NULL);
    if (status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Unable to obtain device info for %s.\n",
              param_str);
      clErrorCode(status);
      return;
    }
    switch (*devType) {
    case CL_DEVICE_TYPE_CPU:
      printf("\tDevice is CPU\n");
      break;
    case CL_DEVICE_TYPE_GPU:
      printf("\tDevice is GPU\n");
      break;
    case CL_DEVICE_TYPE_ACCELERATOR:
      printf("\tDevice is Accelerator\n");
      break;
    case CL_DEVICE_TYPE_DEFAULT:
      printf("\tDevice is Unknown\n");
      break;
    }
  } break;
  case CL_DEVICE_VENDOR_ID:
  case CL_DEVICE_MAX_COMPUTE_UNITS:
  case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: {
    cl_uint *ret = (cl_uint *)alloca(sizeof(cl_uint) * param_size);
    status = clGetDeviceInfo(id, param_name, param_size, ret, NULL);
    if (status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Unable to obtain device info for %s.\n",
              param_str);
      clErrorCode(status);
      return;
    }
    switch (param_name) {
    case CL_DEVICE_VENDOR_ID:
      printf("\tVendor ID: 0x%x\n", *ret);
      break;
    case CL_DEVICE_MAX_COMPUTE_UNITS:
      printf("\tMaximum number of parallel compute units: %u\n", *ret);
      break;
    case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
      printf("\tMaximum dimensions for global/local work-item IDs: %u\n", *ret);
      break;
    }
  } break;
  case CL_DEVICE_MAX_WORK_ITEM_SIZES: {
    cl_uint maxWIDimensions;
    size_t *ret = (size_t *)alloca(sizeof(size_t) * param_size);
    status = clGetDeviceInfo(id, param_name, param_size, ret, NULL);

    status = clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                             sizeof(cl_uint), &maxWIDimensions, NULL);
    if (status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Unable to obtain device info for %s.\n",
              param_str);
      clErrorCode(status);
      return;
    }
    printf("\tMaximum number of work-items in each dimension: ( ");
    for (i = 0; i < maxWIDimensions; ++i) {
      printf("%zu ", ret[i]);
    }
    printf(" )\n");
  } break;
  case CL_DEVICE_MAX_WORK_GROUP_SIZE: {
    size_t *ret = (size_t *)alloca(sizeof(size_t) * param_size);
    status = clGetDeviceInfo(id, param_name, param_size, ret, NULL);
    if (status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Unable to obtain device info for %s.\n",
              param_str);
      clErrorCode(status);
      return;
    }
    printf("\tMaximum number of work-items in a work-group: %zu\n", *ret);
  } break;
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT: {
    cl_uint *preferred_size = (cl_uint *)alloca(sizeof(cl_uint) * param_size);
    status = clGetDeviceInfo(id, param_name, param_size, preferred_size, NULL);
    if (status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Unable to obtain device info for %s.\n",
              param_str);
      clErrorCode(status);
      return;
    }
    printf("\tPreferred vector width size for float: %d\n", (*preferred_size));
  } break;
  case CL_DEVICE_NAME:
  case CL_DEVICE_VENDOR: {
    char data[48];
    status = clGetDeviceInfo(id, param_name, param_size, data, NULL);
    if (status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Unable to obtain device info for %s.\n",
              param_str);
      clErrorCode(status);
      return;
    }
    switch (param_name) {
    case CL_DEVICE_NAME:
      printf("\tDevice name is %s\n", data);
      break;
    case CL_DEVICE_VENDOR:
      printf("\tDevice vendor is %s\n", data);
      break;
    }
  } break;
  case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE: {
    cl_uint *size = (cl_uint *)alloca(sizeof(cl_uint) * param_size);
    status = clGetDeviceInfo(id, param_name, param_size, size, NULL);
    if (status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Unable to obtain device info for %s.\n",
              param_str);
      clErrorCode(status);
      return;
    }
    printf("\tDevice global cacheline size: %d bytes\n", (*size));
  } break;
  case CL_DEVICE_GLOBAL_MEM_SIZE:
  case CL_DEVICE_MAX_MEM_ALLOC_SIZE: {
    cl_ulong *size = (cl_ulong *)alloca(sizeof(cl_ulong) * param_size);
    status = clGetDeviceInfo(id, param_name, param_size, size, NULL);
    if (status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Unable to obtain device info for %s.\n",
              param_str);
      clErrorCode(status);
      return;
    }
    switch (param_name) {
    case CL_DEVICE_GLOBAL_MEM_SIZE:
      printf("\tDevice global mem: %llu mega-bytes\n", (*size) >> 20);
      break;
    case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
      printf("\tDevice max memory allocation: %llu mega-bytes\n",
             (*size) >> 20);
      break;
    }
  } break;
  }
}

/// Class containing all the device information.
class RTLDeviceInfoTy {

public:
  std::vector<FuncOrGblEntryTy> FuncGblEntries;
  std::vector<cl_device_id> _device;
  std::vector<cl_context> _context;
  std::vector<cl_command_queue> _cmd_queue;
  std::vector<cl_platform_id> _platform;
  cl_uint _ndevices;
  cl_int _status;

  int _spir_support;
  int _gpu_present;
  int _cpu_present;
  int _verbose;
  int _profile;

  cl_event _global_event;

  enum RtlModeOptions { RTL_none, RTL_verbose, RTL_profile, RTL_all };

  ///
  /// This constructor initialize the opencl device and
  /// hold all device information. rtlmode defines the
  /// kind of information will be shown to the user
  ///
  RTLDeviceInfoTy(int32_t rtlmode) {

    cl_uint nplatforms;
    cl_uint ndev;
    cl_uint i;
    cl_uint idx;

    _verbose = (rtlmode == RTL_verbose) || (rtlmode == RTL_all);
    _profile = (rtlmode == RTL_profile) || (rtlmode == RTL_all);

    if (_device == NULL) {
      // Fetch the main Platform (the first one)
      _status = clGetPlatformIDs(1, &_platform, &nplatforms);
      if (_status != CL_SUCCESS || nplatforms <= 0) {
        fprintf(stderr, "<rtl> Failed to find any OpenCL platform.\n");
        clErrorCode(_status);
        exit(1);
      }

      // Get the name of the plataform
      char platformName[100];
      memset(platformName, '\0', 100);
      _status = clGetPlatformInfo(_platform[0], CL_PLATFORM_NAME,
                                  sizeof(platformName), platformName, NULL);

      // Check if the plataform supports spir
      char extension_string[1024];
      memset(extension_string, '\0', 1024);
      _status =
          clGetPlatformInfo(_platform[0], CL_PLATFORM_EXTENSIONS,
                            sizeof(extension_string), extension_string, NULL);
      char *extStringStart = strstr(extension_string, "cl_khr_spir");
      if (extStringStart != 0) {
        _spir_support = 1;
      } else {
        if (_verbose)
          printf(
              "<rtl> This platform does not support cl_khr_spir extension.\n");
        _spir_support = 0;
      }

      // Fetch the device list for this platform
      _status =
          clGetDeviceIDs(_platform[0], CL_DEVICE_TYPE_ALL, 0, NULL, &_ndevices);
      if (_status != CL_SUCCESS) {
        fprintf(stderr, "<rtl> Failed to find any OpenCL device.\n");
        clErrorCode(_status);
        exit(1);
      }
      if (_verbose)
        printf("<rtl> Find %u devices on platform.\n", _ndevices);

      idx = 0;
      // Fetch the CPU device list for this platform. Note that the allocation
      // of handlers is done after test because device 0 is reserved to CPU
      _status =
          clGetDeviceIDs(_platform[0], CL_DEVICE_TYPE_CPU, 0, NULL, &ndev);
      if (_status != CL_SUCCESS) {
        _ndevices += 1;
        _cpu_present = 0;
      }
      _gpu_present = 0;

      _device.resize(_ndevices);
      _context.resize(_ndevices);
      _cmd_queue.resize(_ndevices);

      if (_status == CL_SUCCESS) {
        if (_verbose) {
          printf(" The CPU was handled on device id %d.\n", idx);
        }
        _status = clGetDeviceIDs(_platform[0], CL_DEVICE_TYPE_CPU, 1,
                                 &_device[idx], NULL);
        if (_status != CL_SUCCESS) {
          fprintf(stderr, "<rtl> Failed to create CPU device id .\n");
          clErrorCode(_status);
        } else {
          _cpu_present = 1;
        }
      }

      idx += 1;
      if (_ndevices > idx) {
        // Try to fetch the GPU device list for this platform
        _status =
            clGetDeviceIDs(_platform[0], CL_DEVICE_TYPE_GPU, 0, NULL, &ndev);
        if (_status == CL_SUCCESS) {
          if (_verbose) {
            printf("<rtl> Find %u GPU device(s). ", ndev);
            printf("GPU(s) was handled on device(s) id(s) starting with %d.\n",
                   idx);
          }
          _status = clGetDeviceIDs(_platform[0], CL_DEVICE_TYPE_GPU, ndev,
                                   &_device[idx], NULL);
          if (_status != CL_SUCCESS) {
            fprintf(stderr, "<rtl> Failed to create GPU device id(s) .\n");
            clErrorCode(_status);
          } else {
            _gpu_present = 1;
          }
        }
      }

      idx += ndev;
      if (_ndevices > idx) {
        // Fetch all accelerator devices for this platform
        _status = clGetDeviceIDs(_platform[0], CL_DEVICE_TYPE_ACCELERATOR, 0,
                                 NULL, &ndev);
        if (_status == CL_SUCCESS) {
          if (_verbose) {
            printf("<rtl> Find %u ACC device(s). ", ndev);
            printf("Each accelerator device was handled on device id starting "
                   "with %d.\n",
                   idx);
          }
          _status = clGetDeviceIDs(_platform[0], CL_DEVICE_TYPE_ACCELERATOR,
                                   ndev, &_device[idx], NULL);
          if (_status != CL_SUCCESS) {
            fprintf(stderr,
                    "<rtl> Failed to create any accelerator device id .\n");
            clErrorCode(_status);
          }
        }
      }

      idx += ndev;
      if (_ndevices > idx) {
        // Fetch all default devices for this platform
        _status = clGetDeviceIDs(_platform[0], CL_DEVICE_TYPE_DEFAULT, 0, NULL,
                                 &ndev);
        if (_status == CL_SUCCESS) {
          if (_verbose) {
            printf("<rtl> Find %u unknown device(s). ", ndev);
            printf("Each unknown device was handled on device id starting with "
                   "%d.\n",
                   idx);
          }
          _status = clGetDeviceIDs(_platform[0], CL_DEVICE_TYPE_DEFAULT, ndev,
                                   &_device[idx], NULL);
          if (_status != CL_SUCCESS) {
            fprintf(stderr, "<rtl> Failed to create any unknown device id .\n");
            clErrorCode(_status);
          }
        }
      }
    }
  };

  ~RTLDeviceInfoTy() {}

  // Profile
  void _cl_profile(const char *str, int32_t device_id) {
    cl_ulong time_start;
    cl_ulong time_end;
    cl_ulong time_elapsed;

    if (!_profile) {
      return;
    }

    _status = clFinish(_cmd_queue[device_id]);
    if (_status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> unable to finish command queue.\n");
      clErrorCode(_status);
      return;
    }

    _status = clGetEventProfilingInfo(_global_event, CL_PROFILING_COMMAND_START,
                                      sizeof(cl_ulong), &time_start, NULL);
    if (_status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> unable to start profile.\n");
      clErrorCode(_status);
      return;
    }

    _status = clGetEventProfilingInfo(_global_event, CL_PROFILING_COMMAND_END,
                                      sizeof(cl_ulong), &time_end, NULL);
    if (_status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> unable to finish profile.\n");
      clErrorCode(_status);
      return;
    }

    time_elapsed = time_end - time_start;
    printf("<rtl><profile> %s = %llu ns\n", str, time_elapsed);
  }

  // Fix this in the future: obtain rtlmode from compiling parameters
  static RTLDeviceInfoTy DeviceInfo(RTL_verbose);

#ifdef __cplusplus
  extern "C" {
#endif

  // Return an integer different from zero if the provided device image can be
  // supported by the runtime. The functionality is similar to comparing the
  // result of __tgt__rtl__load__binary to NULL. However, this is meant to be a
  // lightweight query to determine if the RTL is suitable for an image without
  // having to load the library, which can be expensive.
  int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *image) { return true; }

  // Return the number of available devices of the type supported by the
  // target RTL.
  int32_t __tgt_rtl_number_of_devices() { return DeviceInfo._ndevices; }

  // Initialize the specified device. In case of success return 0; otherwise
  // return an error code.
  int32_t __tgt_rtl_init_device(int32_t device_id) {
    cl_int _status;

    if (DeviceInfo._verbose && DeviceInfo._device[device_id] != NULL) {
      printf("<rtl> Retrieve information about device %u:\n", device_id);
      deviceDetails(device_id, CL_DEVICE_TYPE, "CL_DEVICE_TYPE");
      deviceDetails(device_id, CL_DEVICE_NAME, "CL_DEVICE_NAME");
      deviceDetails(device_id, CL_DEVICE_VENDOR, "CL_DEVICE_VENDOR");
      deviceDetails(device_id, CL_DEVICE_VENDOR_ID, "CL_DEVICE_VENDOR_ID");
      deviceDetails(device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                    "CL_DEVICE_MAX_MEM_ALLOC_SIZE");
      deviceDetails(device_id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,
                    "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE");
      deviceDetails(device_id, CL_DEVICE_GLOBAL_MEM_SIZE,
                    "CL_DEVICE_GLOBAL_MEM_SIZE");
      deviceDetails(device_id, CL_DEVICE_MAX_COMPUTE_UNITS,
                    "CL_DEVICE_MAX_COMPUTE_UNITS");
      deviceDetails(device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                    "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS");
      deviceDetails(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES,
                    "CL_DEVICE_MAX_WORK_ITEM_SIZES");
      deviceDetails(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                    "CL_DEVICE_MAX_WORK_GROUP_SIZE");
      deviceDetails(device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
                    "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT");
    }

    if (DeviceInfo._device[device_id] != NULL) {
      // Create one OpenCL context for device in the platform
      DeviceInfo._context[device_id] = clCreateContext(
          NULL, 1, &DeviceInfo._device[device_id], NULL, NULL, &_status);
      if (_status != CL_SUCCESS) {
        fprintf(stderr, "<rtl> Failed to create context for device %u.\n",
                device_id);
        clErrorCode(_status);
      }

      cl_command_queue_properties properties;
      // enabling profile if set (i.e. rtlmode == profile or all)
      properties = DeviceInfo._profile ? CL_QUEUE_PROFILING_ENABLE : 0;

      // Create a command queue for context to communicate with the device
      DeviceInfo._cmd_queue[device_id] = clCreateCommandQueue(
          DeviceInfo._context[device_id], DeviceInfo._device[device_id],
          properties, &_status);
      if (_status != CL_SUCCESS) {
        fprintf(stderr, "<rtl> Failed to create commandQueue for device %u.\n",
                device_id);
        clErrorCode(_status);
        return OFFLOAD_FAIL;
      }
      return OFFLOAD_SUCCESS;
    }
    return OFFLOAD_FAIL;
  }

  // Pass an executable image section described by image to the specified
  // device and prepare an address table of target entities. In case of error,
  // return NULL. Otherwise, return a pointer to the built address table.
  // Individual entries in the table may also be NULL, when the corresponding
  // offload region is not supported on the target device.
  __tgt_target_table *__tgt_rtl_load_binary(int32_t device_id,
                                            __tgt_device_image *image) {

    cl_int errNum = 0;
    cl_int status = 0;
    cl_program program;

    size_t ImageSize = (size_t)image->ImageEnd - (size_t)image->ImageStart;
    size_t NumEntries = (size_t)(image->EntriesEnd - image->EntriesBegin);

    program = clCreateProgramWithBinary(
        DeviceInfo._context[device_id], 1, &DeviceInfo._device[device_id],
        &ImageSize, (const unsigned char **)&image, &binaryStatus, &errNum);

    if (errNum != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Error loading binary for device %d. Status %d.\n",
              device_id, errNum);
      return NULL;
    }
    if (binaryStatus != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Invalid binary for device %d. Status %d\n",
              device_id, status);
      return NULL;
    }

    const char *flags = NULL;
    if (DeviceInfo._spir_support) {
      flags = "-x spir";
    }
    errNum = clBuildProgram(program, 1, &DeviceInfo._device[device_id], flags,
                            NULL, NULL);

    if (errNum != CL_SUCCESS) {
      // Determine the reason for the error
      char buildLog[16384];
      clGetProgramBuildInfo(program, DeviceInfo._device[device_id],
                            CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog,
                            NULL);
      if (!DeviceInfo._spir_support) {
        fprintf(
            stderr,
            "<rtl> %s: This platform does not support cl_khr_spir extension!\n",
            buildLog);
      } else {
        fprintf(stderr, "<rtl> Error building program : %s\n", buildLog);
      }
      clReleaseProgram(program);
      return NULL;
    }

    // create kernel and target entries
    DeviceInfo.FuncGblEntries[device_id].Entries.resize(NumEntries);
    DeviceInfo.FuncGblEntries[device_id].Kernels.resize(NumEntries);
    std::vector<__tgt_offload_entry> &entries =
        DeviceInfo.FuncGblEntries[device_id].Entries;
    std::vector<cl_kernel> &kernels =
        DeviceInfo.FuncGblEntries[device_id].Kernels;
    for (int i = 0; i < NumEntries; i++) {
      char *name = image->EntriesBegin[i].name;
      kernels[i] = clCreateKernel(program, name, &status);
      if (status != 0) {
        fprintf(stderr, "<rtl> Failed to create opencl kernel object %s.\n",
                name);
        return NULL;
      }
      entries[i].addr = &kernels[i];
      entries[i].name = name;
    }

    __tgt_target_table &table = DeviceInfo.FuncGblEntries[device_id].Table;
    table.EntriesBegin = &(entries[0]);
    table.EntriesEnd = &(entries[entries.size()]);
    return &table;
  }

  // Allocate data on the particular target device, of the specified size.
  // HostPtr is a address of the host data the allocated target data
  // will be associated with (HostPtr may be NULL if it is not known at
  // allocation time, like for example it would be for target data that
  // is allocated by omp_target_alloc() API). Return address of the
  // allocated data on the target that will be used by libomptarget.so to
  // initialize the target data mapping structures. These addresses are
  // used to generate a table of target variables to pass to
  // __tgt_rtl_run_region(). The __tgt_rtl_data_alloc() returns NULL in
  // case an error occurred on the target device.
  void *__tgt_rtl_data_alloc(int32_t device_id, int64_t size, void *hst_ptr) {
    cl_int status = 0;
    cl_mem mem = clCreateBuffer(DeviceInfo._context[device_id],
                                CL_MEM_READ_WRITE, size, NULL, &status);
    if (_status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Failed creating a %lu buffer on device %d.\n",
              size, device_id);
      clErrorCode(_status);
      return nullptr;
    }
    return mem;
  }

  // Pass the data content to the target device using the target address.
  // In case of success, return zero. Otherwise, return an error code.
  int32_t __tgt_rtl_data_submit(int32_t device_id, void *tgt_ptr, void *hst_ptr,
                                int64_t size) {
    cl_int status = clEnqueueWriteBuffer(
        DeviceInfo._cmd_queue[device_id], (cl_mem)tgt_ptr, CL_TRUE, 0, size,
        hst_ptr, 0, nullptr, (DeviceInfo._profile) ? &_global_event : nullptr);

    if (status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Failed writing %lu bytes into buffer %d.\n", size,
              device_id);
      clErrorCode(status);
      return OFFLOAD_FAIL;
    }

    if (DeviceInfo._profile) {
      DeviceInfo._cl_profile("__tgt_rtl_data_submit", device_id);
    }
    if (DeviceInfo._verbose) {
      printf("<rtl> Creating read-write buffer %d of size: %lu\n", device_id,
             size);
    }
    return OFFLOAD_SUCCESS;
  }

  // Retrieve the data content from the target device using its address.
  // In case of success, return zero. Otherwise, return an error code.
  int32_t __tgt_rtl_data_retrieve(int32_t device_id, void *hst_ptr,
                                  void *tgt_ptr, int64_t size) {
    cl_int status =
        clEnqueueReadBuffer(DeviceInfo.Queues[device_id], (cl_mem)tgt_ptr,
                            CL_TRUE, 0, size, hst_ptr, 0, nullptr, nullptr);
    if (status != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Failed reading %lu bytes from buffer %d.\n", size,
              device_id);
      clErrorCode(status);
      return OFFLOAD_FAIL;
    }

    if (DeviceInfo._profile) {
      DeviceInfo._cl_profile("__tgt_rtl_data_retrieve", device_id);
    }
    if (DeviceInfo._verbose) {
      printf("<rtl> Creating read-write buffer %d of size: %lu\n", device_id,
             size);
    }
    return OFFLOAD_SUCCESS;
  }

  // De-allocate the data referenced by target ptr on the device. In case of
  // success, return zero. Otherwise, return an error code.
  int32_t __tgt_rtl_data_delete(int32_t device_id, void *tgt_ptr) {
    if (cl_int status = clReleaseMemObject((cl_mem)tgt_ptr) != CL_SUCCESS) {
      fprintf(stderr, "<rtl> Failed releasing buffer %d.\n", device_id);
      clErrorCode(status);
      return OFFLOAD_FAIL;
    }
    return OFFLOAD_SUCCESS;
  }

  // Transfer control to the offloaded entry Entry on the target device.
  // Args and Offsets are arrays of NumArgs size of target addresses and
  // offsets. An offset should be added to the target address before passing it
  // to the outlined function on device side. In case of success, return zero.
  // Otherwise, return an error code.
  int32_t __tgt_rtl_run_target_team_region(int32_t device_id,
                                           void *tgt_entry_ptr, void **tgt_args,
                                           ptrdiff_t *tgt_offsets,
                                           int32_t arg_num, int32_t team_num,
                                           int32_t thread_limit,
                                           uint64_t loop_tripcount) {

    cl_int status;
    cl_kernel *kernel = static_cast<cl_kernel *>(tgt_entry_ptr);

    if (DeviceInfo._verbose) {
      char buffer[128];
      clGetKernelInfo(*kernel, CL_KERNEL_FUNCTION_NAME, 128, buffer, nullptr);
      cl_uint n;
      clGetKernelInfo(*kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), &n,
                      nullptr);
      printf("<rtl> number of kernel parameters: %d\n", n);
      printf("<rtl> number of arguments: %d\n", arg_num);
    }

    // set kernel args
    std::vector<void *> ptrs(arg_num - 1);
    for (int32_t i = 0; i < arg_num - 1; ++i) {
      ptrs[i] = (void *)((intptr_t)tgt_args[i] + tgt_offsets[i]);
      status = clSetKernelArg(*kernel, i, sizeof(cl_mem), &ptrs[i]);
      if (status != CL_SUCCESS) {
        fprintf(stderr,
                "OpenCL Error: Failed to set kernel arg %d for %s: %d\n", i,
                buffer, status);
        return OFFLOAD_FAIL;
      } else if (DeviceInfo._verbose) {
        printf("<rtl> OpenCL kernel arg %d set successfully.\n", i);
      }
    }

    // teams use (for now) only one dimmension
    cl_uint wd = 1;
    
    // calculate number of threads in each team:
    size_t *global_size;
    size_t *local_size;
    size_t wsize0, block0;

    global_size = (size_t *)calloc(3, sizeof(size_t));
    local_size = (size_t *)calloc(3, sizeof(size_t));

    if (thread_limit) {
      block0 = (size_t)thread_limit;
    } else {
      clGetKernelWorkGroupInfo(*kernel, DeviceInfo.deviceIDs[device_id],
                               CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                               sizeof(size_t), &block0, nullptr);
    }
    if (team_num) {
      wsize0 = (size_t)team_num;
    } else {
      wsize0 = block0 * DeviceInfo.maxWorkGroups[device_id] * 8;
    }

    global_size[0] = wsize0 * block0;
    local_size[0] = block0;
    if (_verbose) {
      printf("<rtl> Work Group was configured to:\n");
      printf("\tX-size=%d\t,Local X-WGS=%lu\t,Global X-WGS=%lu\n", wsize0,
             local_size[0], global_size[0]);
    }

    status = clEnqueueNDRangeKernel(
        DeviceInfo._cmd_queue[device_id], *kernel,
        wd,          // number of dimmensions
        nullptr,     // global_work_offset
        global_size, // global_work_size
        local_size,  // local_work_size
        0,           // num_events_in_wait_list
        nullptr,     // event_wait_list
        (DeviceInfo._profile) ? &DeviceInfo._global_event : nullptr // event
    );

    if (status == CL_SUCCESS) {
      if (DeviceInfo._profile) {
        DeviceInfo._cl_profile("__tgt_rtl_run_target_team_region", device_id);
      }
      if (_verbose) {
        printf("<rtl> Device %d has been running successfully.\n", device_id);
      }
      return OFFLOAD_SUCCESS;
    } else {
      if (status == CL_INVALID_WORK_DIMENSION)
        fprintf(stderr,
                "<rtl> Error executing kernel. Number of dimmensions is "
                "not a valid value.\n");
      else if (status == CL_INVALID_GLOBAL_WORK_SIZE)
        fprintf(stderr,
                "<rtl> Error executing kernel. Global Work Size is NULL or "
                "exceeded valid range.\n");
      else if (status == CL_INVALID_WORK_GROUP_SIZE)
        fprintf(stderr,
                "<rtl> Error executing kernel. Local Work Size does not "
                "match the Work Group size.\n");
      else if (status == CL_INVALID_WORK_ITEM_SIZE)
        fprintf(stderr,
                "<rtl> Error executing kernel. The number of work-items is "
                "greater than Max Work-items.\n");
      else
        fprintf(stderr, "<rtl> Error executing kernel on device %d\n", _clid);
      clErrorCode(status);
      return OFFLOAD_FAIL;
    }
  }

  // Similar to __tgt_rtl_run_target_region, but additionally specify the
  // number of teams to be created and a number of threads in each team.
  int32_t __tgt_rtl_run_target_region(int32_t device_id, void *tgt_entry_ptr,
                                      void **tgt_args, ptrdiff_t *tgt_offsets,
                                      int32_t arg_num) {

    cl_int status;
    cl_kernel *kernel = static_cast<cl_kernel *>(tgt_entry_ptr);

    if (DeviceInfo._verbose) {
      char buffer[128];
      clGetKernelInfo(*kernel, CL_KERNEL_FUNCTION_NAME, 128, buffer, nullptr);
      cl_uint n;
      clGetKernelInfo(*kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), &n,
                      nullptr);
      printf("<rtl> number of kernel parameters: %d\n", n);
      printf("<rtl> number of arguments: %d\n", arg_num);
    }

    // set kernel args
    std::vector<void *> ptrs(arg_num - 1);
    for (int32_t i = 0; i < arg_num - 1; ++i) {
      ptrs[i] = (void *)((intptr_t)tgt_args[i] + tgt_offsets[i]);
      status = clSetKernelArg(*kernel, i, sizeof(cl_mem), &ptrs[i]);
      if (status != CL_SUCCESS) {
        fprintf(stderr,
                "OpenCL Error: Failed to set kernel arg %d for %s: %d\n", i,
                buffer, status);
        return OFFLOAD_FAIL;
      } else if (DeviceInfo._verbose) {
        printf("<rtl> OpenCL kernel arg %d set successfully.\n", i);
      }
    }

    // TODO: We need to seek the global & local work sizes
    int wsize0, wsize1, wsize2;
    int block0, block1, block2;
    int dim;

    size_t *global_size;
    size_t *local_size;
    cl_uint wd = dim;

    global_size = (size_t *)calloc(3, sizeof(size_t));
    local_size = (size_t *)calloc(3, sizeof(size_t));

    global_size[0] = (size_t)(wsize0 * block0);
    local_size[0] = (size_t)block0;

    if (dim == 2) {
      global_size[1] = (size_t)(wsize1 * block1);
      local_size[1] = block1;
      if (block2 == 0) {
        global_size[2] = 0;
        local_size[2] = 0;
      } else {
        global_size[2] = block2;
        local_size[2] = block2;
        wd = 3;
      }
    } else if (dim == 3) {
      global_size[1] = (size_t)(wsize1 * block1);
      local_size[1] = block1;
      global_size[2] = (size_t)(wsize2 * block2);
      local_size[2] = block2;
    }

    if (DeviceInfo._verbose) {
      printf("<rtl> Work Group was configured to:\n");
      printf("\tX-size=%d\t,Local X-WGS=%lu\t,Global X-WGS=%lu\n", wsize0,
             local_size[0], global_size[0]);
      if (wd >= 2)
        printf("\tY-size=%d\t,Local Y-WGS=%lu\t,Global Y-WGS=%lu\n", wsize1,
               local_size[1], global_size[1]);
      if (wd == 3)
        printf("\tZ-size=%d\t,Local Z-WGS=%lu\t,Global Z-WGS=%lu\n", wsize2,
               local_size[2], global_size[2]);
    }

    status = clEnqueueNDRangeKernel(
        DeviceInfo._cmd_queue[device_id], *kernel,
        wd,          // number of dimmensions
        nullptr,     // global_work_offset
        global_size, // global_work_size
        local_size,  // local_work_size
        0,           // num_events_in_wait_list
        nullptr,     // event_wait_list
        (DeviceInfo._profile) ? &DeviceInfo._global_event : nullptr // event
    );

    if (status == CL_SUCCESS) {
      if (DeviceInfo._profile) {
        DeviceInfo._cl_profile("__tgt_rtl_run_target_team_region", device_id);
      }
      if (_verbose) {
        printf("<rtl> Device %d has been running successfully.\n", device_id);
      }
      return OFFLOAD_SUCCESS;
    } else {
      if (status == CL_INVALID_WORK_DIMENSION)
        fprintf(stderr,
                "<rtl> Error executing kernel. Number of dimmensions is "
                "not a valid value.\n");
      else if (status == CL_INVALID_GLOBAL_WORK_SIZE)
        fprintf(stderr,
                "<rtl> Error executing kernel. Global Work Size is NULL or "
                "exceeded valid range.\n");
      else if (status == CL_INVALID_WORK_GROUP_SIZE)
        fprintf(stderr,
                "<rtl> Error executing kernel. Local Work Size does not "
                "match the Work Group size.\n");
      else if (status == CL_INVALID_WORK_ITEM_SIZE)
        fprintf(stderr,
                "<rtl> Error executing kernel. The number of work-items is "
                "greater than Max Work-items.\n");
      else
        fprintf(stderr, "<rtl> Error executing kernel on device %d\n", _clid);
      clErrorCode(status);
      return OFFLOAD_FAIL;
    }
  }

#ifdef __cplusplus
  }
#endif
