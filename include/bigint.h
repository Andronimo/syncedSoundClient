
#ifndef BIGINT_H
#define BIGINT_H

#define BIGINT_NUMS_SIZE 10

#define BIGINT_OK 				  0u
#define BIGINT_NUMBER_PARSE_ERROR 1u

typedef struct bigint {
	uint8 nums[BIGINT_NUMS_SIZE];
} bigint_t;

extern uint8 bigint_zero(bigint_t* num);
extern void bigint_print(bigint_t* num);
extern void bigint_clone(bigint_t* target, bigint_t* src);
extern void bigint_init(bigint_t* bigint, uint32 num);
extern void bigint_add(bigint_t* sum1, bigint_t* sum2);
extern void bigint_div(bigint_t* dividend, bigint_t* divisor);
extern void bigint_divUint32(bigint_t* dividend, uint32 divisor);
extern void bigint_mult(bigint_t* factor1, bigint_t* factor2);
extern void bigint_multUint32(bigint_t* factor1, uint32);
extern void bigint_divRaw(bigint_t* dividend, uint32 divisor);
extern void bigint_sub(bigint_t* sum1, bigint_t* sum2);
extern void bigint_multSingle(bigint_t* factor1, uint8 factor2);
extern void bigint_shift(bigint_t* bigint, uint8 amount);
extern void bigint_printRaw(bigint_t* num);
extern void bigint_test();
extern uint8 bigint_greatherThan(bigint_t* b1, bigint_t* b2, uint8 equal);
extern uint8 bigint_greatherThanShift(bigint_t* b1, bigint_t* b2, uint8 equal, uint8 shift1, uint8 shift2);
extern void bigint_toString(bigint_t* num, char* out);
extern uint8 bigint_parseString(bigint_t* bigint, char* num);
extern uint32 bigint_toUint32(bigint_t* bigint);
extern void bigint_addUint32(bigint_t* bigint, uint32 summand);
extern void bigint_subUint32(bigint_t* bigint, uint32 summand);

#endif
