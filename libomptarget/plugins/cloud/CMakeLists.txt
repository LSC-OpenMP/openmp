##===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
#
##===----------------------------------------------------------------------===##
#
# Build a plugin for a cloud machine if available.
#
##===----------------------------------------------------------------------===##
if(LIBOMPTARGET_DEP_LIBELF_FOUND)
  if(LIBOMPTARGET_DEP_LIBSSH_FOUND)
    if(LIBOMPTARGET_DEP_ZLIB_FOUND)
      if(LIBOMPTARGET_DEP_LIBHDFS3_FOUND)
        message("Building cloud offloading plugin")

        include_directories(src/)
        include_directories(src/providers)
        include_directories(src/util)
        include_directories(inih)
        include_directories(${LIBOMPTARGET_DEP_LIBSSH_INCLUDE_DIRS})
        include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS})
        include_directories(${LIBOMPTARGET_DEP_LIBHDFS3_INCLUDE_DIRS})
        include_directories(${LIBOMPTARGET_DEP_ZLIB_INCLUDE_DIRS})

        add_library(
            inih SHARED inih/ini.c inih/INIReader.cpp)

        add_library(
            omptarget.rtl.cloud
            SHARED
            src/rtl.cpp
            src/providers/amazon.cpp
            src/providers/azure.cpp
            src/providers/generic.cpp
            src/providers/local.cpp
            src/util/cloud_ssh.cpp
            src/util/cloud_compression.cpp
            src/util/cloud_util.cpp)

        # Install plugin under the lib destination folder.
        install(TARGETS omptarget.rtl.cloud LIBRARY DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})

        if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
           target_link_libraries(omptarget.rtl.cloud
              inih
              ${LIBOMPTARGET_DEP_LIBHDFS3_LIBRARIES}
              ${LIBOMPTARGET_DEP_LIBELF_LIBRARIES}
              ${LIBOMPTARGET_DEP_LIBSSH_LIBRARIES}
              ${LIBOMPTARGET_DEP_ZLIB_LIBRARIES})
        else()
           target_link_libraries(omptarget.rtl.cloud
              inih
              ${LIBOMPTARGET_DEP_LIBHDFS3_LIBRARIES}
              ${LIBOMPTARGET_DEP_LIBELF_LIBRARIES}
              ${LIBOMPTARGET_DEP_LIBSSH_LIBRARIES}
              ${LIBOMPTARGET_DEP_ZLIB_LIBRARIES}
              "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")
        endif()

      else()
        libomptarget_say("Not building cloud offloading plugin: libhdfs3 not found in system.")
      endif()
    else()
      libomptarget_say("Not building cloud offloading plugin: zlib not found in system.")
    endif()
  else()
    libomptarget_say("Not building cloud offloading plugin: libssh not found in system.")
  endif()
else()
  libomptarget_say("Not building cloud offloading plugin: libelf dependency not found.")
endif()
