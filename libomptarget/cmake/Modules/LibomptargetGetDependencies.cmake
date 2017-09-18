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
# Intel HARP: required to control offloading to Intel HARP FPGAs.

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
    ENV CPATH)

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
      ENV LD_LIBRARY_PATH)
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
# Looking for Intel HARP...
################################################################################
find_path (
    LIBOMPTARGET_DEP_AALSDK_INCLUDE_DIRS
  NAMES
    aalsdk/Runtime.h
    aalsdk/AALTypes.h
    aalsdk/Runtime.h
    aalsdk/AALLoggerExtern.h
    aalsdk/service/IALIAFU.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ${AALSDK}/include
    ENV CPATH)

find_library (
    LIBOMPTARGET_DEP_AALSDK_AALRT_LIBRARY
  NAMES
    aalrt
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ${AALSDK}/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

find_library (
    LIBOMPTARGET_DEP_AALSDK_AALCLP_LIBRARY
  NAMES
    aalclp
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ${AALSDK}/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

find_library (
    LIBOMPTARGET_DEP_AALSDK_OSAL_LIBRARY
  NAMES
    OSAL
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ${AALSDK}/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

find_library (
    LIBOMPTARGET_DEP_AALSDK_AAS_LIBRARY
  NAMES
    AAS
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ${AALSDK}/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

set(LIBOMPTARGET_DEP_AALSDK_LIBRARIES
  ${LIBOMPTARGET_DEP_AALSDK_LIBRARIES}
  ${LIBOMPTARGET_DEP_AALSDK_AALCLP_LIBRARY})

set(LIBOMPTARGET_DEP_AALSDK_LIBRARIES
  ${LIBOMPTARGET_DEP_AALSDK_LIBRARIES}
  ${LIBOMPTARGET_DEP_AALSDK_OSAL_LIBRARY})

set(LIBOMPTARGET_DEP_AALSDK_LIBRARIES
  ${LIBOMPTARGET_DEP_AALSDK_LIBRARIES}
  ${LIBOMPTARGET_DEP_AALSDK_AAS_LIBRARY})

find_package_handle_standard_args(
  LIBOMPTARGET_DEP_AALSDK
  DEFAULT_MSG
  LIBOMPTARGET_DEP_AALSDK_LIBRARIES
  LIBOMPTARGET_DEP_AALSDK_INCLUDE_DIRS)

mark_as_advanced(
  LIBOMPTARGET_DEP_AALSDK_INCLUDE_DIRS
  LIBOMPTARGET_DEP_AALSDK_LIBRARIES)

