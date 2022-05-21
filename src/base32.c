#include <p2p/base32.h>


int p2p_base32_encode(char *buf, size_t size, unsigned char *data, size_t data_size){

  int ret = 0;
  unsigned int i, v, u;
  size_t nbits = data_size * 8, bit;

  if( data_size >= SIZE_T_CEILING/8 ){
    ret = P2P_ERR_GENERIC;fprintf(stderr, "BELLA");
    goto exit;
  }

  if( (nbits%5) != 0 ){
    ret = P2P_ERR_GENERIC;
    goto exit;
  }

  if( (nbits/5)+1 > size ){
    ret = P2P_ERR_GENERIC;
    goto exit;
  }

  if( size >= SIZE_T_CEILING ){
    ret = P2P_ERR_GENERIC;
    goto exit;
  }


  for (i=0,bit=0; bit < nbits; ++i, bit+=5) {
    /* set v to the 16-bit value starting at src[bits/8], 0-padded. */
    v = ((unsigned char)data[bit/8]) << 8;
    if (bit+5<nbits) v += (unsigned char)data[(bit/8)+1];
    /* set u to the 5-bit value at the bit'th bit of src. */
    u = (v >> (11-(bit%8))) & 0x1F;
    buf[i] = BASE32_CHARS[u];
  }
  buf[i] = '\0';

exit:
  return ret;

}
