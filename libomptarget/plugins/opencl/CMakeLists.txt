if(LIBOMPTARGET_DEP_OPENCL_FOUND)

  libomptarget_say("Building SPIR-V offloading plugin.")

  # Define the suffix for the runtime messaging dumps.
  add_definitions(-DTARGET_NAME=OPENCL)

  if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
    add_definitions(-DOPENCL_ERROR_REPORT)
  endif()

  include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS})

  add_library(omptarget.rtl.opencl SHARED src/rtl.cpp)

  # Install plugin under the lib destination folder.
  install(TARGETS omptarget.rtl.opencl LIBRARY DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
  target_link_libraries(
           omptarget.rtl.spir
           ${LIBOMPTARGET_DEP_OPENCL_LIBRARIES}
           OpenCL
           "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")

  # Report to the parent scope that we are building a plugin for opencl/spir.
  set(LIBOMPTARGET_SYSTEM_TARGETS "${LIBOMPTARGET_SYSTEM_TARGETS} opencl-unknown-unknown" PARENT_SCOPE)
  set(LIBOMPTARGET_SYSTEM_TARGETS "${LIBOMPTARGET_SYSTEM_TARGETS}
  spir64-unknown-unknown" PARENT_SCOPE)
  
else()
  libomptarget_say("Not building opencl/spir offloading plugin: OpenCL 1.2+ not found in system.")
endif()
