//===--- opae_generic_app.cpp - AFU Link Interface Implementation - C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// Generic Link Interface implementation
//
//===----------------------------------------------------------------------===//

#include "awsf1_generic_app.h"
#include "utils.h"

int h_axi00_ptr0_input[MAX_LENGTH];                    // host memory for input vector
int h_axi00_ptr0_output[MAX_LENGTH];

char *xclbin;

AWSF1GenericApp::AWSF1GenericApp() {}

AWSF1GenericApp::~AWSF1GenericApp() {}

int AWSF1GenericApp::init() {
  //  xclbin = "binary_container_1.xclbin";
  init_util();
  return 0;
}

void* AWSF1GenericApp::alloc_buffer(uint64_t size) {
  return data_alloc(size);

}

void AWSF1GenericApp::submit_buffer(void *tgt_ptr, void *hst_ptr, int64_t size) {
  data_submit(tgt_ptr, hst_ptr, size);

}


void AWSF1GenericApp::retrieve_buffer(void *hst_ptr, void *tgt_ptr, int64_t size){
  data_retrieve(hst_ptr, tgt_ptr, size);

}

void AWSF1GenericApp::delete_buffer(void *tgt_ptr) {
  data_delete(tgt_ptr);
  return;
}

int AWSF1GenericApp::finish() {

  return 0;
}

int AWSF1GenericApp::program(const char *module) {

  return 0;
}

int AWSF1GenericApp::run() {

    // device memory used for a vector


   // if (argc != 2) {
   //     printf("Usage: %s xclbin\n", argv[0]);
   //     return EXIT_FAILURE;
   // }

   // xclbin = argv[1];





    // Create the input and output arrays in device memory for our calculation


//    d_axi00_ptr0 = (cl_mem)data_alloc(number_of_words);


    // Write our data set into the input array in device memory
    //

  //  data_submit(d_axi00_ptr0, h_axi00_ptr0_input,sizeof(int) * number_of_words);


    // Set the arguments to our compute kernel
    // int vector_length = MAX_LENGTH;

    run_target();

    // Read back the results from the device to verify the output
    //

  //  data_retrieve(h_axi00_ptr0_output, d_axi00_ptr0,sizeof(int) * number_of_words);

//    for (uint i = 0; i < number_of_words; i++) {
  //  	if ((h_axi00_ptr0_input[i] + 1) != h_axi00_ptr0_output[i]) {
  //  		printf("ERROR in sdx_kernel_wizard_0 - array index %d (host addr 0x%03x) - input=%d (0x%x), output=%d (0x%x)\n", i, i*4, h_axi00_ptr0_input[i], h_axi00_ptr0_input[i], h_axi00_ptr0_output[i], h_axi00_ptr0_output[i]);
  //  	    check_status = 1;
  //  	}
    	      //  printf("i=%d, input=%d, output=%d\n", i,  h_axi00_ptr0_input[i], h_axi00_ptr0_output[i]);
  //  }

    //--------------------------------------------------------------------------
    // Shutdown and cleanup
    //--------------------------------------------------------------------------

    return 0;
}
