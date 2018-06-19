#
#//===----------------------------------------------------------------------===//
#//
#//                     The LLVM Compiler Infrastructure
#//
#// This file is dual licensed under the MIT and the University of Illinois Open
#// Source Licenses. See LICENSE.txt for details.
#//
#//===----------------------------------------------------------------------===//
#

# Try to detect in the system several dependencies required by the different
# components of libomptarget. These are the dependencies we have:
#
# libelf : required by some targets to handle the ELF files at runtime.
# libffi : required to launch target kernels given function and argument
#          pointers.
# CUDA : required to control offloading to NVIDIA GPUs.
# libhdfs3 : required to access HDFS based cloud provider

include (FindPackageHandleStandardArgs)

################################################################################
# Looking for libelf...
################################################################################

find_path (
  LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIR
  NAMES
    libelf.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ENV CPATH
  PATH_SUFFIXES
    libelf)

find_library (
  LIBOMPTARGET_DEP_LIBELF_LIBRARIES
  NAMES
    elf
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

set(LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS ${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIR})
find_package_handle_standard_args(
  LIBOMPTARGET_DEP_LIBELF
  DEFAULT_MSG
  LIBOMPTARGET_DEP_LIBELF_LIBRARIES
  LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS)

mark_as_advanced(
  LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS
  LIBOMPTARGET_DEP_LIBELF_LIBRARIES)

################################################################################
# Looking for libffi...
################################################################################
find_package(PkgConfig)

pkg_check_modules(LIBOMPTARGET_SEARCH_LIBFFI QUIET libffi)

find_path (
  LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR
  NAMES
    ffi.h
  HINTS
    ${LIBOMPTARGET_SEARCH_LIBFFI_INCLUDEDIR}
    ${LIBOMPTARGET_SEARCH_LIBFFI_INCLUDE_DIRS}
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ENV CPATH
  PATH_SUFFIXES
    ffi)

# Don't bother look for the library if the header files were not found.
if (LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR)
  find_library (
      LIBOMPTARGET_DEP_LIBFFI_LIBRARIES
    NAMES
      ffi
    HINTS
      ${LIBOMPTARGET_SEARCH_LIBFFI_LIBDIR}
      ${LIBOMPTARGET_SEARCH_LIBFFI_LIBRARY_DIRS}
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH
    PATH_SUFFIXES
      ffi)
endif()

set(LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIRS ${LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR})
find_package_handle_standard_args(
  LIBOMPTARGET_DEP_LIBFFI
  DEFAULT_MSG
  LIBOMPTARGET_DEP_LIBFFI_LIBRARIES
  LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIRS)

mark_as_advanced(
  LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIRS
  LIBOMPTARGET_DEP_LIBFFI_LIBRARIES)

################################################################################
# Looking for CUDA...
################################################################################
find_package(CUDA QUIET)

set(LIBOMPTARGET_DEP_CUDA_FOUND ${CUDA_FOUND})
set(LIBOMPTARGET_DEP_CUDA_LIBRARIES ${CUDA_LIBRARIES})
set(LIBOMPTARGET_DEP_CUDA_INCLUDE_DIRS ${CUDA_INCLUDE_DIRS})

mark_as_advanced(
  LIBOMPTARGET_DEP_CUDA_FOUND
  LIBOMPTARGET_DEP_CUDA_INCLUDE_DIRS
  LIBOMPTARGET_DEP_CUDA_LIBRARIES)


################################################################################
# Looking for libhdfs3...
################################################################################

find_path (
  LIBOMPTARGET_DEP_LIBHDFS3_INCLUDE_DIR
  NAMES
    hdfs.h
  HINTS
    ${LIBOMPTARGET_SEARCH_LIBHDFS3_INCLUDE_DIRS}
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ENV CPATH
  PATH_SUFFIXES
    hdfs)

# Don't bother look for the library if the header files were not found.
if (LIBOMPTARGET_DEP_LIBHDFS3_INCLUDE_DIR)
  find_library (
      LIBOMPTARGET_DEP_LIBHDFS3_LIBRARIES
    NAMES
      hdfs3
    HINTS
      ${LIBOMPTARGET_SEARCH_LIBHDFS3_LIBDIR}
      ${LIBOMPTARGET_SEARCH_LIBHDFS3_LIBRARY_DIRS}
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)
endif()

set(LIBOMPTARGET_DEP_LIBHDFS3_INCLUDE_DIRS ${LIBOMPTARGET_DEP_LIBHDFS3_INCLUDE_DIR})
find_package_handle_standard_args(
  LIBOMPTARGET_DEP_LIBHDFS3
  DEFAULT_MSG
  LIBOMPTARGET_DEP_LIBHDFS3_LIBRARIES
  LIBOMPTARGET_DEP_LIBHDFS3_INCLUDE_DIRS)

mark_as_advanced(
  LIBOMPTARGET_DEP_LIBHDFS3_INCLUDE_DIRS
  LIBOMPTARGET_DEP_LIBHDFS3_LIBRARIES)


################################################################################
# Looking for libssh...
################################################################################

find_path (
  LIBOMPTARGET_DEP_LIBSSH_INCLUDE_DIR
  NAMES
    libssh.h
  HINTS
    ${LIBOMPTARGET_SEARCH_LIBSSH_INCLUDE_DIRS}
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ENV CPATH
  PATH_SUFFIXES
    libssh)

# Don't bother look for the library if the header files were not found.
if (LIBOMPTARGET_DEP_LIBSSH_INCLUDE_DIR)
  find_library (
      LIBOMPTARGET_DEP_LIBSSH_LIBRARIES
    NAMES
      ssh
    HINTS
      ${LIBOMPTARGET_SEARCH_LIBSSH_LIBDIR}
      ${LIBOMPTARGET_SEARCH_LIBSSH_LIBRARY_DIRS}
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH
    PATH_SUFFIXES
      ${CMAKE_LIBRARY_ARCHITECTURE})
endif()

set(LIBOMPTARGET_DEP_LIBSSH_INCLUDE_DIRS ${LIBOMPTARGET_DEP_LIBSSH_INCLUDE_DIR})
find_package_handle_standard_args(
  LIBOMPTARGET_DEP_LIBSSH
  DEFAULT_MSG
  LIBOMPTARGET_DEP_LIBSSH_LIBRARIES
  LIBOMPTARGET_DEP_LIBSSH_INCLUDE_DIRS)

mark_as_advanced(
  LIBOMPTARGET_DEP_LIBSSH_INCLUDE_DIRS
  LIBOMPTARGET_DEP_LIBSSH_LIBRARIES)

################################################################################
# Looking for zlib...
################################################################################

find_path (
  LIBOMPTARGET_DEP_ZLIB_INCLUDE_DIR
  NAMES
    zlib.h
  HINTS
    ${LIBOMPTARGET_SEARCH_ZLIB_INCLUDE_DIRS}
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ENV CPATH
  PATH_SUFFIXES
    libssh)

# Don't bother look for the library if the header files were not found.
if (LIBOMPTARGET_DEP_ZLIB_INCLUDE_DIR)
  find_library (
      LIBOMPTARGET_DEP_ZLIB_LIBRARIES
    NAMES
      z zlib zdll zlib1
    HINTS
      ${LIBOMPTARGET_SEARCH_ZLIB_LIBDIR}
      ${LIBOMPTARGET_SEARCH_ZLIB_LIBRARY_DIRS}
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)
endif()

set(LIBOMPTARGET_DEP_ZLIB_INCLUDE_DIRS ${LIBOMPTARGET_DEP_ZLIB_INCLUDE_DIR})
find_package_handle_standard_args(
  LIBOMPTARGET_DEP_ZLIB
  DEFAULT_MSG
  LIBOMPTARGET_DEP_ZLIB_LIBRARIES
  LIBOMPTARGET_DEP_ZLIB_INCLUDE_DIRS)

mark_as_advanced(
  LIBOMPTARGET_DEP_ZLIB_INCLUDE_DIRS
  LIBOMPTARGET_DEP_ZLIB_LIBRARIES)

################################################################################
# Looking for sbt...
################################################################################

find_program(LIBOMPTARGET_DEP_SBT_EXECUTABLE
  NAMES
    sbt
  HINTS
    $ENV{LIBOMPTARGET_SEARCH_SBT_DIR}
  PATHS
    /usr/bin
    /usr/local/bin
    /opt/local/bin
    /sw/bin
)

find_package_handle_standard_args(
  LIBOMPTARGET_DEP_SBT
  DEFAULT_MSG
  LIBOMPTARGET_DEP_SBT_EXECUTABLE)

mark_as_advanced(
  LIBOMPTARGET_DEP_SBT_EXECUTABLE
)


################################################################################
# Looking for threads...
################################################################################

find_package(THREADS QUIET)

find_package(JNI REQUIRED)

set(LIBOMPTARGET_DEP_JNI_FOUND ${JNI_FOUND})
set(LIBOMPTARGET_DEP_JNI_LIBRARIES ${JNI_LIBRARIES})
set(LIBOMPTARGET_DEP_JNI_INCLUDE_DIRS ${JNI_INCLUDE_DIRS})

mark_as_advanced(
  LIBOMPTARGET_DEP_JNI_FOUND
  LIBOMPTARGET_DEP_JNI_INCLUDE_DIRS
  LIBOMPTARGET_DEP_JNI_LIBRARIES)



    message(STATUS "JAVA INFO ARE " ${LIBOMPTARGET_DEP_Java_INCLUDE_DIRS} ${LIBOMPTARGET_DEP_Java_LIBRARIES})
