##===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
#
##===----------------------------------------------------------------------===##
#
# Build the jni Device RTL if the Java Development Kit is available
#
##===----------------------------------------------------------------------===##

if(LIBOMPTARGET_DEP_JNI_FOUND)
  libomptarget_say("Building JNI offloading device RTL.")

  set(src_files
      src/omptarget-jni.c
  )

  include_directories(
    ${LIBOMPTARGET_DEP_JNI_INCLUDE_DIRS}
    src/
  )

  set(BUILD_SHARED_LIBS OFF)

  add_library(omptarget-jni STATIC ${src_files} ${omp_data_objects})

  # Install device RTL under the lib destination folder.
  install(TARGETS omptarget-jni ARCHIVE DESTINATION "lib")

  target_link_libraries(omptarget-jni ${LIBOMPTARGET_DEP_JNI_LIBRARIES})
else()
  libomptarget_say("Not building JNI offloading device RTL: JDK not found in the system.")
endif()
