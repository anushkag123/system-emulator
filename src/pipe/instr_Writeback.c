/**************************************************************************
 * C S 429 architecture emulator
 *
 * instr_Writeback.c - Writeback stage of instruction processing pipeline.
 **************************************************************************/

#include "instr.h"
#include "instr_pipeline.h"
#include "hw_elts.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

extern int64_t W_wval;

/*
 * Write-back stage logic.
 * STUDENT TO-DO:
 * Implement the writeback stage.
 *
 * Use in as the input pipeline register.
 *
 * You will need the global variable W_wval.
 */
comb_logic_t wback_instr(w_instr_impl_t *in) {
    // Student TODO{
        // if(in->W_sigs.dst_sel){
        //     W_wval = 30;
            
        // } else if (in->W_sigs.wval_sel){
        //     W_wval = in->val_mem;
        // } else {
        //     W_wval = in->val_ex;
        // }

    uint8_t dst = in->W_sigs.dst_sel ? 30 : in->dst;
    W_wval = in->W_sigs.wval_sel ? in->val_mem : in->val_ex;
    regfile_write(dst, W_wval, in->W_sigs.w_enable);
    return;
}
