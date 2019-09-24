
#ifndef BIGINT_H
#define BIGINT_H

#define BIGINT_NUMS_SIZE 10

typedef struct bigint {
	uint8 nums[BIGINT_NUMS_SIZE];
} bigint_t;

extern uint8 bigint_zero(bigint_t* num);
extern void bigint_print(bigint_t* num);
extern void bigint_clone(bigint_t* target, bigint_t* src);
extern void bigint_init(bigint_t* bigint, uint32 num);
extern void bigint_add(bigint_t* sum1, bigint_t* sum2);
extern void bigint_div(bigint_t* dividend, bigint_t* divisor);
extern void bigint_mult(bigint_t* factor1, bigint_t* factor2);
extern void bigint_divRaw(bigint_t* dividend, uint32 divisor);
extern void bigint_sub(bigint_t* sum1, bigint_t* sum2);
extern void bigint_multSingle(bigint_t* factor1, uint8 factor2);
extern void bigint_shift(bigint_t* bigint, uint8 amount);
extern void bigint_printRaw(bigint_t* num);
extern void bigint_test();
extern uint8 bigint_greatherThan(bigint_t* b1, bigint_t* b2, uint8 equal);

#endif
