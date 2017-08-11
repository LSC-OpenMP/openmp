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
libomptarget_say("NOT building smartnic plugin contact us for more information.")

#if(LIBOMPTARGET_DEP_LIBELF_FOUND)
#  if(LIBOMPTARGET_DEP_LIBFFI_FOUND)
#    libomptarget_say("Building SmartNIC offloading plugin.")

    # Define the suffix for the runtime messaging dumps.
#    add_definitions(-DTARGET_NAME=SMARTNIC)

#    if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
#      add_definitions(-DSMARTNIC_ERROR_REPORT)
#    endif()

#    include_directories(${LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR})
#    include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS})

#    add_library(omptarget.rtl.smartnic SHARED src/rtl.cpp)

    # Install plugin under the lib destination folder.
#    if(CMAKE_SYSTEM_NAME MATCHES "Windows")
#      install(TARGETS omptarget.rtl.smartnic RUNTIME DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
#    else()
#      install(TARGETS omptarget.rtl.smartnic LIBRARY DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
#    endif()

#    target_link_libraries(omptarget.rtl.smartnic
#      ${LIBOMPTARGET_DEP_LIBFFI_LIBRARIES}
#      ${LIBOMPTARGET_DEP_LIBELF_LIBRARIES}
#      dl
#      "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")

    # Report to the parent scope that we are building a plugin for SmartNIC.
#    set(LIBOMPTARGET_SYSTEM_TARGETS "${LIBOMPTARGET_SYSTEM_TARGETS} x86_64" PARENT_SCOPE)
#  else(LIBOMPTARGET_DEP_LIBFFI_FOUND)
#    libomptarget_say("Not building SmartNIC offloading plugin: libffi dependency not found.")
#  endif(LIBOMPTARGET_DEP_LIBFFI_FOUND)
#else(LIBOMPTARGET_DEP_LIBELF_FOUND)
#  libomptarget_say("Not building SmartNIC offloading plugin: libelf dependency not found.")
#endif(LIBOMPTARGET_DEP_LIBELF_FOUND)
