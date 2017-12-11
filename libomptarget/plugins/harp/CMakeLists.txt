##===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
#
##===----------------------------------------------------------------------===##
#
# Build a plugin for a Intel HARP machine if available.
#
##===----------------------------------------------------------------------===##
if(LIBOMPTARGET_DEP_LIBELF_FOUND)
if(LIBOMPTARGET_DEP_LIBFFI_FOUND)
if(LIBOMPTARGET_DEP_OPAE_FOUND)
if(LIBOMPTARGET_DEP_BBB_CCI_MPF_FOUND)
if(LIBOMPTARGET_DEP_UUID_FOUND)
if(LIBOMPTARGET_DEP_BOOST_FOUND)
  libomptarget_say("Building Intel HARP offloading plugin.")

  # Define the suffix for the runtime messaging dumps.
  add_definitions(-DTARGET_NAME=HARP)

  if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
    add_definitions(-DHARP_ERROR_REPORT)
  endif()

  include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS})
  include_directories(${LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIRS})
  include_directories(${LIBOMPTARGET_DEP_OPAE_INCLUDE_DIRS})
  include_directories(${LIBOMPTARGET_DEP_BBB_CCI_MPF_INCLUDE_DIRS})
  include_directories(${LIBOMPTARGET_DEP_UUID_INCLUDE_DIRS})
  include_directories(${LIBOMPTARGET_DEP_BOOST_INCLUDE_DIRS})

  add_library(omptarget.rtl.harp SHARED
              src/util/opae_generic_app.cpp
              src/rtl.cpp)

  # Install plugin under the lib destination folder.
  if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    install(TARGETS omptarget.rtl.harp RUNTIME DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
  else()
    install(TARGETS omptarget.rtl.harp LIBRARY DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
  endif()

  target_link_libraries(omptarget.rtl.harp
    ${LIBOMPTARGET_DEP_LIBFFI_LIBRARIES}
    ${LIBOMPTARGET_DEP_LIBELF_LIBRARIES}
    ${LIBOMPTARGET_DEP_OPAE_LIBRARIES}
    ${LIBOMPTARGET_DEP_BBB_CCI_MPF_LIBRARIES}
    ${LIBOMPTARGET_DEP_UUID_LIBRARIES}
    ${LIBOMPTARGET_DEP_BOOST_LIBRARIES}
    dl
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")

  # Report to the parent scope that we are building a plugin for Intel HARP.
  set(LIBOMPTARGET_SYSTEM_TARGETS "${LIBOMPTARGET_SYSTEM_TARGETS} x86_64" PARENT_SCOPE)
else(LIBOMPTARGET_DEP_BOOST_FOUND)
  libomptarget_say("Not building Intel HARP offloading plugin: boost program-options dependency not found.")
endif(LIBOMPTARGET_DEP_BOOST_FOUND)
else(LIBOMPTARGET_DEP_UUID_FOUND)
  libomptarget_say("Not building Intel HARP offloading plugin: uuid dependency not found.")
endif(LIBOMPTARGET_DEP_UUID_FOUND)
else(LIBOMPTARGET_DEP_BBB_CCI_MPF_FOUND)
  libomptarget_say("Not building Intel HARP offloading plugin: intel bbb_cci_mpf dependency not found.")
endif(LIBOMPTARGET_DEP_BBB_CCI_MPF_FOUND)
else(LIBOMPTARGET_DEP_OPAE_FOUND)
  libomptarget_say("Not building Intel HARP offloading plugin: intel opae dependency not found.")
endif(LIBOMPTARGET_DEP_OPAE_FOUND)
else(LIBOMPTARGET_DEP_LIBFFI_FOUND)
  libomptarget_say("Not building Intel HARP offloading plugin: libffi dependency not found.")
endif(LIBOMPTARGET_DEP_LIBFFI_FOUND)
else(LIBOMPTARGET_DEP_LIBELF_FOUND)
  libomptarget_say("Not building Intel HARP offloading plugin: libelf dependency not found.")
endif(LIBOMPTARGET_DEP_LIBELF_FOUND)

