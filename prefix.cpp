#include <cstddef>

#include "prefix.hpp"
#include "util.hpp"

struct prefix_lut {
  int8_t data[256];
};

static constexpr int opcode_to_prefix_group(uint8_t byte)
{
  int group = -1;

  switch (byte) {
  case 0xF0:                    // LOCK
  case 0xF2:                    // REPNE
  case 0xF3:                    // REP
    group = 0;
    break;
  case 0x2E:                    // CS
  case 0x36:                    // SS
  case 0x3e:                    // DS
  case 0x26:                    // ES
  case 0x64:                    // FS
  case 0x65:                    // GS
    group = 1;
    break;
  case 0x66:                    // operand size override
    group = 2;
    break;
  case 0x67:                    // address size override
    group = 3;
    break;
  case 0x40 ... 0x4F:           // REX prefixes
    group = 4;
    break;
  }

  return group;
}

static constexpr prefix_lut create_prefix_group_lut()
{
  prefix_lut group_lut {};

  for (size_t i = 0; i < array_size(group_lut.data); i++) {
    group_lut.data[i] = (int8_t)opcode_to_prefix_group((uint8_t)i);
  }

  return group_lut;
}

static prefix_lut prefix_group_lut {create_prefix_group_lut()};

bool has_duplicated_prefixes(instruction_bytes const &instr)
{
  // Count prefix occurrence per group.
  int prefix_count[5] {};

  for (uint8_t b : instr.raw) {
    int group = prefix_group_lut.data[b];
    if (group < 0)
      break;

    if (++prefix_count[group] > 1)
      return true;
  }

  return false;
}

// We assume no duplicated prefixes here.
bool has_ordered_prefixes(instruction_bytes const &instr)
{
  // Count prefix occurrence per group.
  uint8_t prefix_pos[5] {};
  bool has_prefix[5] {};
  size_t i = 0;

  for (uint8_t b : instr.raw) {
    int group = prefix_group_lut.data[b];
    if (group < 0)
      break;

    prefix_pos[group] = i++;
    has_prefix[group] = true;
  }

  // TODO There has to be a better way to express this. This also generates an
  // amazing number of conditional jumps with clang. Maybe use non-short-circuit
  // boolean operators.
  for (size_t i = 0; i < array_size(prefix_pos) - 1; i++)
    for (size_t j = i + 1; j < array_size(prefix_pos); j++)
      // Use non-short-circuit operators to reduce the number of conditional
      // jumps.
      if (has_prefix[i] and has_prefix[j] and (prefix_pos[i] > prefix_pos[j]))
        return false;

  return true;

}