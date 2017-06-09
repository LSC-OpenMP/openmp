##===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
#
##===----------------------------------------------------------------------===##
#
# Build a plugin for a SmartNIC machine if available.
#
##===----------------------------------------------------------------------===##
if(LIBOMPTARGET_DEP_LIBELF_FOUND)
  libomptarget_say("Building SmartNIC offloading plugin.")

  # Define the suffix for the runtime messaging dumps.
  add_definitions(-DTARGET_NAME=SMARTNIC)

  if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
    add_definitions(-DSMARTNIC_ERROR_REPORT)
  endif()

  include_directories(${LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR})
  include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS})

  add_library(omptarget.rtl.smartnic SHARED src/rtl.cpp)

  # Install plugin under the lib destination folder.
  install(TARGETS omptarget.rtl.smartnic LIBRARY DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})

  target_link_libraries(omptarget.rtl.smartnic
    ${LIBOMPTARGET_DEP_LIBFFI_LIBRARIES} 
    ${LIBOMPTARGET_DEP_LIBELF_LIBRARIES}
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")

  # Report to the parent scope that we are building a plugin for SmartNIC.
  set(LIBOMPTARGET_SYSTEM_TARGETS "${LIBOMPTARGET_SYSTEM_TARGETS} x86_64" PARENT_SCOPE)
else(LIBOMPTARGET_DEP_LIBELF_FOUND)
  libomptarget_say("Not building SmartNICoffloading plugin: libelf dependency not found.")
endif(LIBOMPTARGET_DEP_LIBELF_FOUND)
