#ifndef P2P_RSA_H
#define P2P_RSA_H
#include <string.h>

#include <polarssl/sha1.h>
#include <polarssl/pk.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>

int p2p_rsa_key_gen(pk_context *key, int bits);
int p2p_rsa_key_exp(pk_context *key, unsigned char *buf, size_t size);
int p2p_rsa_key_write(pk_context *key, char *output_file);
int p2p_rsa_key_digest(pk_context *key, unsigned char *digest);


#endif
