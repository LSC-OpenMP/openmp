//===-RTLs/generic-64bit/src/rtl.cpp - Target RTLs Implementation - C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// RTL for generic 64-bit machine
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <ffi.h>
#include <gelf.h>
#ifndef __APPLE__
#include <link.h>
#endif
#include <list>
#include <string>
#include <vector>

#include <chrono>
#include <fstream>
#include <inttypes.h>
#include <thread>
#include <unistd.h>

#include "INIReader.h"
#include "amazon.h"
#include "azure.h"
#include "cloud_compression.h"
#include "cloud_util.h"
#include "generic.h"
#include "local.h"
#include "omptargetplugin.h"
#include "provider.h"
#include "rtl.h"

#ifndef TARGET_NAME
#define TARGET_NAME Cloud
#endif

#ifndef TARGET_ELF_ID
#define TARGET_ELF_ID 0
#endif

#ifdef OMPTARGET_DEBUG
static int DebugLevel = 0;

#define GETNAME2(name) #name
#define GETNAME(name) GETNAME2(name)
#define DP(...)                                                                \
  do {                                                                         \
    if (DebugLevel > 0) {                                                      \
      DEBUGP("Target " GETNAME(TARGET_NAME) " RTL", __VA_ARGS__);              \
    }                                                                          \
  } while (false)
#else // OMPTARGET_DEBUG
#define DP(...)                                                                \
  {}
#endif // OMPTARGET_DEBUG

#include "../../common/elf_common.c"

#define OFFLOADSECTIONNAME ".omp_offloading.entries"

/// Array of Dynamic libraries loaded for this target.
struct DynLibTy {
  char *FileName;
  void *Handle;
};

/// Keep entries table per device.
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
};

static std::vector<struct ProviderListEntry> ExistingProviderList = {
    {"Local", createLocalProvider, "LocalProvider"},
    {"Generic", createGenericProvider, "HdfsProvider"},
    {"Azure", createAzureProvider, "AzureProvider"},
    {"AWS", createAmazonProvider, "AmazonProvider"}};

static std::vector<struct ProviderListEntry> ProviderList;

static char *library_tmpfile = strdup("/tmp/libompcloudXXXXXX");

/// Class containing all the device information.
class RTLDeviceInfoTy {
  std::vector<FuncOrGblEntryTy> FuncGblEntries;

public:
  int NumberOfDevices;

  std::list<DynLibTy> DynLibs;

  std::string working_path;

  INIReader *reader;

  Verbosity verbose;

  uintptr_t CurrentTargetDataPtr;

  std::vector<SparkInfo> SparkClusters;
  std::vector<CloudProvider *> Providers;
  std::vector<std::string> AddressTables;
  std::vector<ElapsedTime> ElapsedTimes;

  std::vector<std::vector<std::thread>> submitting_threads;
  std::vector<std::vector<std::thread>> retrieving_threads;

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

  RTLDeviceInfoTy() {
#ifdef OMPTARGET_DEBUG
    if (char *envStr = getenv("LIBOMPTARGET_DEBUG")) {
      DebugLevel = std::stoi(envStr);
    }
#endif // OMPTARGET_DEBUG

    NumberOfDevices = 0;
    CurrentTargetDataPtr = 1;

    std::string ConfigPath;
    if (char *envStr = getenv(OMPCLOUD_CONF_ENV.c_str())) {
      ConfigPath = std::string(envStr);
    } else {
      DP("There is no configuration file describing cloud devices.\n");
      return;
    }

    reader = new INIReader(ConfigPath);

    if (reader->ParseError() < 0) {
      fprintf(stderr, "ERROR: Path to OmpCloud configuration seems wrong: %s\n",
              ConfigPath.c_str());
      return;
    }

    std::string vmode = reader->Get("Spark", "VerboseMode", "info");
    if (vmode == "debug") {
      verbose = Verbosity::debug;
    } else if (vmode == "info") {
      verbose = Verbosity::info;
    } else if (vmode == "quiet") {
      verbose = Verbosity::quiet;
    } else {
      fprintf(stderr, "Warning: invalid verbose mode\n");
      verbose = Verbosity::info;
    }

    working_path = std::string("/tmp/ompcloud.") + random_string(6);
    std::string cmd("mkdir -p " + working_path);

    DP("%s\n", exec_cmd(cmd.c_str()).c_str());

    // Checking how many providers we have in the configuration file
    for (auto entry : ExistingProviderList) {
      if (reader->HasSection(entry.SectionName)) {
        if (verbose != Verbosity::quiet)
          DP("Provider '%s' detected in configuration file.\n",
             entry.ProviderName.c_str());
        ProviderList.push_back(entry);
        NumberOfDevices++;
      }
    }

    if (ProviderList.size() == 0) {
      if (verbose != Verbosity::quiet) {
        DP("No specific provider detected in configuration file.\n");
        DP("Local provider will be used.\n");
      }
      ProviderList.push_back(ExistingProviderList.front());
      NumberOfDevices++;
    }

    assert(NumberOfDevices == 1 && "Do not support more than 1 device!");

    FuncGblEntries.resize(NumberOfDevices);
    SparkClusters.resize(NumberOfDevices);
    Providers.resize(NumberOfDevices);
    ElapsedTimes = std::vector<ElapsedTime>(NumberOfDevices);
    submitting_threads.resize(NumberOfDevices);
    retrieving_threads.resize(NumberOfDevices);

    for (int i = 0; i < NumberOfDevices; i++) {
      char *tmpname = strdup((working_path + "/addresstable_XXXXXX").c_str());
      int ret = mkstemp(tmpname);
      if (ret < 0) {
        perror("Cannot create address table file");
        exit(EXIT_FAILURE);
      }
      AddressTables.push_back(std::string(tmpname));
    }
  }

