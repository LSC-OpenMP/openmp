#include "utils.h"

struct buffer_t {
  cl::Buffer buffer;
  int* ptr;
};

cl::Device                device;
std::vector<cl::Device>   devices;
std::vector<cl::Platform> platforms;
cl::Kernel                kernel;
cl::Context               *context;
cl::CommandQueue          *q;

int narg;

std::vector<buffer_t> buffers;

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
  bool found_device = false;

  //traversing all Platforms To find Xilinx Platform and targeted

  //Device in Xilinx Platform
  cl::Platform::get(&platforms);

  for (size_t i = 0; (i < platforms.size() ) & (found_device == false); i++) {
    cl::Platform platform = platforms[i];
    std::string platformName = platform.getInfo<CL_PLATFORM_NAME>();

    if (platformName == "Xilinx") {
      devices.clear();
      platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);

      if (devices.size()) {
        device = devices[0];
        found_device = true;

        break;
      }
    }
  }

  if (found_device == false) {
    std::cout << "Error: Unable to find Target Device " <<
      device.getInfo<CL_DEVICE_NAME>() << std::endl;

    return OFFLOAD_FAIL;
  }

  // Creating Context and Command Queue for selected device
  context = new cl::Context(device);
  q = new cl::CommandQueue(*context, device, CL_QUEUE_PROFILING_ENABLE);

  // Load xclbin
  const char *extension = ".xclbin";

  char *filename_binary;

  filename_binary = (char*) malloc (strlen(xclbin) + strlen(extension) + 1);

  strcpy(filename_binary, xclbin);
  strcat(filename_binary, extension);

  std::cout << "Loading: '" << filename_binary << "'\n";

  std::ifstream bin_file(filename_binary, std::ifstream::binary);

  bin_file.seekg(0, bin_file.end);
  unsigned nb = bin_file.tellg();
  bin_file.seekg(0, bin_file.beg);

  char *buf = new char [nb];

  bin_file.read(buf, nb);

  // Creating Program from Binary File
  cl::Program::Binaries bins;

  bins.push_back({buf, nb});
  devices.resize(1);

  cl::Program program(*context, devices, bins);

  // This call will get the kernel object from program. A kernel is an
  // OpenCL function that is executed on the FPGA.
  cl::Kernel kernel(program, "hardcloud_top");

  return OFFLOAD_SUCCESS;
}

void* data_alloc(int size) {
  cl::Buffer buffer(*context, CL_MEM_READ_WRITE, size);

  // set the kernel arguments
  kernel.setArg(narg++, buffer);

  // we then need to map our opencl buffers to get the pointers
  int *ptr = (int *) q->enqueueMapBuffer(buffer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size);

  if (!(ptr)) {
    printf("Error: Failed to allocate device memory!\n");
    printf("Test failed\n");
  }

  buffer_t tmp = {buffer, ptr};

  buffers.push_back(tmp);

  return (void*) ptr;
}

void data_submit(void *tgt_ptr, void *hst_ptr, int64_t size) {
  memcpy(tgt_ptr, hst_ptr, size);
}

int run_target() {
  q->enqueueTask(kernel);

  return OFFLOAD_SUCCESS;
}

void data_retrieve(void *hst_ptr, void *tgt_ptr, int size) {

  for (auto data : buffers) {
    if (data.ptr == (int *) tgt_ptr)
      q->enqueueMigrateMemObjects({data.buffer}, CL_MIGRATE_MEM_OBJECT_HOST);
  }

  memcpy(hst_ptr, tgt_ptr, size);

  q->finish();
}

void data_delete(void *tgt_ptr) {
  for (auto data : buffers) {
    q->enqueueUnmapMemObject(data.buffer, data.ptr);
  }
}

void cleanup() {
  q->finish();
}

