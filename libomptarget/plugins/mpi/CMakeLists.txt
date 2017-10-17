##===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
#
##===----------------------------------------------------------------------===##
#
# Build a plugin for a CUDA machine if available.
#
##===----------------------------------------------------------------------===##
if(LIBOMPTARGET_DEP_LIBELF_FOUND)
  if(LIBOMPTARGET_DEP_LIBFFI_FOUND)
    if(LIBOMPTARGET_DEP_MPI_FOUND)
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)|(ppc64le)|(aarch64)$" AND (CMAKE_SYSTEM_NAME MATCHES "Linux"
        # Temporary allows building on MacOS for development
        OR CMAKE_SYSTEM_NAME MATCHES "Darwin"))

        libomptarget_say("Building MPI offloading plugin.")

        # Define the suffix for the runtime messaging dumps.
        add_definitions(-DTARGET_NAME=MPI)

        include_directories(${LIBOMPTARGET_DEP_MPI_INCLUDE_DIRS})
        include_directories(${LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR})
        include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS})

        add_library(omptarget.rtl.mpi SHARED src/rtl.cpp)

        # Install plugin under the lib destination folder.
        install(TARGETS omptarget.rtl.mpi LIBRARY DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})

        target_link_libraries(omptarget.rtl.mpi
          ${LIBOMPTARGET_DEP_MPI_LIBRARIES}
          ${LIBOMPTARGET_DEP_LIBFFI_LIBRARIES}
          ${LIBOMPTARGET_DEP_LIBELF_LIBRARIES}
          dl
          "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")
      else()
        libomptarget_say("Not building MPI offloading plugin: only support MPI in Linux x86_64, ppc64le and aarch64 hosts.")
      endif()
    else()
      libomptarget_say("Not building MPI offloading plugin: MPI not found in system.")
    endif()
  else()
    libomptarget_say("Not building MPI offloading plugin: libffi dependency not found.")
  endif()
else(LIBOMPTARGET_DEP_LIBELF_FOUND)
  libomptarget_say("Not building MPI offloading plugin: libelf dependency not found.")
endif(LIBOMPTARGET_DEP_LIBELF_FOUND)
