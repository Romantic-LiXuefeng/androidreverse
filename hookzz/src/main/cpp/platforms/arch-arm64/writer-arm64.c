#include "writer-arm64.h"
#include <stdlib.h>
#include <string.h>

// REF:
// ARM Architecture Reference Manual ARMV8
// C2.1 Understanding the A64 instruction descriptions
// C2.1.3 The instruction encoding or encodings

// ATTENTION !!!:
// 写 writer 部分, 需要参考, `Instrcution Set Encoding` 部分
// `witer` REF: `ZzInstruction Set Encoding`

ZzARM64AssemblerWriter *zz_arm64_writer_new(zz_ptr_t data_ptr) {
    ZzARM64AssemblerWriter *writer = (ZzARM64AssemblerWriter *)zz_malloc_with_zero(sizeof(ZzARM64AssemblerWriter));
    zz_addr_t align_address        = (zz_addr_t)data_ptr & ~(zz_addr_t)3;
    writer->w_current_address      = align_address;
    writer->w_start_address        = align_address;
    writer->current_pc             = align_address;
    writer->start_pc               = align_address;
    writer->size                   = 0;
    writer->insn_size              = 0;
    return writer;
}

void zz_arm64_writer_init(ZzARM64AssemblerWriter *self, zz_ptr_t data_ptr, zz_addr_t target_ptr) {
    zz_arm64_writer_reset(self, data_ptr, target_ptr);
}

void zz_arm64_writer_reset(ZzARM64AssemblerWriter *self, zz_ptr_t data_ptr, zz_addr_t target_ptr) {
    zz_addr_t align_address        = (zz_addr_t)data_ptr & ~(zz_addr_t)3;
    zz_addr_t target_align_address = (zz_addr_t)target_ptr & ~(zz_addr_t)3;
    self->w_current_address        = align_address;
    self->w_start_address          = align_address;
    self->current_pc               = target_align_address;
    self->start_pc                 = target_align_address;
    self->size                     = 0;

    if (self->insn_size) {
        for (int i = 0; i < self->insn_size; ++i) {
            free(self->insns[i]);
        }
    }
    self->insn_size = 0;
}

void zz_arm64_writer_free(ZzARM64AssemblerWriter *self) {
    if (self->insn_size) {
        for (int i = 0; i < self->insn_size; i++) {
            free(self->insns[i]);
        }
    }
    free(self);
}

// ======= user custom =======

void zz_arm64_writer_put_ldr_br_reg_address(ZzARM64AssemblerWriter *self, ZzARM64Reg reg, zz_addr_t address) {
    zz_arm64_writer_put_ldr_reg_imm(self, reg, 0x8);
    zz_arm64_writer_put_br_reg(self, reg);
    zz_arm64_writer_put_bytes(self, (zz_ptr_t)&address, sizeof(zz_ptr_t));
}

void zz_arm64_writer_put_ldr_blr_b_reg_address(ZzARM64AssemblerWriter *self, ZzARM64Reg reg, zz_addr_t address) {
    zz_arm64_writer_put_ldr_reg_imm(self, reg, 0xc);
    zz_arm64_writer_put_blr_reg(self, reg);
    zz_arm64_writer_put_b_imm(self, 0xc);
    zz_arm64_writer_put_bytes(self, (zz_ptr_t)&address, sizeof(zz_ptr_t));
}

void zz_arm64_writer_put_ldr_b_reg_address(ZzARM64AssemblerWriter *self, ZzARM64Reg reg, zz_addr_t address) {
    zz_arm64_writer_put_ldr_reg_imm(self, reg, 0x8);
    zz_arm64_writer_put_b_imm(self, 0xc);
    zz_arm64_writer_put_bytes(self, (zz_ptr_t)&address, sizeof(address));
}

zz_size_t zz_arm64_writer_near_jump_range_size() { return ((1 << 25) << 2); }

void zz_arm64_writer_put_ldr_br_b_reg_address(ZzARM64AssemblerWriter *self, ZzARM64Reg reg, zz_addr_t address) {
    zz_arm64_writer_put_ldr_reg_imm(self, reg, 0xc);
    zz_arm64_writer_put_br_reg(self, reg);
    zz_arm64_writer_put_b_imm(self, 0xc);
    zz_arm64_writer_put_bytes(self, (zz_ptr_t)&address, sizeof(address));
}

// ======= default =======

void zz_arm64_writer_put_bytes(ZzARM64AssemblerWriter *self, char *data, zz_size_t data_size) {
    memcpy((zz_ptr_t)self->w_current_address, data, data_size);
    self->w_current_address = self->w_current_address + data_size;
    self->current_pc += data_size;
    self->size += data_size;

    ZzARM64Instruction *arm64_insn = (ZzARM64Instruction *)zz_malloc_with_zero(sizeof(ZzARM64Instruction));
    arm64_insn->pc                 = self->current_pc - data_size;
    arm64_insn->address            = self->w_current_address - data_size;
    arm64_insn->size               = data_size;
    arm64_insn->insn               = 0;
    self->insns[self->insn_size++] = arm64_insn;
}

void zz_arm64_writer_put_instruction(ZzARM64AssemblerWriter *self, uint32_t insn) {
    *(uint32_t *)(self->w_current_address) = insn;
    self->w_current_address                = self->w_current_address + sizeof(uint32_t);
    self->current_pc += 4;
    self->size += 4;

    ZzARM64Instruction *arm64_insn = (ZzARM64Instruction *)zz_malloc_with_zero(sizeof(ZzARM64Instruction));
    arm64_insn->pc                 = self->current_pc - 4;
    arm64_insn->address            = self->w_current_address - 4;
    arm64_insn->size               = 4;
    arm64_insn->insn               = insn;
    self->insns[self->insn_size++] = arm64_insn;
}

