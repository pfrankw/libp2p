#ifndef P2P_X509_H
#define P2P_X509_H

#include <p2p/err.h>


#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include <polarssl/x509.h>
#include <polarssl/x509_crt.h>
#include <polarssl/sha1.h>


int p2p_x509_cert_gen(x509_crt *crt, pk_context *key);
int p2p_x509_cert_digest(x509_crt *crt, unsigned char digest[20]);

#endif
