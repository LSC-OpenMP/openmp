##===----------------------------------------------------------------------===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
# 
##===----------------------------------------------------------------------===##
#
# Build the MPI Device RTL if the MPI tools are available
#
##===----------------------------------------------------------------------===##

if(LIBOMPTARGET_DEP_MPI_FOUND)
  libomptarget_say("Building MPI offloading device RTL.")
  
  # NVPTX runtime library has to be statically linked. Dynamic linking is not
  # yet supported by the MPI toolchain on the device.
  set(BUILD_SHARED_LIBS OFF)

  set(src_files
    src/omptarget-mpi.cpp
  )

  add_library(omptarget-mpi STATIC ${src_files})
      
  # Install device RTL under the lib destination folder.
  install(TARGETS omptarget-mpi ARCHIVE DESTINATION "lib")

  # target_link_libraries(omptarget-mpi ${MPI_LIBRARIES})
  
    # Find a clang compiler capable of compiling cuda files to LLVM bitcode and
    # an llvm linker.
    # We use the one provided for the user, attempt to use the
    # clang in the tree, attempt to use clang in the PATH, or just fail.
    
    set(LIBOMPTARGET_MPI_COMPILER /home/ubuntu/lsc/clang-ykt/bin/clang++)
    set(LIBOMPTARGET_BC_LINKER /home/ubuntu/lsc/clang-ykt/bin/llvm-link)
    
  
    if(NOT LIBOMPTARGET_MPI_COMPILER)
      libomptarget_say("Cannot find a MPI compiler capable of emitting LLVM bitcode.")
      libomptarget_say("Please configure with flag -DLIBOMPTARGET_NVPTX_MPI_COMPILER")
    endif()
    if(NOT LIBOMPTARGET_BC_LINKER)
      libomptarget_say("Cannot find a linker capable of linking LLVM bitcode objects.")
      libomptarget_say("Please configure with flag -DLIBOMPTARGET_BC_LINKER")
    endif()
    
    if(LIBOMPTARGET_MPI_COMPILER AND LIBOMPTARGET_BC_LINKER)
      libomptarget_say("Building MPI LLVM bitcode offloading device RTL.")

      
      # Set flags for Clang cuda compilation.  Only Clang is supported because there is
      # no other compiler capable of generating bitcode from cuda sources.
      set(MPI_FLAGS 
          -emit-llvm 
          -O1
	  -pthread
      )

      # This is currently broken
      #set(MPI_INCLUDES
      #  -I${LIBOMPTARGET_DEP_MPI_INCLUDE_DIRS}
      #)

      # Compile files to bitcode.
      set(bc_files "")
      foreach(src ${src_files})
	get_filename_component(infile ${src} ABSOLUTE)
	get_filename_component(outfile ${src} NAME)
    
        add_custom_command(OUTPUT ${outfile}.bc
          COMMAND ${LIBOMPTARGET_MPI_COMPILER} ${MPI_FLAGS}
            -c ${infile} -o ${outfile}.bc
          DEPENDS ${infile} ${LIBOMPTARGET_MPI_COMPILER}
          IMPLICIT_DEPENDS ${infile}
          COMMENT "Building LLVM bitcode mirella ${outfile}.bc"
          VERBATIM
        )
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${outfile}.bc)
	list(APPEND bc_files ${outfile}.bc)
      endforeach()
    

      # Link to a bitcode library.
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libomptarget-mpi.bc
          COMMAND ${LIBOMPTARGET_BC_LINKER} 
            -o ${CMAKE_CURRENT_BINARY_DIR}/libomptarget-mpi.bc ${bc_files}
          DEPENDS ${bc_files} ${LIBOMPTARGET_BC_LINKER}
          COMMENT "Linking LLVM bitcode libomptarget-mpi.bc"
      )
      set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES libomptarget-mpi.bc)

      add_custom_target(omptarget-mpi-bc ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libomptarget-mpi.bc)

      # Copy library to destination.
      add_custom_command(TARGET omptarget-mpi-bc POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libomptarget-mpi.bc
                         $<TARGET_FILE_DIR:omptarget-mpi>)

      # Install device RTL under the lib destination folder.
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libomptarget-mpi.bc DESTINATION "lib")  
      
    endif()
else()
  libomptarget_say("Not building MPI offloading device RTL: MPI tools not found in the system.")
endif()
