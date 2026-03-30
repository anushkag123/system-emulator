/**************************************************************************
 * C S 429 system emulator
 *
 * hw_elts.c - Module for emulating hardware elements.
 *
 * Copyright (c) 2022, 2023, 2024, 2025.
 * Authors: S. Chatterjee, Z. Leeper., P. Jamadagni, W. Borden.
 * All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/

#include "hw_elts.h"
#include "err_handler.h"
#include "instr.h"
#include "instr_pipeline.h"
#include "machine.h"
#include "mem.h"
#include <assert.h>
#include <stdint.h>

extern machine_t guest;

fa_t rca[64];

/*
 * Read from instruction memory. Students, do not modify.
 */
comb_logic_t imem(uint64_t imem_addr, uint32_t *imem_rval, bool *imem_err) {
    // imem_addr must be in "instruction memory" and a multiple of 4
    *imem_err = (!addr_in_imem(imem_addr) || (imem_addr & 0x3U));
    *imem_rval = (uint32_t) mem_read_I(imem_addr);
}

/*
 * Sets up the inputs to the ripple carry adder.
 * STUDENT TO-DO:
 */
comb_logic_t init_rca(uint64_t val_a, uint64_t val_b, bool c_in) {
    /* your implementation */
    for (int i = 0; i < 64; i++){
        rca[i].c_in = i == 0 ? c_in : rca[i - 1].c_out;
        rca[i].bit_a = val_a & 1;
        rca[i].bit_b = val_b & 1;
        rca[i].c_out = (rca[i].bit_a & rca[i].bit_b) | (rca[i].c_in & (rca[i].bit_a ^ rca[i].bit_b));
        val_a = val_a >> 1;
        val_b = val_b >> 1;
    }
    return;
}

/*
 * Performs the ripple carry add.
 * STUDENT TO-DO:
 */
comb_logic_t ripple_carry_add(uint64_t *sum) {
    /* your implementation */
    *sum = 0;
    for (int i = 63; i >= 0; i--){
        *sum = *sum << 1;
        *sum += rca[i].bit_a ^ rca[i].bit_b ^ rca[i].c_in; 
    }
    return;
}

/*
 * Read from register file.
 * STUDENT TO-DO:
 * Read from src1 and src2 registers. Take extra care for SP/XZR.
 */
comb_logic_t regfile_read(uint8_t src1, uint8_t src2, uint64_t *val_a,
                          uint64_t *val_b) {
    /* your implementation */
    if(src1 == XZR_NUM) {
        *val_a = 0;
    } else if (src1 == SP_NUM) {
        *val_a = guest.proc->SP;
    } else {
        *val_a = guest.proc->GPR[src1];
    }

    if(src2 == XZR_NUM) {
        *val_b = 0;
    } else if (src2 == SP_NUM) {
        *val_b = guest.proc->SP;
    } else {
        *val_b = guest.proc->GPR[src2];
    }
    return;
}

/*
 * Write to register file.
 * STUDENT TO-DO:
 * Write to dst register if enabled. Take extra care for SP/XZR.
 */
comb_logic_t regfile_write(uint8_t dst, uint64_t val_w, bool w_enable) {
    if(!w_enable || dst == XZR_NUM || dst == SP_NUM) {
        return;
    }

    guest.proc->GPR[dst] = val_w;
    return;
}

/*
 * Check whether a condition is satisfied given the NCZV status flags.
 * STUDENT TO-DO:
 */