  ~RTLDeviceInfoTy() {
    // Close dynamic libraries
    for (auto &lib : DynLibs) {
      if (lib.Handle) {
        dlclose(lib.Handle);
        remove(lib.FileName);
      }
    }

    // Do not do anything if there was no device
    if (NumberOfDevices == 0)
      return;

    if (verbose != Verbosity::quiet)
      for (int i = 0; i < NumberOfDevices; i++) {
        ElapsedTime &timing = ElapsedTimes[i];
        DP("Uploading = %ds\n", timing.UploadTime);
        DP("Downloading = %ds\n", timing.DownloadTime);
        DP("Compression = %ds\n", timing.CompressionTime);
        DP("Decompression = %ds\n", timing.DecompressionTime);
        DP("Execution = %ds\n", timing.SparkExecutionTime);
      }

    if (!SparkClusters[0].KeepTmpFiles)
      remove_directory(working_path.c_str());
  }
};

static RTLDeviceInfoTy DeviceInfo;

#ifdef __cplusplus
extern "C" {
#endif

int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *image) {
  return elf_check_machine(image, EM_X86_64, 9003);
}

int32_t __tgt_rtl_number_of_devices() { return DeviceInfo.NumberOfDevices; }

int32_t __tgt_rtl_init_device(int32_t device_id) {

  if (DeviceInfo.verbose != Verbosity::quiet)
    DP("Initializing device %d\n", device_id);

  // TODO: Check connection to Apache Spark cluster

  SparkMode mode;
  std::string smode =
      DeviceInfo.reader->Get("Spark", "Mode", DEFAULT_SPARK_MODE);
  if (smode == "client") {
    mode = SparkMode::client;
  } else if (smode == "cluster") {
    mode = SparkMode::cluster;
  } else {
    mode = SparkMode::invalid;
  }

  SparkInfo spark{
      DeviceInfo.reader->Get("Spark", "HostName", ""),
      int(DeviceInfo.reader->GetInteger("Spark", "Port", DEFAULT_SPARK_PORT)),
      mode,
      DeviceInfo.reader->Get("Spark", "User", DEFAULT_SPARK_USER),
      DeviceInfo.reader->Get("Spark", "BinPath", ""),
      DeviceInfo.reader->Get("Spark", "Package", DEFAULT_SPARK_PACKAGE),
      DeviceInfo.reader->Get("Spark", "JarPath", DEFAULT_SPARK_JARPATH),
      DeviceInfo.reader->Get("Spark", "AdditionalArgs", ""),
      DeviceInfo.reader->Get("Spark", "WorkingDir", ""),
      DeviceInfo.reader->GetBoolean("Spark", "Compression", true),
      DeviceInfo.reader->Get("Spark", "CompressionFormat",
                             DEFAULT_COMPRESSION_FORMAT),
      DeviceInfo.reader->GetBoolean("Spark", "UseThreads", true),
      DeviceInfo.verbose,
      DeviceInfo.reader->GetBoolean("Spark", "KeepTmpFiles", false),
      DeviceInfo.reader->Get("Spark", "SchedulingSize", "0"),
      DeviceInfo.reader->Get("Spark", "SchedulingKind", "static"),
      1,
  };

  if (spark.WorkingDir.empty()) {
    spark.WorkingDir = "ompcloud.workingdir" + random_string(8);
  }

  // Checking if given WorkingDir and BinPath ends in a slash for path
  // concatenation. If it doesn't, add it!
  if (spark.WorkingDir.back() != '/') {
    spark.WorkingDir += "/";
  }

  if (!spark.BinPath.empty()) {
    if (spark.BinPath.back() != '/') {
      spark.BinPath += "/";
    }

    std::ifstream sparkSubmit((spark.BinPath + "spark-submit").c_str());
    if (!sparkSubmit.good()) {
      DP("ERROR: spark-submit is not accessible\n");
      exit(EXIT_FAILURE);
    }
  }

  if (spark.ServAddress.empty()) {
    // Look for env variable defining Spark hostname
    if (char *enServAddress = std::getenv("OMPCLOUD_SPARK_HOSTNAME")) {
      spark.ServAddress = std::string(enServAddress);
    }
  }

  if (spark.Mode == SparkMode::invalid) {
    DP("ERROR: Invalid Spark execution mode\n");
    exit(EXIT_FAILURE);
  }

  if (DeviceInfo.verbose != Verbosity::quiet) {
    DP("Spark HostName: '%s' - Port: '%d' - User: '%s' - Mode: %s\n",
       spark.ServAddress.c_str(), spark.ServPort, spark.UserName.c_str(),
       smode.c_str());
    DP("Jar: %s - Class: %s - WorkingDir: '%s'\n", spark.JarPath.c_str(),
       spark.Package.c_str(), spark.WorkingDir.c_str());
  }

  // Checking for listed provider. Each device id refers to a provider
  // position in the list
  if (spark.VerboseMode != Verbosity::quiet)
    DP("Creating provider %s\n", ProviderList[device_id].ProviderName.c_str());

  DeviceInfo.SparkClusters[device_id] = spark;
  std::string providerSectionName = ProviderList[device_id].SectionName;
  DeviceInfo.Providers[device_id] =
      ProviderList[device_id].ProviderGenerator(spark);
  DeviceInfo.Providers[device_id]->parse_config(DeviceInfo.reader);
  DeviceInfo.Providers[device_id]->init_device();

  return OFFLOAD_SUCCESS;
}

