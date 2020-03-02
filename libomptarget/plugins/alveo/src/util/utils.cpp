#include "utils.h"

struct buffer_t {
  int size;
  int status;
  void* ptr;
  void* fake_ptr;
  cl::Buffer buffer;
};

cl::Device                device;
std::vector<cl::Device>   devices;
std::vector<cl::Platform> platforms;
cl::Kernel                *kernel;
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
  kernel = new cl::Kernel(program, "hardcloud_top");

  return OFFLOAD_SUCCESS;
}

void* data_alloc(int size) {
  static int buffer_id = 0;

  void* ptr;

  // fake pointer
  ptr = (void *) buffer_id++;

  buffer_t tmp = {size, 0 /* unmapped */, nullptr, ptr, (cl::Buffer) NULL};

  buffers.push_back(tmp);

  return ptr;
}

void data_submit(void *tgt_ptr, void *hst_ptr, int64_t size) {
  cl::Buffer buffer(*context, CL_MEM_READ_ONLY, size);

  // set the kernel arguments
  kernel->setArg(narg++, buffer);

  int id = -1;
  for (int i = 0; i < buffers.size(); i++) {
    if (buffers[i].fake_ptr == tgt_ptr) {
      id = i;

      break;
    }
  }

  if (id == -1)
    return;

  // we then need to map our opencl buffers to get the pointers and
  // update the tgt_ptr
  tgt_ptr = q->enqueueMapBuffer(buffer, CL_TRUE, CL_MAP_WRITE, 0, size);

  buffers[id].ptr = tgt_ptr;
  buffers[id].buffer = buffer;
  buffers[id].status = 1; // write

  memcpy(tgt_ptr, hst_ptr, size);

  q->enqueueMigrateMemObjects({buffers[id].buffer}, 0);
}

int run_target() {
  // create read buffers
  for (int i = 0; i < buffers.size(); i++) {
    if (0 == buffers[i].status) {
      cl::Buffer buffer(*context, CL_MEM_WRITE_ONLY, buffers[i].size);

      kernel->setArg(narg++, buffer);

      buffers[i].ptr = q->enqueueMapBuffer(buffer, CL_TRUE, CL_MAP_READ, 0, buffers[i].size);
      buffers[i].buffer = buffer;
      buffers[i].status = 2; // read
    }
  }

  // run
  q->enqueueTask(*kernel);

  // copy read buffers to user space
  for (int i = 0; i < buffers.size(); i++) {
    if (2 == buffers[i].status) {
      q->enqueueMigrateMemObjects({buffers[i].buffer}, CL_MIGRATE_MEM_OBJECT_HOST);
    }
  }

  q->finish();

  return OFFLOAD_SUCCESS;
}

void data_retrieve(void *hst_ptr, void *tgt_ptr, int size) {
  for (auto data : buffers) {
    if (data.fake_ptr == tgt_ptr && 2 == data.status) {
      tgt_ptr = data.ptr;
    }
  }

  memcpy(hst_ptr, tgt_ptr, size);
}

void data_delete(void *tgt_ptr) {
  for (auto data : buffers) {
    q->enqueueUnmapMemObject(data.buffer, data.ptr);
  }
}

void cleanup() {
  q->finish();
}

