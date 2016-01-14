// pretty simple PSRNG
// using a linear congruential generator, learned it from:
// http://en.wikipedia.org/wiki/Linear_congruential_generator
// Also took these constants from there (from glibc):
#define MODULUS 4294967296LL
#define MULTIPLIER 110351245LL
#define INCREMENT 12345LL

static unsigned long g_seed = 0;

void srand(unsigned long seed) 
{
	g_seed = seed;
}

unsigned long rand()
{
	unsigned long next = ((MULTIPLIER * (signed long)g_seed) + INCREMENT);
	g_seed = next % MODULUS;
	return g_seed;
}

unsigned char rand_byte()
{
	unsigned long v = rand();
	return (v >> 24) & 0xFF;
}

// rand_min: inclusive
// rand_max: exclusive
unsigned char rand_range(unsigned char rand_min, unsigned char rand_max)
{
	unsigned char byte = rand_byte();
	unsigned char range = rand_max - rand_min;
	return (byte % range) + rand_min;
}


/*
#include <stdio.h>
#define TEST_SIZE	1000000
int main(int argc, char **argv)
{
	srand(time(NULL));
	unsigned long i;
	for (i = 0; i < TEST_SIZE; i++)
	printf("%u\n", rand());
	return 0;	
}

>> gcc rand.c
>> ./a.exe | sort | uniq | wc -l
>> 10000000

Well, we're not cycling, so I guess rand is working good
*/
