#ifndef P2P_BASE32_H
#define P2P_BASE32_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <p2p/err.h>



#define BASE32_CHARS "abcdefghijklmnopqrstuvwxyz234567"

#define SIZE_T_CEILING  ((size_t)(INT32_MAX-16))

int p2p_base32_encode(char *buf, size_t size, unsigned char *data, size_t data_size);


#endif
