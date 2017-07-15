##===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
#
##===----------------------------------------------------------------------===##
#
# Build the Spark Device RTL if SBT is available
#
##===----------------------------------------------------------------------===##)

if(LIBOMPTARGET_DEP_SBT_FOUND)
  libomptarget_say("Building Spark offloading device RTL.")

  add_custom_target(omptarget-spark ALL
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${SBT_EXECUTABLE} publish-local < /dev/null)

else()
  libomptarget_say("Not building Spark offloading device RTL: SBT not found in the system.")
endif()
