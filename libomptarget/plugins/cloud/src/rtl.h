#ifndef _INCLUDE_RTL_H
#define _INCLUDE_RTL_H

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "INIReader.h"

class CloudProvider;

extern const char *__progname; /* for job name */

enum SparkMode { client, cluster, invalid };
enum Verbosity { debug, info, quiet };

struct SparkInfo {
  std::string ServAddress;
  int ServPort;
  SparkMode Mode;
  std::string UserName;
  std::string BinPath;
  std::string Package;
  std::string JarPath;
  std::string AdditionalArgs;
  std::string WorkingDir;
  bool Compression;
  std::string CompressionFormat;
  bool UseThreads;
  Verbosity VerboseMode;
  bool KeepTmpFiles;
  std::string SchedulingSize;
  std::string SchedulingKind;
  uintptr_t currAddr;
};

struct ProviderListEntry {
  std::string ProviderName;
  CloudProvider *(*ProviderGenerator)(SparkInfo &);
  std::string SectionName;
};

struct ElapsedTime {
  int CompressionTime = 0;
  std::mutex CompressionTime_mutex;
  int DecompressionTime = 0;
  std::mutex DecompressionTime_mutex;
  int UploadTime = 0;
  std::mutex UploadTime_mutex;
  int DownloadTime = 0;
  std::mutex DownloadTime_mutex;
  int SparkExecutionTime = 0;
};

const std::string OMPCLOUD_CONF_ENV = "OMPCLOUD_CONF_PATH";
const int DEFAULT_SPARK_PORT = 7077;
const std::string DEFAULT_SPARK_USER = "anonymous";
const std::string DEFAULT_SPARK_MODE = "client";
const std::string DEFAULT_SPARK_PACKAGE = "org.llvm.openmp.OmpKernel";
const std::string DEFAULT_SPARK_JARPATH =
    "target/scala-2.11/test-assembly-0.2.0.jar";

// Only data larger than about 1MB are compressed
const int MIN_SIZE_COMPRESSION = 1000000;
const std::string DEFAULT_COMPRESSION_FORMAT = "gzip";

// Maximal size of offloaded data is about 2GB
// Size of JVM's ByteArrays are limited by MAX_JAVA_INT = 2^31-1
const long MAX_JAVA_INT = 2147483647;
const double MAX_SIZE_IN_MB = MAX_JAVA_INT / (1024 * 1024);

#endif
