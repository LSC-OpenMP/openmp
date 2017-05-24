//===-- elf_common.c - Common ELF functionality -------------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// Common ELF functionality for target plugins.
// Must be included in the plugin source file AFTER omptarget.h has been
// included and macro DP(...) has been defined.
// .
//
//===----------------------------------------------------------------------===//

#if !(defined(_OMPTARGET_H_) && defined(DP))
#error Include elf_common.c in the plugin source AFTER omptarget.h has been\
 included and macro DP(...) has been defined.
#endif

#include <elf.h>
#include <libelf.h>

static inline int32_t dump_data(Elf_Scn* scn, GElf_Shdr shdr) {
  size_t n = 0;
  Elf_Data *data = NULL;

  __tgt_configuration *cfg;

  while ( (n < shdr.sh_size) &&
          (data = elf_getdata(scn, data)) != NULL) {
    cfg = (__tgt_configuration*) data->d_buf;
    DP("env_id = %d\n", cfg->env_id);
  }

  return 1;
}

static inline int32_t dump_section(Elf* e, Elf_Scn* scn) {
  GElf_Shdr shdr;

  if (gelf_getshdr(scn , &shdr) != &shdr) {
    DP("Could not get section data\n");

    return 0;
  }

  if (shdr.sh_type == SHT_NOBITS)
    return 1;
  return dump_data(scn, shdr);
}

static inline int32_t dump_section_by_name(Elf* e, const char* section_name) {
  // Find the rodata section offset
  Elf_Scn *section = 0;
  Elf64_Off rodata_offset = 0;

  size_t shstrndx;

  if (elf_getshdrstrndx(e, &shstrndx)) {
    DP("Unable to get ELF strings index!\n");

    return 0;
  }

  while ((section = elf_nextscn(e, section))) {
    GElf_Shdr hdr;
    gelf_getshdr(section, &hdr);

    if (!strcmp(elf_strptr(e, shstrndx, hdr.sh_name), section_name)) {
      DP("found section name: %s\n", section_name);

      rodata_offset = hdr.sh_addr;

      return dump_section(e, section);
    }
  }

  return 0;
}

// Check whether an image is valid for execution on target_id
static inline int32_t elf_check_machine(__tgt_device_image *image,
    uint16_t target_id) {

  // Is the library version incompatible with the header file?
  if (elf_version(EV_CURRENT) == EV_NONE) {
    DP("Incompatible ELF library!\n");
    return 0;
  }

  char *img_begin = (char *)image->ImageStart;
  char *img_end = (char *)image->ImageEnd;
  size_t img_size = img_end - img_begin;

  // Obtain elf handler
  Elf *e = elf_memory(img_begin, img_size);
  if (!e) {
    DP("Unable to get ELF handle: %s!\n", elf_errmsg(-1));
    return 0;
  }

  // Check if ELF is the right kind.
  if (elf_kind(e) != ELF_K_ELF) {
    DP("Unexpected ELF type!\n");
    return 0;
  }
  Elf64_Ehdr *eh64 = elf64_getehdr(e);
  Elf32_Ehdr *eh32 = elf32_getehdr(e);

  if (!eh64 && !eh32) {
    DP("Unable to get machine ID from ELF file!\n");
    elf_end(e);
    return 0;
  }

  uint16_t MachineID;
  if (eh64 && !eh32)
    MachineID = eh64->e_machine;
  else if (eh32 && !eh64)
    MachineID = eh32->e_machine;
  else {
    DP("Ambiguous ELF header!\n");
    elf_end(e);
    return 0;
  }

  dump_section_by_name(e, ".omp_offloading.configuration");

  elf_end(e);
  return MachineID == target_id;
}
