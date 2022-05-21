#include <p2p/cache.h>



int p2p_cache_init( p2p_cache_ctx *ctx, FILE *cache_file ){

  if ( !ctx || !cache_file ) {
    return P2P_ERR_NULL_PARAM;
  }

  ctx->f = cache_file;
  ctx->lock = 0;

  if( p2p_cache_check( ctx ) != 0 )
    p2p_cache_touch( ctx );

  return 0;
}

int p2p_cache_free( p2p_cache_ctx *ctx ){

  if( !ctx )
    return P2P_ERR_NULL_PARAM;

  if( ctx->f )
    fclose( ctx->f );

  return 0;
}

int p2p_cache_valid_magic( char* magic ){
  return memcmp( magic, P2P_CACHE_MAGIC, P2P_CACHE_MAGIC_SIZE ) == 0;
}

int p2p_cache_check( p2p_cache_ctx *ctx ){

  int ret = 0;
  unsigned char buf[sizeof(p2p_cache_header)];
  p2p_cache_header *h;


  if( !ctx ){
    return P2P_ERR_NULL_PARAM;
  }

  while( ctx->lock );
  ctx->lock = 1;

  rewind( ctx->f );
  fread( buf, sizeof(p2p_cache_header), 1, ctx->f );
  h = (p2p_cache_header*) buf;

  if( ! p2p_cache_valid_magic( h->magic ) ){
    ret = 1;
    goto exit;
  }


  if( h->n_section > P2P_CACHE_MAX_SECTIONS ){
    ret = 1;
    goto exit;
  }


  ret = 0;
exit:
  rewind( ctx->f );
  ctx->lock = 0;
  return ret;


}


int p2p_cache_touch( p2p_cache_ctx *ctx ){

  int ret = 0;
  p2p_cache_header h;

  if( !ctx ){
    return P2P_ERR_NULL_PARAM;
  }

  while( ctx->lock );
  ctx->lock = 1;

  memcpy( h.magic, P2P_CACHE_MAGIC, P2P_CACHE_MAGIC_SIZE );
  h.n_section = 0;

  ctx->f = freopen( 0, "w+", ctx->f );

  if( ctx->f == 0 ){
    ret = P2P_ERR_CACHE_REOPEN;
    goto exit;
  }

  fwrite( &h, sizeof(p2p_cache_header), 1, ctx->f );




  ret = 0;
exit:
  ctx->lock = 0;
  return ret;

}


int p2p_cache_get_section( p2p_cache_ctx *ctx, int n, p2p_cache_section *s ){

  int ret = 0;
  if( !ctx || !s )
    return P2P_ERR_NULL_PARAM;

  while( ctx->lock );
  ctx->lock = 1;

  if( fseek( ctx->f, sizeof(p2p_cache_header) + ( n * sizeof(p2p_cache_section) ), SEEK_SET ) < 0 ){
    ret = P2P_ERR_DENIED;
    goto exit;
  }

  if( fread( s, sizeof(p2p_cache_section), 1, ctx->f ) != 1 ){
    ret = P2P_ERR_DENIED;
    goto exit;
  }

exit:
  rewind( ctx->f );
  ctx->lock = 0;
  return ret;
}


