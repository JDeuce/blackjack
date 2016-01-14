#ifndef _RAND_H
#define _RAND_H

void srand(unsigned long seed);
unsigned long rand();
unsigned char rand_byte();
unsigned char rand_range(unsigned char rand_min, unsigned char rand_max);
#endif
