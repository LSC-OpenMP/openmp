//===----RTLs/smartnic/src/rtl.cpp - Target RTLs Implementation --- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// RTL for SMARTNIC machine
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstddef>
#include <list>
#include <string>
#include <vector>
#include <ffi.h>
#include <gelf.h>
#include <link.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "omptarget.h"

#ifndef TARGET_NAME
#define TARGET_NAME SMARTNIC
#endif

#define GETNAME2(name) #name
#define GETNAME(name) GETNAME2(name)
#define DP(...) DEBUGP("Target " GETNAME(TARGET_NAME) " RTL", __VA_ARGS__)

#include "../../common/elf_common.c"

#define NUMBER_OF_DEVICES 1
#define OFFLOADSECTIONNAME ".omp_offloading.entries"

// Utility for retrieving and printing SMARTNIC error string.
#ifdef SMARTNIC_ERROR_REPORT
#define SMARTNIC_ERR_STRING(err)                                               \
  do {                                                                         \
    const char *errStr;                                                        \
    cuGetErrorString(err, &errStr);                                            \
    DP("SMARTNIC error is: %s\n", errStr);                                     \
  } while (0)
#else
#define SMARTNIC_ERR_STRING(err)                                               \
  {}
#endif

/// Array of Dynamic libraries loaded for this target.
struct DynLibTy {
  char *FileName;
  void *Handle;
};

/// Keep entries table per device.
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
};

/// Class to handle socket.
class SocketHandle {
 private:
  int sockfd;
  int portno;
  int conn_status;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  inline void send_ack() {
    if (send(this->sockfd, "ack", 3, 0) < 0) {
      DP("[smartnic] error writing ack to socket\n");
      throw -1;
    }
  }

  inline void get_ack() {
    char buffer[3];

    bzero(buffer, 3);
    if (recv(this->sockfd, buffer, 3, 0) < 0) {
      DP("[smartnic] error reading ack from socket\n");
      throw -1;
    }

    if (buffer[0] != 'a' || buffer[1] != 'c' || buffer[2] != 'k') {
      DP("[smartnic] error not received ack message\n");
      throw -1;
    }

    DP("[smartnic] ack received!\n");
  }

  inline void send_cmd(char cmd) {
    char buffer[1];

    buffer[0] = cmd;

    if (send(this->sockfd, buffer, 1, 0) < 0) {
      DP("[smartnic] error cmd from socket!\n");
      throw -1;
    }

    this->get_ack();
  }

  inline void send_data(void *data, int64_t size) {
    if (send(this->sockfd, data, size, 0) < 0) {
      DP("[smartnic] send data error!\n");
      throw -1;
    }

    this->get_ack();
  }

  inline void recv_data(void *data, int64_t size) {
    ssize_t recv_bytes = 0;

    do {
      recv_bytes += recv(this->sockfd, data + recv_bytes, size, 0);

      if (recv_bytes < 0) {
        DP("[smartnic] recv data error!\n");
        throw -1;
      }
    } while(recv_bytes != size);

    this->send_ack();
  }

 public:
  int conn() {
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
      DP("[smartnic] error - opening the socket!");
      return -1;
    }

    bzero((char *) &this->serv_addr, sizeof(serv_addr));

    inet_pton(AF_INET, "127.0.0.1", &(this->serv_addr.sin_addr));
    this->serv_addr.sin_family = AF_INET;
    this->serv_addr.sin_port = htons(this->portno);

    conn_status = connect(this->sockfd,
                          (struct sockaddr *) &this->serv_addr,
                          sizeof(this->serv_addr));

    return conn_status;
  }

  int write(void *data, int64_t size) {
    int write_status = 1;

    assert(size > 0);

    try {
      this->send_cmd('w');
      this->send_data(reinterpret_cast<void*>(&size), 8);
      this->send_data(data, size);
    } catch (int e) {
      write_status = e;
    }

    return write_status;
  }

  int program(void *data, int64_t size) {
    int program_status = 1;

    assert(size > 0);

    try {
      this->send_cmd('p');
      this->send_data(data, size);
    } catch (int e) {
      program_status = e;
    }

    return program_status;
  }

  int read(void *data, int64_t size) {
    int read_status = 1;

    assert(size > 0);

    try {
      this->send_cmd('r');
      this->send_data(reinterpret_cast<void*>(&size), 8);
      this->recv_data(data, size);
    } catch (int e) {
      read_status = e;
    }

    return read_status;
  }

  void disconnect() {
    this->send_cmd('q');
    this->send_ack();

    close(this->sockfd);
  }

  int get_conn_status() {
    return conn_status;
  }

  SocketHandle() {
    this->portno = 51717;
    this->conn_status = -1;

    this->conn();
  }

  SocketHandle(int portno) {
    this->portno = portno;
    this->conn_status = -1;
  }

  ~SocketHandle() {
    DP("[smartnic] socket closed\n");
    this->send_cmd('q');
    this->send_ack();

    close(this->sockfd);
  }
};