int p2p_cache_search( p2p_cache_ctx *ctx, unsigned char *digest, long *offsets, int *ip, size_t *ipsize ){ /* CHANGE: ipsize => size */

  int ret = 0;
  p2p_cache_section s; /* Cache section */
  size_t ipi; /* IP array counter */
  time_t ts_now;

  if( !ctx || !digest || !ipsize || ( !ip && *ipsize > 0 ) ){
    return P2P_ERR_NULL_PARAM;
  }

  ts_now = time( 0 );

  if( p2p_cache_check( ctx ) != 0 ){
    ret = P2P_ERR_CACHE_BAD_FORMAT;
    goto exit;
  }

  while( ctx->lock );
  ctx->lock = 1;

  ipi = 0;

  fseek( ctx->f, sizeof(p2p_cache_header), SEEK_SET );

  while( fread( &s, sizeof(s), 1, ctx->f ) == 1 && ( *ipsize == 0 || ipi < *ipsize ) ){ /* fread is successful and ( *ipsize = 0 or ipi is less than *ipsize )*/

    if( ts_now - s.timestamp > P2P_CACHE_REMOVE_TIME ){ /* "Automatically" removing old entries */
      size_t cur_seek;
      ctx->lock = 0;
      cur_seek = ftell( ctx->f );
      p2p_cache_remove( ctx, digest, s.ip );
      fseek( ctx->f, cur_seek, SEEK_SET );
      while(ctx->lock); ctx->lock = 1;
      continue;
    }

    if( memcmp( s.desc.pk_digest, digest, P2P_DIGEST_SIZE ) == 0 ){
      if( *ipsize > 0 ){
         ip[ipi] = s.ip; /* Saving ips into IP array only if IP array size > 0 */
         if(offsets) offsets[ipi] = ftell( ctx->f ) - sizeof(s); /* If offsets pointer isn't == 0 then it will be filled */
      }
      ipi++;
    }

  }

  *ipsize = ipi;

exit:
  rewind( ctx->f );
  ctx->lock = 0;
  return ret;
}

int p2p_cache_seek_null_section( p2p_cache_ctx *ctx ){

  char buf[sizeof(p2p_cache_section)];
  char bufcmp[sizeof(buf)];
  int ret = 1;

  if( !ctx )
    return P2P_ERR_NULL_PARAM;

  while( ctx->lock );
  ctx->lock = 1;

  memset( bufcmp, 0, sizeof(bufcmp) );
  fseek( ctx->f, sizeof(p2p_cache_header), SEEK_SET );

  while( fread( buf, sizeof(buf), 1, ctx->f ) == 1 ){
    if( memcmp( buf, bufcmp, sizeof(buf) ) == 0 ){
      ret = 0;
      fseek( ctx->f, /*ftell( ctx->f )*/- sizeof(buf), SEEK_CUR );
      break;
    }
  }

  ctx->lock = 0;
  return ret;
}

int p2p_cache_insert( p2p_cache_ctx *ctx, unsigned char *digest, int ip, time_t timestamp ){

  int ret = 0;
  int *ips = 0;
  long *offsets = 0;

  size_t ips_size;
  p2p_cache_section s;

  if( !ctx || !digest || !timestamp ){
    return P2P_ERR_NULL_PARAM;
  }

  ips_size = 0;

  if( ( ret = p2p_cache_search( ctx, digest, 0, 0, &ips_size ) ) != 0 ){
    goto exit;
  }

  while( ctx->lock );
  ctx->lock = 1;

  memcpy( s.desc.pk_digest, digest, P2P_DIGEST_SIZE );
  s.ip = ip;
  s.timestamp = timestamp;


  if( ips_size == 0 ){ /* If there are no entries with the supplied digest */

    ctx->lock = 0;
    p2p_cache_seek_null_section( ctx );
    while(ctx->lock); ctx->lock = 1;

    int r;
    if( ( r = fwrite( &s, sizeof(s), 1, ctx->f ) ) != 1 ){
      ret = P2P_ERR_DENIED;
      goto exit;
    }

  } else { /* It there are entries we find the one with the same ip address and update its timestamp */

    int found = 0;
    size_t i;
    ips = (int*) malloc( sizeof(int) * ips_size );
    offsets = (long*) malloc( sizeof(long) * ips_size );

    ctx->lock = 0;
    if( ( ret = p2p_cache_search( ctx, digest, offsets, ips, &ips_size ) ) != 0 ){
      goto exit;
    }
    while(ctx->lock); ctx->lock = 1;

    for(i=0; i<ips_size; i++){
      if( ips[i] == ip ){
        found = 1;
        break;
      }
    }

    if(found){
      fseek( ctx->f, offsets[i], SEEK_SET );
    } else {
      fseek( ctx->f, 0, SEEK_END );
    }

    fwrite( &s, sizeof(s), 1, ctx->f );

  }

exit:
  free(ips);
  free(offsets);
  ctx->lock = 0;
  return ret;

}

