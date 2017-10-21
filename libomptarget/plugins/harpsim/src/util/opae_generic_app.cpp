//===--- opae_generic_app.cpp - AFU Link Interface Implementation - C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// Generic AFU Link Interface implementation
//
//===----------------------------------------------------------------------===//

#include "opae_generic_app.h"

OPAEGenericApp::OPAEGenericApp() {

}

OPAEGenericApp::~OPAEGenericApp() {
}

int OPAEGenericApp::init() {

  fpga_properties filter = NULL;
  fpga_guid guid;
  fpga_token accel_token;
  uint32_t num_matches;
  fpga_result r;

  // Don't print verbose messages in ASE by default
  // setenv("ASE_LOG", "0", 0);

  // Set up a filter that will search for an accelerator
  fpgaGetProperties(NULL, &filter);
  fpgaPropertiesSetObjectType(filter, FPGA_ACCELERATOR);

  // Add the desired UUID to the filter
  uuid_parse(AFU_UUID, guid);
  fpgaPropertiesSetGUID(filter, guid);

  // Do the search across the available FPGA contexts
  num_matches = 1;
  fpgaEnumerate(&filter, 1, &accel_token, 1, &num_matches);

  // Not needed anymore
  fpgaDestroyProperties(&filter);

  if (num_matches < 1)
  {
      fprintf(stderr, "Accelerator %s not found!\n", AFU_UUID);
      return 1;
  }

  // Open accelerator
  r = fpgaOpen(accel_token, &handle, 0);
  if (FPGA_OK != r) return 1;

  // Map MMIO space
  r = fpgaMapMMIO(handle, 0, NULL);
  if (FPGA_OK != r) return 1;

  // Done with token
  fpgaDestroyToken(&accel_token);

  // Device Status Memory (DSM)
  dsm.size = DSM_SIZE;

  r = fpgaPrepareBuffer(handle, DSM_SIZE, &dsm.m_virt, &dsm.wsid, 0);
  if (FPGA_OK != r) return 1;

  // Get the physical address of the buffer in the accelerator
  r = fpgaGetIOAddress(handle, dsm.wsid, &dsm.m_phys);
  if (FPGA_OK != r) return 1;

  // ::memset(dsm.m_virt, 0, dsm.size);

  return 0;
}

void* OPAEGenericApp::alloc_buffer(uint64_t size) {
  fpga_result r;

  Buffer buffer;

  uint64_t wsid;
  uint64_t *io_addr;
  void* va;

  buffer.size = size;

  r = fpgaPrepareBuffer(handle, size, &buffer.m_virt, &buffer.wsid, 0);
  if (FPGA_OK != r) return NULL;

  // Get the physical address of the buffer in the accelerator
  r = fpgaGetIOAddress(handle, buffer.wsid, &buffer.m_phys);
  if (FPGA_OK != r) return NULL;

  // ::memset(buffer.m_virt, 0, buffer.size);

  buffers.push_front(buffer);

  return buffer.m_virt;
}

void OPAEGenericApp::delete_buffer() {
  fpgaReleaseBuffer(handle, buffers.front().wsid);

  buffers.pop_front();

  if (buffers.empty())
    this->finish();
}

int OPAEGenericApp::finish() {
  fpgaReleaseBuffer(handle, dsm.wsid);

  fpgaUnmapMMIO(handle, 0);
  fpgaClose(handle);

  return 0;
}

int OPAEGenericApp::program(const char *module) {

  init();

  return 0;
}

int OPAEGenericApp::run() {

  Buffer buffer_in = buffers[0];
  Buffer buffer_out = buffers[1];

  volatile int* StatusAddr =
    (volatile int*) (dsm.m_virt + DSM_STATUS_COMPLETE);

  // Clear the DSM
  ::memset( dsm.m_virt, 0, dsm.size);

  // Initiate DSM Reset
  // Set DSM base, high then low
  fpgaWriteMMIO64(handle, 0, CSR_AFU_DSM_BASEL, dsm.m_phys);

  // Assert AFU reset
  fpgaWriteMMIO64(handle, 0, CSR_CTL, 0);

  //De-Assert AFU reset
  fpgaWriteMMIO64(handle, 0, CSR_CTL, 1);

  // Set the test mode
  fpgaWriteMMIO64(handle, 0, CSR_CFG, 0);

  // Set input workspace address
  fpgaWriteMMIO64(handle, 0, CSR_SRC_ADDR, buffer_in.m_phys/CL(1));

  // Set output workspace address
  fpgaWriteMMIO64(handle, 0, CSR_DST_ADDR, buffer_out.m_phys/CL(1));

  // Set the number of cache lines for the test
  fpgaWriteMMIO64(handle, 0, CSR_NUM_LINES, buffer_in.size/CL(1));

  // Start the test
  fpgaWriteMMIO64(handle, 0, CSR_CTL, 3);

  // Wait for test completion
  while( 0 == ((*StatusAddr)&0x1) ) {
    usleep(100);
  }

  // Stop the device
  fpgaWriteMMIO64(handle, 0, CSR_CTL, 7);

  return 0;
}

