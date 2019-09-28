#include "types.h"
#include "stdio.h"
#include "bigint.h"
#include "stdlib.h"
#include "time.h"

uint8 bigint_zero(bigint_t* num) {
	uint8 i;

	for (i = 0u; i < BIGINT_NUMS_SIZE; i++) {
		if (0u != num->nums[i]) {
			return FALSE;
		}
	}
	return TRUE;
}

uint32 bigint_mod(bigint_t* num, uint32 mod) {
	uint8 i;
	uint32 res = 0u;
	uint8 powMod = 1;

	for (i = 0; i < BIGINT_NUMS_SIZE; i++) {
		res += (num->nums[i] % mod) * powMod;
		powMod *= (0x100u % mod);
		powMod %= mod;
	}

	return res % mod;
}

void bigint_toString(bigint_t* num, char* out) {
	char buf[100];
	uint8 i,j;
	bigint_t temp;

	bigint_clone(&temp, num);
	for (i = 0u; (i < 100) && (FALSE == bigint_zero(&temp)); i++) {
		buf[i] = 0x30u + bigint_mod(&temp, 10);
		bigint_divRaw(&temp, 10);
	}

	for (j = 0; j < i; j++) {
		out[j] = buf[i-1-j];
	}
	out[i] = 0u;
}

void bigint_print(bigint_t* num) {
	char buf[100];
	char out[100];
	uint8 i,j;
	bigint_t temp;
	bigint_clone(&temp, num);

	for (i = 0u; (i < 100) && (FALSE == bigint_zero(&temp)); i++) {
		buf[i] = 0x30u + bigint_mod(&temp, 10);
		//printf("%x %x\n", i , buf[i]);
		bigint_divRaw(&temp, 10);
	}

	for (j = 0; j < i; j++) {
		out[j] = buf[i-1-j];
	}
	out[i] = 0u;

	printf("%s\n", out);
}

void bigint_clone(bigint_t* target, bigint_t* src) {
	uint8 i;

	for (i = 0; i < BIGINT_NUMS_SIZE; i++) {
		target->nums[i] = src->nums[i];
	}
}


void bigint_init(bigint_t* bigint, uint32 num) {
	int i;

	bigint->nums[0] =  num        & 0xffu;
	bigint->nums[1] = (num >>  8) & 0xffu;
	bigint->nums[2] = (num >> 16) & 0xffu;
	bigint->nums[3] = (num >> 24) & 0xffu;

	for (i = 4; i < BIGINT_NUMS_SIZE; i++) {
		bigint->nums[i] = 0u;
	}
}

void bigint_add(bigint_t* sum1, bigint_t* sum2)  {
	uint8 carry = 0;
	int i;

	for (i = 0; i < BIGINT_NUMS_SIZE; i++) {
		uint8 temp = sum1->nums[i];
		sum1->nums[i] += sum2->nums[i] + carry;

		if ((sum1->nums[i] >= temp) && (sum1->nums[i] >= sum2->nums[i])) {
			carry = 0u;
		} else {
			carry = 1u;
		}
	}
}

void bigint_sub(bigint_t* minuend, bigint_t* subtrahend)  {
	uint8 carry = 0;
	int i;

	for (i = 0; i < BIGINT_NUMS_SIZE; i++) {
		uint8 temp = minuend->nums[i];
		minuend->nums[i] -= subtrahend->nums[i] + carry;

		if (minuend->nums[i] > temp || ((minuend->nums[i] == temp) && (1u == carry))) {
			carry = 1u;
		} else {
			carry = 0u;
		}
	}
}

void bigint_divRaw(bigint_t* dividend, uint32 divisor) {
	bigint_t divisor_temp;
	bigint_init(&divisor_temp, divisor);
	bigint_div(dividend, &divisor_temp);
}

uint8 bigint_greatherThan(bigint_t* b1, bigint_t* b2, uint8 equal) {
	uint8 i;

	for (i = BIGINT_NUMS_SIZE-1; i != 0xffu; i--) {
		if (b1->nums[i] > b2->nums[i]) {
			return TRUE;
		} else if (b1->nums[i] < b2->nums[i]) {
			return FALSE;
		}
	}

	return equal;
}

uint8 bigint_greatherThanShift(bigint_t* b1, bigint_t* b2, uint8 equal, uint8 shift1, uint8 shift2) {
	uint8 i;

	for (i = BIGINT_NUMS_SIZE-1; i != 0xffu; i--) {
                uint8 j, k;
                
                if (i - shift1 >= 0) {
                    j = b1->nums[i - shift1];
                } else {
                    j = 0;
                }
                
                if (i - shift2 >= 0) {
                    k = b2->nums[i - shift2];
                } else {
                    k = 0;
                }
            
		if (j > k) {
			return TRUE;
		} else if (j < k) {
			return FALSE;
		}
	}

	return equal;
}