void zz_arm64_writer_put_ldr_reg_imm(ZzARM64AssemblerWriter *self, ZzARM64Reg reg, uint32_t offset) {
    ZzARM64RegInfo ri;
    zz_arm64_register_describe(reg, &ri);

    uint32_t imm19, Rt_ndx;

    imm19  = offset >> 2;
    Rt_ndx = ri.index;

    zz_arm64_writer_put_instruction(self, 0x58000000 | imm19 << 5 | Rt_ndx);
    return;
}

// PAGE: C6-871
void zz_arm64_writer_put_str_reg_reg_offset(ZzARM64AssemblerWriter *self, ZzARM64Reg src_reg, ZzARM64Reg dst_reg,
                                            uint64_t offset) {
    ZzARM64RegInfo rs, rd;

    zz_arm64_register_describe(src_reg, &rs);
    zz_arm64_register_describe(dst_reg, &rd);

    uint32_t size, v = 0, opc = 0, Rn_ndx, Rt_ndx;
    Rn_ndx = rd.index;
    Rt_ndx = rs.index;

    if (rs.is_integer) {
        size = (rs.width == 64) ? 0b11 : 0b10;
    }

    uint32_t imm12 = offset >> size;

    zz_arm64_writer_put_instruction(self, 0x39000000 | size << 30 | opc << 22 | imm12 << 10 | Rn_ndx << 5 | Rt_ndx);
    return;
}

void zz_arm64_writer_put_ldr_reg_reg_offset(ZzARM64AssemblerWriter *self, ZzARM64Reg dst_reg, ZzARM64Reg src_reg,
                                            uint64_t offset) {
    ZzARM64RegInfo rs, rd;

    zz_arm64_register_describe(src_reg, &rs);
    zz_arm64_register_describe(dst_reg, &rd);

    uint32_t size, v = 0, opc = 0b01, Rn_ndx, Rt_ndx;
    Rn_ndx = rs.index;
    Rt_ndx = rd.index;

    if (rs.is_integer) {
        size = (rs.width == 64) ? 0b11 : 0b10;
    }

    uint32_t imm12 = offset >> size;

    zz_arm64_writer_put_instruction(self, 0x39000000 | size << 30 | opc << 22 | imm12 << 10 | Rn_ndx << 5 | Rt_ndx);
    return;
}

// C6-562
void zz_arm64_writer_put_br_reg(ZzARM64AssemblerWriter *self, ZzARM64Reg reg) {
    ZzARM64RegInfo ri;
    zz_arm64_register_describe(reg, &ri);

    uint32_t op = 0, Rn_ndx;
    Rn_ndx      = ri.index;
    zz_arm64_writer_put_instruction(self, 0xd61f0000 | op << 21 | Rn_ndx << 5);
    return;
}

// C6-561
void zz_arm64_writer_put_blr_reg(ZzARM64AssemblerWriter *self, ZzARM64Reg reg) {
    ZzARM64RegInfo ri;
    zz_arm64_register_describe(reg, &ri);

    uint32_t op = 0b01, Rn_ndx;

    Rn_ndx = ri.index;

    zz_arm64_writer_put_instruction(self, 0xd63f0000 | op << 21 | Rn_ndx << 5);
    return;
}

// C6-550
void zz_arm64_writer_put_b_imm(ZzARM64AssemblerWriter *self, uint64_t offset) {
    uint32_t op = 0b0, imm26;
    imm26       = (offset >> 2) & 0x03ffffff;
    zz_arm64_writer_put_instruction(self, 0x14000000 | op << 31 | imm26);
    return;
}

// TODO: standard form, need fix others
// PAGE: C6-549
void zz_arm64_writer_put_b_cond_imm(ZzARM64AssemblerWriter *self, uint32_t condition, uint64_t imm) {
    uint32_t imm19, cond;
    cond  = condition;
    imm19 = (imm >> 2) & 0x7ffff;
    zz_arm64_writer_put_instruction(self, 0x54000000 | imm19 << 5 | cond);
    return;
}

// C6-525
void zz_arm64_writer_put_add_reg_reg_imm(ZzARM64AssemblerWriter *self, ZzARM64Reg dst_reg, ZzARM64Reg left_reg,
                                         uint64_t imm) {
    ZzARM64RegInfo rd, rl;

    zz_arm64_register_describe(dst_reg, &rd);
    zz_arm64_register_describe(left_reg, &rl);

    uint32_t sf = 1, op = 0, S = 0, shift = 0b00, imm12, Rn_ndx, Rd_ndx;

    Rd_ndx = rd.index;
    Rn_ndx = rl.index;
    imm12  = imm & 0xFFF;

    zz_arm64_writer_put_instruction(self, 0x11000000 | sf << 31 | op << 30 | S << 29 | shift << 22 | imm12 << 10 |
                                              Rn_ndx << 5 | Rd_ndx);
    return;
}

// C6-930
void zz_arm64_writer_put_sub_reg_reg_imm(ZzARM64AssemblerWriter *self, ZzARM64Reg dst_reg, ZzARM64Reg left_reg,
                                         uint64_t imm) {
    ZzARM64RegInfo rd, rl;

    zz_arm64_register_describe(dst_reg, &rd);
    zz_arm64_register_describe(left_reg, &rl);

    uint32_t sf = 1, op = 1, S = 0, shift = 0b00, imm12, Rn_ndx, Rd_ndx;

    Rd_ndx = rd.index;
    Rn_ndx = rl.index;
    imm12  = imm & 0xFFF;

    zz_arm64_writer_put_instruction(self, 0x11000000 | sf << 31 | op << 30 | S << 29 | shift << 22 | imm12 << 10 |
                                              Rn_ndx << 5 | Rd_ndx);
    return;
}