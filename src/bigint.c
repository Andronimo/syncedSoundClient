#include "types.h"
#include "stdio.h"
#include "bigint.h"

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

void bigint_print(bigint_t* num) {
	char buf[100];
	char out[100];
	uint8 i,j;

	for (i = 0u; (i < 100) && (FALSE == bigint_zero(num)); i++) {
		buf[i] = 0x30u + bigint_mod(num, 10);
		printf("%x %x\n", i , buf[i]);
		bigint_divRaw(num, 10);
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


void bigint_div(bigint_t* dividend, bigint_t* divisor)  {
	uint8 j = BIGINT_NUMS_SIZE-1;
	uint8 resCount = 0u;
	uint8 res[BIGINT_NUMS_SIZE];
	bigint_t result;
	bigint_init(&result, 0u);

	while (0u == divisor->nums[j] && (j != 0xffu)) j--;

	while (bigint_greatherThan(dividend, divisor, TRUE)) {
		int i = BIGINT_NUMS_SIZE-2;
		uint8 probe = 1u;
		bigint_t bigint_temp, bigint_probe;

		while ((divisor->nums[j] > dividend->nums[i]) && (0u == dividend->nums[i+1]) && (i != 0xffu)) i--;

		uint16 divid = (((uint16) dividend->nums[i+1]) << 8) | dividend->nums[i];
		probe = (uint8) (divid / divisor->nums[j] / 2);
		if ((0u == probe) || divisor->nums[j] == dividend->nums[i]) {
			probe = 1u;
		}
		printf("i:%d j:%d %d %d %d\n",i, j, divid, divisor->nums[j], probe);

		bigint_init(&bigint_probe, probe);
		bigint_shift(&bigint_probe, i-j);
		bigint_clone(&bigint_temp, divisor);
		bigint_mult(&bigint_temp, &bigint_probe);
		bigint_printRaw(&bigint_probe);

		bigint_sub(dividend, &bigint_temp);
		bigint_add(&result, &bigint_probe);
		bigint_printRaw(&bigint_probe);

		//printf("Dividend: ");
		//bigint_printRaw(dividend);
		//printf("Divisor:  ");
		//bigint_printRaw(divisor);
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

void bigint_test() {
	bigint_t test, test2;
	bigint_init(&test, 145678911);
	bigint_init(&test2, 123123);

	bigint_printRaw(&test);
	bigint_printRaw(&test2);
    printf("\n");

	bigint_div(&test, &test2);

	bigint_printRaw(&test);


	//bigint_print(&test);
}

