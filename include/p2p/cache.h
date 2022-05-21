#ifndef P2P_CACHE_H
#define P2P_CACHE_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <p2p/config.h>
#include <p2p/err.h>
#include <p2p/log.h>
#include <p2p/desc.h>

#define P2P_CACHE_MAGIC_SIZE 4
#define P2P_CACHE_MAGIC "P2PC"


typedef struct _p2p_cache_search_result { /* UNUSED */
  int ip;
  long offset;
} p2p_cache_search_result;

typedef struct _p2p_cache_section {
  p2p_desc_peer_min desc;
  int ip;
  time_t timestamp;

} p2p_cache_section;

typedef struct _p2p_cache_header {

  char magic[P2P_CACHE_MAGIC_SIZE];
  unsigned int n_section;

} p2p_cache_header;


typedef struct _p2p_cache_ctx {

  FILE *f;
  int lock;

} p2p_cache_ctx;


int p2p_cache_init( p2p_cache_ctx *ctx, FILE *cache_file );
int p2p_cache_free( p2p_cache_ctx *ctx );
int p2p_cache_check( p2p_cache_ctx *ctx );
int p2p_cache_touch( p2p_cache_ctx *ctx );

int p2p_cache_get_section( p2p_cache_ctx *ctx, int n, p2p_cache_section *s );

unsigned long p2p_cache_count_sections( p2p_cache_ctx *ctx );
int p2p_cache_search( p2p_cache_ctx *ctx, unsigned char *digest, long *offsets, int *ips, size_t *ips_size );
int p2p_cache_get_ips( p2p_cache_ctx *ctx, int *ips, size_t *ips_size );
int p2p_cache_get_random_ips( p2p_cache_ctx *ctx, int *ips, size_t *ips_size );
int p2p_cache_get_random_ip( p2p_cache_ctx *ctx );


int p2p_cache_insert( p2p_cache_ctx *ctx, unsigned char *digest, int ip, time_t timestamp );
int p2p_cache_remove( p2p_cache_ctx *ctx, unsigned char *digest, int ip );






#endif
