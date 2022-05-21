#ifndef P2P_UTILS_H
#define P2P_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void p2p_utils_string2digest( unsigned char *digest, char *string );
void p2p_utils_digest2string( char *string, unsigned char *digest );


#endif