/// Class containing FPGA information
class FPGAInfo {
private:
  SocketHandle *socket_handle;
  char *last_module;
  char *module;

public:

  void set_module(char *module) {
    this->module = (char*) realloc(last_module, sizeof(module));
    memcpy(this->module, module, strlen(module));
  }

  void program_fpga() {
    if (strlen(module) == 0)
      return;

    if (last_module == NULL || strcmp(module, last_module) != 0) {
      last_module = (char*) realloc(last_module, sizeof(module));

      memcpy(last_module, module, strlen(module));

      this->socket_handle->program(reinterpret_cast<void*>(module),
                                   strlen(module) + 1);

      DP("[fpga_info] programming FPGA - %s\n", last_module);
    }
  }

  FPGAInfo(SocketHandle *socket_handle) {
    this->last_module = NULL;
    this->socket_handle = socket_handle;
  }

  ~FPGAInfo() {
    free(this->last_module);
  }
};

/// Class containing all the device information.
class RTLDeviceInfoTy {
  std::vector<FuncOrGblEntryTy> FuncGblEntries;

public:

  std::list<DynLibTy> DynLibs;

  // Record entry point associated with device.
  void createOffloadTable(int32_t device_id, __tgt_offload_entry *begin,
                          __tgt_offload_entry *end) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    E.Table.EntriesBegin = begin;
    E.Table.EntriesEnd = end;
  }

  // Return true if the entry is associated with device.
  bool findOffloadEntry(int32_t device_id, void *addr) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    for (__tgt_offload_entry *i = E.Table.EntriesBegin, *e = E.Table.EntriesEnd;
         i < e; ++i) {
      if (i->addr == addr)
        return true;
    }

    return false;
  }

  // Return the pointer to the target entries table.
  __tgt_target_table *getOffloadEntriesTable(int32_t device_id) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    return &E.Table;
  }

  RTLDeviceInfoTy(int32_t num_devices) { FuncGblEntries.resize(num_devices); }

  ~RTLDeviceInfoTy() {
    // Close dynamic libraries
    for (auto &lib : DynLibs) {
      if (lib.Handle) {
        dlclose(lib.Handle);
        remove(lib.FileName);
      }
    }
  }
};

static RTLDeviceInfoTy DeviceInfo(NUMBER_OF_DEVICES);
static SocketHandle    socket_handle;
static FPGAInfo        fpga_info(&socket_handle);

#ifdef __cplusplus
extern "C" {
#endif

int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *image) {
  uint32_t is_valid_binary = elf_check_machine(image, EM_X86_64, 9001);

  // TODO(ciroc): extract struct information
  if (is_valid_binary) {
    __tgt_configuration *cfg;
    int i;
    char *module = NULL;
    char *img_begin = (char *)image->ImageStart;

    get_tgt_configuration_module(image, &cfg);

    // string constant pointer to .rodata section of elf (img_begin)
    for(i = 0; *(img_begin + (intptr_t)cfg->module + i) != '\0'; i++) {
      module = (char*) realloc(module, (i + 1));
      module[i] = *(img_begin + (intptr_t)cfg->module + i);
    }
    module = (char*) realloc(module, (i + 1));
    module[i] = '\0';

    fpga_info.set_module(module);

    DP("[smartnic] sub_target_id = %d\n", cfg->sub_target_id);
    DP("[smartnic] module = %s\n", module);

    free(module);
  }

  return is_valid_binary;
}

int32_t __tgt_rtl_number_of_devices() {
  return NUMBER_OF_DEVICES;
}

int32_t __tgt_rtl_init_device(int32_t device_id) {

  DP("[smartnic] __tgt_rtl_init_device\n");

  if (socket_handle.get_conn_status() < 0) {
    return OFFLOAD_FAIL;
  }

  fpga_info.program_fpga();

  return OFFLOAD_SUCCESS;
}