__tgt_target_table *__tgt_rtl_load_binary(int32_t device_id,
                                          __tgt_device_image *image) {

  DP("Dev %d: load binary from " DPxMOD " image\n", device_id,
     DPxPTR(image->ImageStart));

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
  int tmp_fd = mkstemp(library_tmpfile);

  if (tmp_fd == -1) {
    elf_end(e);
    perror("Error when creating temporary library file");
    return NULL;
  }

  if (DeviceInfo.verbose != Verbosity::quiet)
    DP("Library will be written in %s\n", library_tmpfile);

  FILE *ftmp = fdopen(tmp_fd, "wb");

  if (!ftmp) {
    elf_end(e);
    perror("Error when opening temporary library file");
    return NULL;
  }

  fwrite(image->ImageStart, ImageSize, 1, ftmp);
  fclose(ftmp);

  DynLibTy Lib = {library_tmpfile, dlopen(library_tmpfile, RTLD_LAZY)};

  if (!Lib.Handle) {
    DP("Target library loading error: %s\n", dlerror());
    elf_end(e);
    return NULL;
  }

  DeviceInfo.DynLibs.push_back(Lib);

#ifndef __APPLE__
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
#endif

  return DeviceInfo.getOffloadEntriesTable(device_id);
}

void *__tgt_rtl_data_alloc(int32_t device_id, int64_t size, void *hst_ptr) {
  uintptr_t tgt_ptr = DeviceInfo.CurrentTargetDataPtr++;

  // if (id >= 0) {
  // Write entry in the address table
  std::ofstream ofs(DeviceInfo.AddressTables[device_id], std::ios_base::app);
  ofs << tgt_ptr << ";" << size << ";" << std::endl;
  ofs.close();

  if (DeviceInfo.verbose != Verbosity::quiet)
    DP("Adding '%" PRIxPTR "' of size %ld to the address table\n", tgt_ptr,
       size);
  //}

  return reinterpret_cast<void *>(tgt_ptr);
}