void bigint_div(bigint_t* dividend, bigint_t* divisor)  {
	uint8 divisorHighCount = BIGINT_NUMS_SIZE-1;
	uint8 resCount = 0u;
	uint8 res[BIGINT_NUMS_SIZE];
	bigint_t result;
	bigint_init(&result, 0u);

	while (0u == divisor->nums[divisorHighCount] && (divisorHighCount != 0xffu)) divisorHighCount--;

	while (bigint_greatherThan(dividend, divisor, TRUE)) {
		uint8 i = 0u;
                uint8 shift = TRUE;
		uint8 probe = 1u;
		bigint_t bigint_temp, bigint_probe;

		while ((TRUE == shift && (i != 0xffu))) {
                    shift = bigint_greatherThanShift(dividend, divisor, TRUE, 0, i);
                    i++;
                }
                i-=2;

		uint16 divid = (((uint16) dividend->nums[divisorHighCount+i+1]) << 8) | dividend->nums[divisorHighCount+i];
                probe = (uint8) (divid / (divisor->nums[divisorHighCount]+1));
		if ((0u == probe) || divisor->nums[divisorHighCount] == dividend->nums[i]) {
			probe = 1u;
		}

		/* Probedivision funktioniert nach Konstruktion immer, kann noch verbessert werden*/ 
		bigint_init(&bigint_probe, probe);
		bigint_shift(&bigint_probe, i);
		bigint_clone(&bigint_temp, divisor);
		bigint_mult(&bigint_temp, &bigint_probe);

                /* Divisionsteil vom Dividenden abzeiehn und auf Ergebnis addieren*/
                bigint_sub(dividend, &bigint_temp);
		bigint_add(&result, &bigint_probe);
	}

	bigint_clone(dividend, &result);
}

void bigint_multSingle(bigint_t* factor1, uint8 factor2)  {
	uint8 i;
	uint8 carry = 0;

	for (i = 0; i < BIGINT_NUMS_SIZE; i++) {
		uint16 temp = factor1->nums[i] * factor2 + carry;

		factor1->nums[i] = temp & 0xffu;
		carry = (uint8) ((temp >> 8) & 0xffu);
	}
}

void bigint_shift(bigint_t* bigint, uint8 amount)  {
	uint8 i;

	for (i = BIGINT_NUMS_SIZE; i != 0xffu; i--) {
		if (i >= amount) {
			bigint->nums[i] = bigint->nums[i-amount];
		} else {
			bigint->nums[i] = 0u;
		}
	}
}

void bigint_mult(bigint_t* factor1, bigint_t* factor2)  {
	uint8 i;
	bigint_t res;
	bigint_init(&res, 0u);

	for (i = 0; i < BIGINT_NUMS_SIZE; i++) {
		bigint_t temp;
		bigint_clone(&temp, factor1);
		bigint_multSingle(&temp, factor2->nums[i]);
		bigint_shift(&temp, i);
		bigint_add(&res, &temp);
	}

	bigint_clone(factor1, &res);
}

void bigint_printRaw(bigint_t* num) {
	uint8 i;

	for (i = BIGINT_NUMS_SIZE-1; i != 0xffu ; i--) {
		printf("%02x", num->nums[i]);
	}

	printf("\n");
}

uint8 bigint_parseString(bigint_t* bigint, char* num) {
	uint8 i = 0u;
	bigint_t base, mult;
	bigint_init(&base, 1u);
	bigint_init(bigint, 0u);

	while(num[i] != 0u) {
		i++;
		if (i > 20u) {
			return BIGINT_NUMBER_PARSE_ERROR;
		}
	}
	i--;

	bigint_init(bigint, 0);
	while (i != 0xff) {
		if ((num[i] >= 0x30u) && (num[i] <= 0x39u)) {
			bigint_init(&mult, num[i] - 0x30u);
			bigint_mult(&mult, &base);
			bigint_add(bigint, &mult);

			/* Next base 10 */
			bigint_multSingle(&base, 10u);
			i--;
		} else {
			return BIGINT_NUMBER_PARSE_ERROR;
		}
	}

	return BIGINT_OK;
}

void bigint_addUint32(bigint_t* bigint, uint32 summand) {
	bigint_t temp;
	bigint_init(&temp, summand);
	bigint_add(bigint, &temp);
}

void bigint_subUint32(bigint_t* bigint, uint32 summand) {
	bigint_t temp;
	bigint_init(&temp, summand);
	bigint_sub(bigint, &temp);
}

void bigint_divUint32(bigint_t* dividend, uint32 divisor) {
	bigint_t temp;
	bigint_init(&temp, divisor);
	bigint_div(dividend, &temp);
}

void bigint_multUint32(bigint_t* factor1, uint32 factor2) {
	bigint_t temp;
	bigint_init(&temp, factor2);
	bigint_mult(factor1, &temp);
}

uint32 bigint_toUint32(bigint_t* bigint) {
	char out[20];
	bigint_toString(bigint, &out[0]);
	return atol(&out[0]);
}

void bigint_test() {
	bigint_t test, test2;
        uint32 i = 0;
        uint8 ok = TRUE;
        char out[20];
        srand(time(0));
        
        for (i = 1; i < 100; i++) {
            uint32 res;
            uint32 testDividend;
            testDividend = rand()*10000+rand();
            bigint_init(&test, testDividend);
            bigint_init(&test2, i);
            bigint_div(&test, &test2);
            bigint_toString(&test, &out[0]);
            res = atol(&out[0]);
            if (testDividend/i != res) {
                ok = FALSE;
            }
        }

	printf("bigint test: %d\n", ok);
}

