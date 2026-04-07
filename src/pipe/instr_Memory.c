/**************************************************************************
 * C S 429 system emulator
 *
 * instr_Memory.c - Memory stage of instruction processing pipeline.
 **************************************************************************/

#include "hw_elts.h"
#include "instr.h"
#include "instr_pipeline.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

extern uint64_t M_PC;

extern comb_logic_t copy_w_ctl_sigs(w_ctl_sigs_t *, w_ctl_sigs_t *);

/*
 * Memory stage logic.
 * STUDENT TO-DO:
 * Implement the memory stage.
 *
 * Use in as the input pipeline register,
 * and update the out pipeline register as output.
 *
 * You will need the following helper functions:
 * copy_w_ctl_signals and dmem.
 */
comb_logic_t memory_instr(m_instr_impl_t *in, w_instr_impl_t *out) {
    // Student TODO
    out->op = in->op;
    out->print_op = in->print_op;
    
    copy_w_ctl_sigs(&out->W_sigs, &in->W_sigs);
    out->dst = in->dst;
    out->val_ex = in->val_ex;

    bool err = false;
    //dmem(out->val_mem, in->val_ex, in->M_sigs.dmem_read, in->M_sigs.dmem_write, &in->val_b, &err);
    //dmem(in->val_ex, in->val_b, in->M_sigs.dmem_read, in->M_sigs.dmem_write, &out->val_mem, &err);
    //dmem(in->val_b, in->val_ex, in->M_sigs.dmem_read, in->M_sigs.dmem_write, &out->val_mem, &err);
    dmem(in->val_ex,             // The address (calculated by ALU)
         in->val_b,              // The value to write (for STUR)
         in->M_sigs.dmem_read,   // Read enable
         in->M_sigs.dmem_write,  // Write enable
         &out->val_mem,          // Pointer to store the read value (for LDUR)
         &err);
    out->status = err ? STAT_ADR : in->status;
    return;
}