static bool cond_holds(cond_t cond, uint8_t flags) {
    /* your implementation */
    uint8_t n = GET_NF(flags);
    uint8_t z = GET_ZF(flags);
    uint8_t c = GET_CF(flags);
    uint8_t v = GET_VF(flags);

    switch (cond) {
        case C_EQ:
            if(z == 1) return true;
            return false;
        case C_NE:
            if (z == 0) return true;
            return false;
        case C_CS:
            if (c == 1) return true;
            return false;
        case C_CC:
            if (c == 0) return true;
            return false;
        case C_MI:
            if (n == 1) return true;
            return false;
        case C_PL:
            if (n == 0) return true;
            return false;
        case C_VS:
            if (v == 1) return true;
            return false;
        case C_VC:
            if (v == 0) return true;
            return false;
        case C_HI:
            if (c == 1 && z == 0) return true;
            return false;
        case C_LS:
            if (!(c == 1 && z == 0)) return true;
            return false;
        case C_GE:
            if(n == v)  return true;
            return false;
        case C_LT:
            if(!(n == v)) return true;
            return false;
        case C_GT:
            if(z == 0 && n == v) return true;
            return false;
        case C_LE:
            if(!(z == 0 && n == v)) return true;
            return false;
        case C_AL:
        case C_NV:
            return true;
        default:
            return false;
    }
}

/*
 * Perform the appropriate ALU operation, setting NZCV flags if needed.
 * STUDENT TO-DO:
 */
comb_logic_t alu(uint64_t alu_vala, uint64_t alu_valb, uint8_t alu_valhw,
                 uint8_t nzcv, alu_op_t ALUop, bool set_flags, cond_t cond,
                 uint64_t *val_e, bool *cond_val, uint8_t *nzcv_dst) {
    /* your implementation */
    uint8_t n = 0;
    uint8_t z = 0;
    uint8_t c = 0;
    uint8_t v = 0;
    switch (ALUop) {
        case PLUS_OP:   // vala + valb
            init_rca(alu_vala, alu_valb, false);
            ripple_carry_add(val_e);  
            c = rca[63].c_out;
            v = rca[63].c_in ^ rca[63].c_out;
            break;

        case MINUS_OP:  // vala - valb
            init_rca(alu_vala, ~alu_valb, true);
            ripple_carry_add(val_e);
            c = rca[63].c_out;
            v = rca[63].c_in ^ rca[63].c_out;
            break;

        case INV_OP:    
            *val_e = alu_vala | (~alu_valb);
            break;
        
        case OR_OP:     
            *val_e = alu_vala | alu_valb;
            break;

        case EOR_OP:
            *val_e = alu_vala ^ alu_valb;
            break;

        case AND_OP:
            *val_e = alu_vala & alu_valb;
            break;
            
        case MOV_OP:
            *val_e = alu_vala | (alu_valb << alu_valhw);
            break;
            
        case MOVK_OP:
            *val_e = (alu_vala & (~(0xFFFFUL << alu_valhw))) | (alu_valb << alu_valhw);
            break;

        case LSL_OP:
            *val_e = alu_vala << (alu_valb & 0x3FUL);
            break;

        case LSR_OP:
            *val_e = alu_vala >> (alu_valb & 0x3FUL);
            break;
            
        case ASR_OP:
            *val_e = (int64_t) alu_vala >> (alu_valb & 0x3FUL);
            break;

        case PASS_A_OP:
            *val_e = alu_vala;
            break;
        
        default:
            break;
    }

    if (set_flags) {
        if (*val_e == 0) z = 1;
        if (*val_e >> 63 == 1) n = 1;
        *nzcv_dst = PACK_CC(n, z, c, v);
    } else {
        *nzcv_dst = nzcv;
    }

    *cond_val = cond_holds(cond, nzcv);
    return;
}

/*
 * Read from data memory, Students do not modify.
 */
comb_logic_t dmem(uint64_t dmem_addr, uint64_t dmem_wval, bool dmem_read,
                  bool dmem_write, uint64_t *dmem_rval, bool *dmem_err) {
    if (!dmem_read && !dmem_write) {
        return;
    }
    // dmem_addr must be in "data memory" and a multiple of 8
    *dmem_err = (!addr_in_dmem(dmem_addr) || (dmem_addr & 0x7U));
    if (is_special_addr(dmem_addr))
        *dmem_err = false;
    if (dmem_read)
        *dmem_rval = (uint64_t) mem_read_L(dmem_addr);
    if (dmem_write)
        mem_write_L(dmem_addr, dmem_wval);
}
