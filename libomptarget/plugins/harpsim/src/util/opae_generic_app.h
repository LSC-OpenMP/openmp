//===--- opae_generic_app.h - AFU Link Interface Implementation --- C++ -*-===//
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

#include <uuid/uuid.h>
#include <opae/fpga.h>
#include <cstring>
#include <deque>
#include <unistd.h>

#define AFU_UUID "C000C966-0D82-4272-9AEF-FE5F84570612"

#define CACHELINE_BYTES 64
#define CL(x) ((x) * CACHELINE_BYTES)
#define MB(x) ((x) * 1024 * 1024)

#define DSM_SIZE                 MB(4)
#define DSM_STATUS_COMPLETE      0x40
#define CSR_SRC_ADDR             0x0120
#define CSR_DST_ADDR             0x0128
#define CSR_CTL                  0x0138
#define CSR_CFG                  0x0140
#define CSR_NUM_LINES            0x0130
#define CSR_AFU_DSM_BASEL        0x0110
#define CSR_AFU_DSM_BASEH        0x0114

struct Buffer {
  size_t     size;   /// workspace size in bytes.
  void*      m_virt; /// workspace virtual address.
  uint64_t   m_phys; /// workspace physical address.
  uint64_t   wsid;
};

class OPAEGenericApp{
private:
  Buffer dsm;
  fpga_handle handle;
  std::deque<Buffer> buffers;

public:

  OPAEGenericApp();
  ~OPAEGenericApp();

  int init();
  int finish();
  void* alloc_buffer(uint64_t size);
  void delete_buffer();

  int run();    ///< Return 0 if success
  int program(const char *module);
};

