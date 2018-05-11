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

AWSF1GenericApp::AWSF1GenericApp() {}

AWSF1GenericApp::~AWSF1GenericApp() {}

int AWSF1GenericApp::init() {

  return 0;
}

void* AWSF1GenericApp::alloc_buffer(uint64_t size) {
  Buffer buffer;

  buffer.size = size;
  buffer.virt = malloc(size);

  buffers.push_front(buffer);

  return buffer.virt;
}

void AWSF1GenericApp::delete_buffer(void *tgt_ptr) {
  for (uint32_t i = 0; buffers.size(); i++) {
    if (buffers[i].virt == tgt_ptr) {
      buffers.erase(buffers.begin() + i);

      if (buffers.empty())
        this->finish();

      return;
    }
  }
}

int AWSF1GenericApp::finish() {

  return 0;
}

int AWSF1GenericApp::program(const char *module) {

  return 0;
}

int AWSF1GenericApp::run() {

  return 0;
}

