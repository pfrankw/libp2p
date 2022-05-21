#include <p2p/utils.h>

void p2p_utils_string2digest( unsigned char *digest, char *string ){
	int i, k, len;
	char tmp[4];

	len = strlen(string);

	for(i=0, k=0; i<len; i+=2, k++){
		memcpy( tmp, string+i, 2 );
		tmp[2] = 0;
		digest[k] = strtoul( tmp, 0, 16 );
	}

}


void p2p_utils_digest2string( char *string, unsigned char *digest ){
  int i;
  
  string[0] = '\0';
  for(i=0; i<20; i++)
    sprintf(string, "%s%.2x", string, digest[i]);
}