static int32_t data_submit(int32_t device_id, void *hst_ptr, size_t size,
                           int64_t id) {
  double sizeInMB = size / (1024 * 1024);
  if (size > MAX_JAVA_INT) {
    fprintf(stderr,
            "ERROR: Not supported -- size of %" PRId64
            " is larger (%.2fMB) than the maximal size of JVM's bytearrays "
            "(%.2fMB).\n",
            id, sizeInMB, MAX_SIZE_IN_MB);
    exit(EXIT_FAILURE);
  }

  ElapsedTime &timing = DeviceInfo.ElapsedTimes[device_id];

  bool needCompression = DeviceInfo.SparkClusters[device_id].Compression &&
                         size >= MIN_SIZE_COMPRESSION;

  // Since we now need the hdfs file, we create it here
  std::string filename = std::to_string(id);
  std::string host_filepath = DeviceInfo.working_path + "/" + filename;

  size_t sendingSize;
  if (needCompression) {
    auto t_start = std::chrono::high_resolution_clock::now();
    sendingSize =
        compress_to_file(host_filepath, static_cast<char *>(hst_ptr), size);
    auto t_end = std::chrono::high_resolution_clock::now();
    auto t_delay =
        std::chrono::duration_cast<std::chrono::seconds>(t_end - t_start)
            .count();
    timing.CompressionTime_mutex.lock();
    timing.CompressionTime += t_delay;
    timing.CompressionTime_mutex.unlock();

    if (DeviceInfo.verbose != Verbosity::quiet)
      DP("Compressed %.1fMB in %lds\n", sizeInMB, t_delay);

  } else {
    std::ofstream tmpfile(host_filepath);
    if (!tmpfile.is_open()) {
      perror("ERROR: Failed to open temporary file\n");
      exit(EXIT_FAILURE);
    }
    tmpfile.write(static_cast<char *>(hst_ptr), std::streamsize(size));
    tmpfile.close();
    sendingSize = size;
  }

  double sendingSizeInMB = sendingSize / (1024 * 1024);
  auto t_start = std::chrono::high_resolution_clock::now();
  int ret_val =
      DeviceInfo.Providers[device_id]->send_file(host_filepath, filename);
  auto t_end = std::chrono::high_resolution_clock::now();
  auto t_delay =
      std::chrono::duration_cast<std::chrono::seconds>(t_end - t_start).count();
  timing.UploadTime_mutex.lock();
  timing.UploadTime += t_delay;
  timing.UploadTime_mutex.unlock();

  if (DeviceInfo.verbose != Verbosity::quiet)
    DP("Uploaded %.1fMB in %lds\n", sendingSizeInMB, t_delay);

  if (!DeviceInfo.SparkClusters[device_id].KeepTmpFiles)
    remove(host_filepath.c_str());

  return ret_val;
}

