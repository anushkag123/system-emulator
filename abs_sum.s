.arch armv8-a
.text
.align 2
.global abs_sum
.type abs_sum, %function

// Compute the sum of the absolute value of values in the array.
// x0: address of long array[32]
// x1: size of array (32)
abs_sum:
    movz    x2, #0              // Total sum
    movz    x5, #2              // Decrement by 2
    movz    x9, #1
    
    cmp     x1, xzr             // Initial check
    b.eq    .done

    /* Odd case */
    ands    xzr, x1, x9
    b.eq    .loop

    ldur    x3, [x0, #0]
    add     x0, x0, #8
    asr     x4, x3, #63
    eor     x3, x3, x4
    subs     x3, x3, x4
    adds     x2, x2, x3
    subs    x1, x1, x9

    cmp     x1, xzr
    b.eq    .done

.loop:
    /* Load */
    ldur    x3, [x0, #0]        // Element A
    ldur    x10, [x0, #8]       // Element B
    add     x0, x0, #16
    
    /* Absolute value for x3 */
    asr     x4, x3, #63         // Create sign mask
    eor     x3, x3, x4          // Invert bits if negative
    subs    x3, x3, x4
    
    /* Absolute value for x10 */
    asr     x11, x10, #63       // Create sign mask
    eor     x10, x10, x11       // Invert bits if negative
    subs    x10, x10, x11

    /* Accumulate both */
    adds    x2, x2, x3          //Update sum
    adds    x2, x2, x10
    
    subs    x1, x1, x5          // Decrement counter
    b.gt    .loop

.done:
    adds    x0, x2, xzr         // Move sum into x0
    ret
.size   abs_sum, .-abs_sum