__tgt_target_table *__tgt_rtl_load_binary(int32_t device_id,
                                          __tgt_device_image *image) {

  DP("Dev %d: load binary from " DPxMOD " image\n", device_id,
     DPxPTR(image->ImageStart));

  assert(device_id >= 0 && device_id < NUMBER_OF_DEVICES && "bad dev id");

  size_t ImageSize = (size_t)image->ImageEnd - (size_t)image->ImageStart;
  size_t NumEntries = (size_t)(image->EntriesEnd - image->EntriesBegin);
  DP("Expecting to have %zd entries defined.\n", NumEntries);

  // Is the library version incompatible with the header file?
  if (elf_version(EV_CURRENT) == EV_NONE) {
    DP("Incompatible ELF library!\n");
    return NULL;
  }

  // Obtain elf handler
  Elf *e = elf_memory((char *)image->ImageStart, ImageSize);
  if (!e) {
    DP("Unable to get ELF handle: %s!\n", elf_errmsg(-1));
    return NULL;
  }

  if (elf_kind(e) != ELF_K_ELF) {
    DP("Invalid Elf kind!\n");
    elf_end(e);
    return NULL;
  }

  // Find the entries section offset
  Elf_Scn *section = 0;
  Elf64_Off entries_offset = 0;

  size_t shstrndx;

  if (elf_getshdrstrndx(e, &shstrndx)) {
    DP("Unable to get ELF strings index!\n");
    elf_end(e);
    return NULL;
  }

  while ((section = elf_nextscn(e, section))) {
    GElf_Shdr hdr;
    gelf_getshdr(section, &hdr);

    if (!strcmp(elf_strptr(e, shstrndx, hdr.sh_name), OFFLOADSECTIONNAME)) {
      entries_offset = hdr.sh_addr;
      break;
    }
  }

  if (!entries_offset) {
    DP("Entries Section Offset Not Found\n");
    elf_end(e);
    return NULL;
  }

  DP("Offset of entries section is (" DPxMOD ").\n", DPxPTR(entries_offset));

  // load dynamic library and get the entry points. We use the dl library
  // to do the loading of the library, but we could do it directly to avoid the
  // dump to the temporary file.
  //
  // 1) Create tmp file with the library contents.
  // 2) Use dlopen to load the file and dlsym to retrieve the symbols.
  char tmp_name[] = "/tmp/tmpfile_XXXXXX";
  int tmp_fd = mkstemp(tmp_name);

  if (tmp_fd == -1) {
    elf_end(e);
    return NULL;
  }

  FILE *ftmp = fdopen(tmp_fd, "wb");

  if (!ftmp) {
    elf_end(e);
    return NULL;
  }

  fwrite(image->ImageStart, ImageSize, 1, ftmp);
  fclose(ftmp);

  DynLibTy Lib = {tmp_name, dlopen(tmp_name, RTLD_LAZY)};

  if (!Lib.Handle) {
    DP("Target library loading error: %s\n", dlerror());
    elf_end(e);
    return NULL;
  }

  DeviceInfo.DynLibs.push_back(Lib);

  struct link_map *libInfo = (struct link_map *)Lib.Handle;

  // The place where the entries info is loaded is the library base address
  // plus the offset determined from the ELF file.
  Elf64_Addr entries_addr = libInfo->l_addr + entries_offset;

  DP("Pointer to first entry to be loaded is (" DPxMOD ").\n",
      DPxPTR(entries_addr));

  // Table of pointers to all the entries in the target.
  __tgt_offload_entry *entries_table = (__tgt_offload_entry *)entries_addr;

  __tgt_offload_entry *entries_begin = &entries_table[0];
  __tgt_offload_entry *entries_end = entries_begin + NumEntries;

  if (!entries_begin) {
    DP("Can't obtain entries begin\n");
    elf_end(e);
    return NULL;
  }

  DP("Entries table range is (" DPxMOD ")->(" DPxMOD ")\n",
      DPxPTR(entries_begin), DPxPTR(entries_end));
  DeviceInfo.createOffloadTable(device_id, entries_begin, entries_end);

  elf_end(e);

  return DeviceInfo.getOffloadEntriesTable(device_id);
}

void *__tgt_rtl_data_alloc(int32_t device_id, int64_t size) {
  DP("[smartnic] __tgt_rtl_data_alloc\n");
  void *ptr = malloc(size);

  return ptr;
}

int32_t __tgt_rtl_data_submit(int32_t device_id, void *tgt_ptr, void *hst_ptr,
    int64_t size) {

  DP("[smartnic] __tgt_rtl_data_submit: %d\n", size);

  socket_handle.write(hst_ptr, size);

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_retrieve(int32_t device_id, void *hst_ptr, void *tgt_ptr,
    int64_t size) {

  DP("[smartnic] __tgt_rtl_data_retrieve\n");

  socket_handle.read(hst_ptr, size);

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_delete(int32_t device_id, void *tgt_ptr) {

  DP("[smartnic] __tgt_rtl_delete\n");

  free(tgt_ptr);

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_team_region(int32_t device_id, void *tgt_entry_ptr,
    void **tgt_args, int32_t arg_num, int32_t team_num, int32_t thread_limit,
    uint64_t loop_tripcount) {

  DP("[smartnic] __tgt_rtl_run_target_team_region\n");

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_region(int32_t device_id, void *tgt_entry_ptr,
    void **tgt_args, int32_t arg_num) {

  DP("[smartnic] __tgt_rtl_run_target_region\n");

  // use one team and one thread.
  return __tgt_rtl_run_target_team_region(device_id, tgt_entry_ptr, tgt_args,
                                          arg_num, 1, 1, 0);
}

#ifdef __cplusplus
}
#endif
