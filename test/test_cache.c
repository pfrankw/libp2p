#include <unistd.h>

#include <p2p/log.h>
#include <p2p/cache.h>


#define DIGEST1 (unsigned char*)"\x79\x1d\x45\x1e\xee\xba\x27\x90\xd9\x4c\x83\x85\x6b\x96\xc4\xd4\x27\xb3\x1d\x1d"
#define DIGEST2 (unsigned char*)"\x79\x1d\x45\x1e\xee\xba\x27\x90\xd9\x4c\x83\x85\x6b\x96\xc4\xd4\x27\xb3\x1d\x1e"
#define DIGEST3 (unsigned char*)"\x79\x1d\x45\x1e\xee\xba\x27\x90\xd9\x4c\x83\x85\x6b\x96\xc4\xd4\x27\xb3\x1d\x1f"

int main(){

  int ret = 0, r;
  int ips[100];
  size_t ips_size = 100;
  FILE *fp;
  p2p_cache_ctx ctx;
  p2p_cache_header h;

  p2p_log_info( "sizeof(p2p_cache_header) = %d", sizeof(h) );
  p2p_log_info( "sizeof(p2p_cache_header.magic) = %d", sizeof(h.magic) );
  p2p_log_info( "sizeof(p2p_cache_header.n_section) = %d", sizeof(h.n_section) );


  if( access(P2P_CACHE_FILE, F_OK ) != -1 ){
    fp = fopen(P2P_CACHE_FILE, "r+");
    p2p_log_info( "Opening file");
  } else {
    fp = fopen(P2P_CACHE_FILE, "w+");
    p2p_log_info("Creating file");
  }

  if( !fp ){
    ret = 1;
    goto exit;
  }

  p2p_cache_init( &ctx, fp );

  //p2p_log_info( "p2p_cache_touch = %d", p2p_cache_touch( &ctx ) );
  p2p_log_info( "p2p_cache_check = %d", p2p_cache_check( &ctx ) );
  p2p_log_info( "p2p_cache_count = %u", p2p_cache_count_sections( &ctx ) );

  p2p_log_info( "p2p_cache_touch = %d", p2p_cache_touch( &ctx ) );
  p2p_log_info(" -------------------------- ");

  ips_size = 100;
  r = p2p_cache_search( &ctx, DIGEST1, 0, ips, &ips_size );
  p2p_log_info( "p2p_cache_search DIGEST1 = %d, ips_size = %u", r, ips_size );
  p2p_log_info( "p2p_cache_insert DIGEST1 = %d", p2p_cache_insert( &ctx, DIGEST1, 0x77000001, time( 0 ) ) );
  p2p_log_info( "p2p_cache_count = %u", p2p_cache_count_sections( &ctx ) );
  ips_size = 100;
  r = p2p_cache_search( &ctx, DIGEST1, 0, ips, &ips_size );
  p2p_log_info( "p2p_cache_search DIGEST1 = %d, ips_size = %u", r, ips_size );
  p2p_log_info(" -------------------------- ");

  ips_size = 100;
  r = p2p_cache_search( &ctx, DIGEST2, 0, ips, &ips_size );
  p2p_log_info( "p2p_cache_search DIGEST2 = %d, ips_size = %u", r, ips_size );
  p2p_log_info( "p2p_cache_insert DIGEST2 = %d", p2p_cache_insert( &ctx, DIGEST2, 0x77000001, time( 0 ) ) );
  p2p_log_info( "p2p_cache_count = %u", p2p_cache_count_sections( &ctx ) );
  ips_size = 100;
  r = p2p_cache_search( &ctx, DIGEST2, 0, ips, &ips_size );
  p2p_log_info( "p2p_cache_search DIGEST2 = %d, ips_size = %u", r, ips_size );
  p2p_log_info(" -------------------------- ");

  ips_size = 100;
  r = p2p_cache_search( &ctx, DIGEST3, 0, ips, &ips_size );
  p2p_log_info( "p2p_cache_search DIGEST3 = %d, ips_size = %u", r, ips_size );
  p2p_log_info( "p2p_cache_insert DIGEST3 = %d", p2p_cache_insert( &ctx, DIGEST3, 0x77000001, time( 0 ) ) );
  p2p_log_info( "p2p_cache_count = %u", p2p_cache_count_sections( &ctx ) );
  ips_size = 100;
  r = p2p_cache_search( &ctx, DIGEST3, 0, ips, &ips_size );
  p2p_log_info( "p2p_cache_search DIGEST3 = %d, ips_size = %u", r, ips_size );
  p2p_log_info(" -------------------------- ");

  p2p_log_info( "p2p_cache_remove DIGEST1 = %d", p2p_cache_remove( &ctx, DIGEST1, 0 ) );
  p2p_log_info( "p2p_cache_remove DIGEST2 = %d", p2p_cache_remove( &ctx, DIGEST2, 0 ) );
  p2p_log_info( "p2p_cache_remove DIGEST3 = %d", p2p_cache_remove( &ctx, DIGEST3, 0 ) );


exit:
  p2p_cache_free( &ctx );
  return ret;
}