static int32_t data_retrieve(int32_t device_id, void *hst_ptr, size_t size,
                             int64_t id) {
  double sizeInMB = size / (1024 * 1024);
  if (size > MAX_JAVA_INT) {
    fprintf(stderr,
            "ERROR: Not supported -- size of %" PRId64
            " is larger (%.1fMB) than the maximal size of JVM's bytearrays "
            "(%.1fMB).\n",
            id, sizeInMB, MAX_SIZE_IN_MB);
    exit(EXIT_FAILURE);
  }

  ElapsedTime &timing = DeviceInfo.ElapsedTimes[device_id];

  bool needDecompression = DeviceInfo.SparkClusters[device_id].Compression &&
                           size >= MIN_SIZE_COMPRESSION;

  std::string filename = std::to_string(id);
  std::string host_filepath = DeviceInfo.working_path + "/" + filename;

  auto t_start = std::chrono::high_resolution_clock::now();
  DeviceInfo.Providers[device_id]->get_file(host_filepath, filename);
  auto t_end = std::chrono::high_resolution_clock::now();
  auto t_delay =
      std::chrono::duration_cast<std::chrono::seconds>(t_end - t_start).count();
  timing.DownloadTime_mutex.lock();
  timing.DownloadTime += t_delay;
  timing.DownloadTime_mutex.unlock();

  if (DeviceInfo.verbose != Verbosity::quiet)
    DP("Downloaded %.1fMB in %lds\n", sizeInMB, t_delay);

  if (needDecompression) {
    auto t_start = std::chrono::high_resolution_clock::now();
    // Decompress data directly to the host memory
    size_t decomp_size =
        decompress_file(host_filepath, static_cast<char *>(hst_ptr), size);
    if (decomp_size != size) {
      fprintf(stderr, "Decompressed data are not the right size. => %zu\n",
              decomp_size);
      exit(EXIT_FAILURE);
    }
    auto t_end = std::chrono::high_resolution_clock::now();
    auto t_delay =
        std::chrono::duration_cast<std::chrono::seconds>(t_end - t_start)
            .count();

    timing.DecompressionTime_mutex.lock();
    timing.DecompressionTime += t_delay;
    timing.DecompressionTime_mutex.unlock();
    if (DeviceInfo.verbose != Verbosity::quiet)
      DP("Decompressed %.1fMB in %lds\n", sizeInMB, t_delay);

  } else {

    // Reading contents of temporary file
    FILE *ftmp = fopen(host_filepath.c_str(), "rb");

    if (!ftmp) {
      perror("ERROR: Could not open temporary file.");
      exit(EXIT_FAILURE);
    }

    if (fread(hst_ptr, 1, size, ftmp) != size) {
      fprintf(stderr,
              "ERROR: Could not successfully read temporary file. => %" PRId64
              "\n",
              size);
      fclose(ftmp);

      if (!DeviceInfo.SparkClusters[device_id].KeepTmpFiles)
        remove(host_filepath.c_str());
      exit(EXIT_FAILURE);
    }

    fclose(ftmp);
  }

  if (!DeviceInfo.SparkClusters[device_id].KeepTmpFiles)
    remove(host_filepath.c_str());

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_submit(int32_t device_id, void *tgt_ptr, void *hst_ptr,
                              int64_t size) {
  int64_t id = int64_t(tgt_ptr);

  if (DeviceInfo.SparkClusters[device_id].UseThreads) {
    DeviceInfo.submitting_threads[device_id].push_back(
        std::thread(data_submit, device_id, hst_ptr, size, id));
  } else {
    return data_submit(device_id, hst_ptr, size, id);
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_retrieve(int32_t device_id, void *hst_ptr, void *tgt_ptr,
                                int64_t size) {
  int64_t id = (int64_t)tgt_ptr;

  if (DeviceInfo.SparkClusters[device_id].UseThreads) {
    DeviceInfo.retrieving_threads[device_id].push_back(
        std::thread(data_retrieve, device_id, hst_ptr, size, id));
  } else {
    return data_retrieve(device_id, hst_ptr, size_t(size), id);
  }

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_delete(int32_t device_id, void *tgt_ptr) {
  uintptr_t id = (uintptr_t)tgt_ptr;
  std::string filename = std::to_string(id);
  // FIXME: Check retrieving thread is over before deleting data
  // return DeviceInfo.Providers[device_id]->delete_file(filename);

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_team_region(int32_t device_id, void *tgt_entry_ptr,
                                         void **tgt_args,
                                         ptrdiff_t *tgt_offsets,
                                         int32_t arg_num, int32_t team_num,
                                         int32_t thread_limit,
                                         uint64_t loop_tripcount /*not used*/) {
  // ignore team num and thread limit.

  // Use libffi to launch execution.
  ffi_cif cif;

  // All args are references.
  std::vector<ffi_type *> args_types(arg_num, &ffi_type_pointer);
  std::vector<void *> args(arg_num);
  std::vector<void *> ptrs(arg_num);

  for (int32_t i = 0; i < arg_num; ++i) {
    ptrs[i] = (void *)((intptr_t)tgt_args[i] + tgt_offsets[i]);
    args[i] = &ptrs[i];
  }

  ffi_status status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, arg_num,
                                   &ffi_type_void, &args_types[0]);

  assert(status == FFI_OK && "Unable to prepare target launch!");

  if (status != FFI_OK)
    return OFFLOAD_FAIL;

  DP("Running entry point at " DPxMOD "...\n", DPxPTR(tgt_entry_ptr));

  void (*entry)(void);
  *((void **)&entry) = tgt_entry_ptr;
  ffi_call(&cif, entry, NULL, &args[0]);
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_region(int32_t device_id, void *tgt_entry_ptr,
                                    void **tgt_args, ptrdiff_t *tgt_offsets,
                                    int32_t arg_num) {
  // use one team and one thread.
  return __tgt_rtl_run_target_team_region(device_id, tgt_entry_ptr, tgt_args,
                                          tgt_offsets, arg_num, 1, 1, 0);
}

#ifdef __cplusplus
}
#endif
