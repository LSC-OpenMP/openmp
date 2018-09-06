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


    run_target();


    return 0;
}
