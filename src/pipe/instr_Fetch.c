/**************************************************************************
 * C S 429 system emulator
 *
 * instr_Fetch.c - Fetch stage of instruction processing pipeline.
 **************************************************************************/

#include "hw_elts.h"
#include "instr.h"
#include "instr_pipeline.h"
#include "machine.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>


extern machine_t guest;
extern uint64_t F_PC;

/*
 * Select PC logic.
 * STUDENT TO-DO:
 * Write the next PC to *current_PC.
 */
static comb_logic_t
select_PC(uint64_t pred_PC,                  // The predicted PC
          opcode_t D_opcode, uint64_t val_a, // Possible correction from RET
          uint64_t D_seq_succ,               // this is only used in CBZ/CBNZ EC
          opcode_t M_opcode, bool M_cond_val, // b.cond correction
          uint64_t seq_succ,                  // Possible correction from B.cond
          uint64_t *current_PC) {
    /*
     * Students: Please leave this code
     * at the top of this function.
     * You may modify below it.
     */
    if (D_opcode == OP_RET && val_a == RET_FROM_MAIN_ADDR) {
        *current_PC = 0; // PC can't be 0 normally.
        return;
    }
    // Modify starting here.
    
    // fix mispredicted branch
    if ((M_opcode == OP_B_COND || M_opcode == OP_CBZ || M_opcode == OP_CBNZ) && !M_cond_val) {
        *current_PC = seq_succ;
        return;
    }

    // ret
    if (D_opcode == OP_RET || D_opcode == OP_BR || D_opcode == OP_BLR) {
        *current_PC = val_a;
        return;
    }

    // base case
    *current_PC = pred_PC;
    return;
}

/*
 * Predict PC logic. Conditional branches are predicted taken.
 * STUDENT TO-DO:
 * Write the predicted next PC to *predicted_PC
 * and the next sequential pc to *seq_succ.
 */
static comb_logic_t predict_PC(uint64_t current_PC, uint32_t insnbits,
                               opcode_t op, uint64_t *predicted_PC,
                               uint64_t *seq_succ) {
    /*
     * Students: Please leave this code
     * at the top of this function.
     * You may modify below it.
     */
    if (!current_PC) {
        return; // We use this to generate a halt instruction.
    }
    // Modify starting here.
    *seq_succ = current_PC + 4;
    
    //uncond branch
    int64_t offset;
    if(op == OP_B || op == OP_BL) {
        offset = bitfield_s64(insnbits, 0, 26);
        *predicted_PC = current_PC + (offset << 2);
        return;
    }
    
    // cond branch
    if (op == OP_B_COND || op == OP_CBZ || op == OP_CBNZ){
        offset = bitfield_s64(insnbits, 5, 19);
        *predicted_PC = current_PC + (offset << 2);
        return;
    }
    
    // base case
    *predicted_PC = current_PC + 4;
    return;
}

/*
 * Helper function to recognize the aliased instructions:
 * LSL (RI/RR), LSR (RI/RR), CMP, CMN, and TST. We do this only to simplify the
 * implementations of the shift operations (rather than having
 * to implement UBFM in full).
 * STUDENT TO-DO
 */
static void fix_instr_aliases(uint32_t insnbits, opcode_t *op) {
    // Student TODO
    int num = bitfield_u32(insnbits, 10, 6);
    // ubfm (lsl/lsr ri)
    if (*op == OP_UBFM){
        if(num == 0b111111) {
            *op = OP_LSR_RI;
        } else {
            *op = OP_LSL_RI;
        }
        return;
    }

    // ubfmv (lsl/lsr rr)
    if (*op == OP_UBFMV){
        if (num == 0b001001) {
            *op = OP_LSR_RR;
        } else if(num == 0b001000) {
            *op = OP_LSL_RR;
        } else {
            assert(0);
        }
        return;
    }

    uint8_t dest = bitfield_u32(insnbits, 0, 5);
    // subs (cmp)
    if (*op == OP_SUBS_RR){
        if (dest == 0x1f){
            *op = OP_CMP_RR;
        }
        return;
    }

    // ands (tst)
    if(*op == OP_ANDS_RR) {
        if(dest == 0x1f) {
            *op = OP_TST_RR;
        }
        return;
    }

    // adds (cmn)
    if(*op == OP_ADDS_RR) {
        if(dest == 0x1f) {
            *op = OP_CMN_RR;
        }
        return;
    }

    if (*op == OP_CSEL) {
        if (bitfield_u32(insnbits, 10, 1) == 1) 
            *op = OP_CSINC;
        return;
    }

    if (*op == OP_CSNEG) {
        if (bitfield_u32(insnbits, 10, 1) == 0) 
            *op = OP_CSINV;
        return;
    }

    if(*op == OP_CBNZ) {
        if(bitfield_u32(insnbits, 24, 1) == 0)
            *op = OP_CBZ;
    }

    return;
}

/*
 * Fetch stage logic.
 * STUDENT TO-DO:
 * Implement the fetch stage.
 *
 * Use in as the input pipeline register,
 * and update the out pipeline register as output.
 * Additionally, update PC for the next
 * cycle's predicted PC.
 *
 * You will also need the following helper functions:
 * select_pc, predict_pc, and imem.
 */
comb_logic_t fetch_instr(f_instr_impl_t *in, d_instr_impl_t *out) {
    bool imem_err = 0;
    uint64_t current_PC = 0;

    // Student TODO: Comment this line back in and fill in parameters
    select_PC(in->pred_PC, X_out->op, X_out->val_a, 
                X_out->multipurpose_val.seq_succ_PC, 
                M_out->op, M_out->cond_holds, 
                M_out->multipurpose_val.seq_succ_PC, &current_PC);
    

    /*
     * Students: This case is for generating HLT instructions
     * to stop the pipeline. Only write your code in the **else** case.
     */
    if (!current_PC || F_in->status == STAT_HLT) {
        out->insnbits = 0xD4400000U;
        out->op = OP_HLT;
        out->print_op = OP_HLT;
        out->format = ftable[out->op];
        imem_err = false;
    } else {
        // Student TODO     
        imem(current_PC, &out->insnbits, &imem_err);

        out->op = itable[bitfield_u32(out->insnbits, 21, 11)]; 
        if (out->op == OP_ERROR){
            out->format = FORMAT_ERROR;
        } else {
            out->format = ftable[out->op];
        }
  
        fix_instr_aliases(out->insnbits, &out->op);
        out->print_op = out->op;
        
        predict_PC(current_PC, out->insnbits, out->op, &F_in->pred_PC, &out->multipurpose_val.seq_succ_PC);
        F_PC = F_in->pred_PC;
        
    }

    if (imem_err || out->op == OP_ERROR) {
        in->status = STAT_INS;
    } else if (out->op == OP_HLT) {
        in->status = STAT_HLT;
    } else {
        in->status = STAT_AOK;
    }
    F_in->status = in->status;
    out->status = in->status;
    return;
}
