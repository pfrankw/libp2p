#ifndef P2P_SSL_H
#define P2P_SSL_H

#include <stdlib.h>

#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include <polarssl/ssl.h>

#include <p2p/err.h>
#include <p2p/rsa.h>
#include <p2p/x509.h>


typedef struct _p2p_ssl_ctx {

  int fd;
  pk_context *pk;
  x509_crt *crt;

  ssl_context ssl;
  entropy_context entropy;
  ctr_drbg_context ctr_drbg;

} p2p_ssl_ctx;


int p2p_ssl_init( p2p_ssl_ctx *ctx, x509_crt *crt, pk_context *pk, int fd, int type );

int p2p_ssl_free( p2p_ssl_ctx *ctx );

int p2p_ssl_send ( p2p_ssl_ctx *ctx, const unsigned char *buf, size_t size );
int p2p_ssl_recv ( p2p_ssl_ctx *ctx, unsigned char *buf, size_t size );



#endif
