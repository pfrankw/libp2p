#include <p2p/ssl.h>


int p2p_ssl_init( p2p_ssl_ctx *ctx, x509_crt *crt, pk_context *pk, int fd, int type ){

  int ret = 0;

  if( !ctx ){
    ret = P2P_ERR_NULL_PARAM;
    goto exit;
  }

  ctx->pk = pk;
  ctx->crt = crt;
  ctx->fd = fd;


  entropy_init( &ctx->entropy );

  if( ( ret = ctr_drbg_init( &ctx->ctr_drbg, entropy_func, &ctx->entropy, 0, 0 ) ) != 0 )
    goto exit;

  memset( &ctx->ssl, 0, sizeof(ssl_context) );
  if( ( ret = ssl_init(&ctx->ssl) ) != 0 )
    goto exit;

  ssl_set_ciphersuites( &ctx->ssl, ssl_list_ciphersuites() );
  ssl_set_rng( &ctx->ssl, ctr_drbg_random, &ctx->ctr_drbg );
  ssl_set_endpoint( &ctx->ssl, type );
  ssl_set_authmode( &ctx->ssl, SSL_VERIFY_NONE );
  ssl_set_min_version( &ctx->ssl, SSL_MAJOR_VERSION_3, SSL_MINOR_VERSION_1 );
  ssl_set_arc4_support( &ctx->ssl, SSL_ARC4_DISABLED ); /* Disabling rc4, it is ultra-unsafe */
  if( !pk || !crt || ( ret = ssl_set_own_cert( &ctx->ssl, ctx->crt, ctx->pk ) ) != 0 ) /* Setting own certificate and private key. NOTE: the certificate is generated at runtime */
    goto exit;

  ssl_set_bio( &ctx->ssl, net_recv, &ctx->fd, net_send, &ctx->fd );
  while( ( ret = ssl_handshake( &ctx->ssl ) ) != 0 );

  ret = 0;
exit:

  return ret;
}

int p2p_ssl_free( p2p_ssl_ctx *ctx ){

  int ret = 0;

  if( !ctx ){
    ret = P2P_ERR_NULL_PARAM;
    goto exit;
  }

  ctr_drbg_free( &ctx->ctr_drbg );
  entropy_free( &ctx->entropy );
  ssl_free( &ctx->ssl );

  ret = 0;
exit:
  return ret;

}


int p2p_ssl_send( p2p_ssl_ctx *ctx, const unsigned char *buf, size_t size ){
  return ssl_write( &ctx->ssl, buf, size );
}

int p2p_ssl_recv( p2p_ssl_ctx *ctx, unsigned char *buf, size_t size){
  return ssl_read( &ctx->ssl, buf, size );
}
