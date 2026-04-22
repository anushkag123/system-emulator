.arch armv8-a
.text
.align 2
.global abs_sum
.type abs_sum, %function

// Compute the sum of the absolute value of values in the array.
// x0: address of long array[32]
// x1: size of array (32)
abs_sum:
    // Modify below here
    movz    x2, #0              // Running sum.
    movz    x5, #1
    cmp     x1, xzr             // Check if there are elements left in array to sum.
    b.eq    .done              // If not, we are done.

.loop:
    ldur    x3, [x0, #0]        // Load element from memory.
    add     x0, x0, #8

    asr     x4, x3, #63
    eor     x3, x3, x4
    subs     x3, x3, x4
    
    adds     x2, x2, x3

    subs    x1, x1, x5
    b.ne    .loop               // Continue loop.

.done:
    adds    x0, x2, xzr         // Move sum into x0.
    ret
.size   abs_sum, .-abs_sum