int p2p_cache_remove( p2p_cache_ctx *ctx, unsigned char *digest, int ip ){

  int ret = 0;
  p2p_cache_section s;

  if( !ctx || !digest ){
    return P2P_ERR_NULL_PARAM;
  }

  while( ctx->lock );
  ctx->lock = 1;

  fseek( ctx->f, sizeof(p2p_cache_header), SEEK_SET );

  while( fread( &s, sizeof(s), 1, ctx->f ) == 1 ){

    if( memcmp( s.desc.pk_digest, digest, P2P_DIGEST_SIZE ) == 0 && ( ip == 0 || s.ip == ip ) ){
      fseek( ctx->f, - sizeof(s), SEEK_CUR );
      memset( &s, 0, sizeof(s) );
      if( fwrite( &s, sizeof(s), 1, ctx->f ) != 1 ){
        ret = P2P_ERR_DENIED;
        goto exit;
      }

      if( ip != 0 )
        goto exit;

    }

  }

exit:
  rewind( ctx->f );
  ctx->lock = 0;
  return ret;
}


unsigned long p2p_cache_count_sections( p2p_cache_ctx *ctx ){

  unsigned long i = 0;
  if( !ctx )
    return 0;

  while( ctx->lock );
  ctx->lock = 1;

  fseek( ctx->f, 0, SEEK_END );
  if( ftell( ctx->f ) < sizeof(p2p_cache_header) )
    i = 0;
  else
    i = ( ftell( ctx->f ) - sizeof(p2p_cache_header) ) / sizeof(p2p_cache_section);

  rewind( ctx->f );
  ctx->lock = 0;
  return i;

}

int p2p_cache_get_ips( p2p_cache_ctx *ctx, int *ips, size_t *ips_size ){

  p2p_cache_section s;
  size_t ipi;
  if( !ctx || !ips_size || ( !ips && *ips_size > 0 ) )
    return P2P_ERR_NULL_PARAM;

  while( ctx->lock );
  ctx->lock = 1;

  ipi = 0;
  fseek( ctx->f, sizeof(p2p_cache_header), SEEK_SET );

  while( fread(&s, sizeof(s), 1, ctx->f ) == 1 && (*ips_size == 0 || ipi < *ips_size) ){
    if( s.timestamp != 0 ){
      if( *ips_size > 0 )
        ips[ipi] = s.ip;
      ipi++;
    }
  }

  *ips_size = ipi;


  ctx->lock = 0;
  return 0;
}

/* I see bugs here */
int p2p_cache_get_random_ips( p2p_cache_ctx *ctx, int *ips, size_t *ips_size ){

  int *cache_ips = 0, *cache_unique_ips = 0;
  size_t cache_ips_size = 0;
  size_t i, k, iu;


  if( !ctx || !ips || !ips_size )
    return P2P_ERR_NULL_PARAM;

  p2p_cache_get_ips( ctx, 0, &cache_ips_size );
  cache_ips = (int*) malloc( cache_ips_size * sizeof(int) );
  cache_unique_ips = (int*) malloc( cache_ips_size * sizeof(int) );
  p2p_cache_get_ips( ctx, cache_ips, &cache_ips_size );


  /* The ips are not uniques */
	iu = 0;
	for(i=0; i<cache_ips_size; i++){
		int found = 0;
		for(k=0; k<iu; k++){
				if( cache_ips[i] == cache_unique_ips[k] ){
					found = 1;
					break;
				}
		}
		if( !found ){
			cache_unique_ips[iu] = cache_ips[i];
			iu++;
		}
	}
	/* Now they are */


  if( *ips_size >= iu ){

    for(i=0; i<iu; i++){
      ips[i] = cache_unique_ips[i];
    }
    *ips_size = iu;
    goto exit;

  } else {

    srand( time(0) );
    size_t offset;

    offset = (size_t) rand() % (iu - *ips_size);

    for(i=0; i<*ips_size; i++){
      ips[i] = cache_unique_ips[offset+i];
    }

    goto exit;

  }

exit:
  free( cache_ips );
  free( cache_unique_ips );

  return 0;


}

int p2p_cache_get_random_ip( p2p_cache_ctx *ctx ){

  int ip;
  size_t size_ip;

  if( !ctx )
    return 0;

  size_ip = 1;

  if( p2p_cache_get_random_ips( ctx, &ip, &size_ip ) != 0 )
    return 0;

  return ip;

}
