##===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
#
##===----------------------------------------------------------------------===##
#
# Build a plugin for a AWS EC2 F1 machine if available.
#
##===----------------------------------------------------------------------===##
if(LIBOMPTARGET_DEP_LIBELF_FOUND)
if(LIBOMPTARGET_DEP_LIBFFI_FOUND)
  libomptarget_say("Building AWS EC2 F1 offloading plugin.")

  # Define the suffix for the runtime messaging dumps.
  add_definitions(-DTARGET_NAME=AWSF1)

  if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
    add_definitions(-DAWSF1_ERROR_REPORT)
  endif()

  include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS})
  include_directories(${LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIRS})

  add_library(omptarget.rtl.awsf1 SHARED
              src/util/awsf1_generic_app.cpp
              src/rtl.cpp)

  # Install plugin under the lib destination folder.
  if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    install(TARGETS omptarget.rtl.awsf1 RUNTIME DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
  else()
    install(TARGETS omptarget.rtl.awsf1 LIBRARY DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
  endif()

  target_link_libraries(omptarget.rtl.awsf1
    ${LIBOMPTARGET_DEP_LIBFFI_LIBRARIES}
    ${LIBOMPTARGET_DEP_LIBELF_LIBRARIES}
    dl
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")

  # Report to the parent scope that we are building a plugin for AWS EC2 F1.
  set(LIBOMPTARGET_SYSTEM_TARGETS "${LIBOMPTARGET_SYSTEM_TARGETS} x86_64" PARENT_SCOPE)
else(LIBOMPTARGET_DEP_LIBFFI_FOUND)
  libomptarget_say("Not building AWS EC2 F1 offloading plugin: libffi dependency not found.")
endif(LIBOMPTARGET_DEP_LIBFFI_FOUND)
else(LIBOMPTARGET_DEP_LIBELF_FOUND)
  libomptarget_say("Not building AWS EC2 F1 offloading plugin: libelf dependency not found.")
endif(LIBOMPTARGET_DEP_LIBELF_FOUND)

