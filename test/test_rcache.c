
#include <polarssl/net.h>

#include <p2p/log.h>
#include <p2p/cache.h>
#include <p2p/p2p.h>

/*

  This little "script" is used to connect to a remote server and
  fetch its cache.

  Usage: test_rcache server_ip


*/



int cb_cache_section( p2p_cache_section *section, void *param ){

  if( section->timestamp ){
    int i;
    char *ipp = (char*)&section->ip;
    char ext_digest[41];

    memset( ext_digest, 0, sizeof(ext_digest) );
    for(i=0; i<20; i++)
      sprintf(ext_digest, "%s%.2x", ext_digest, section->desc.pk_digest[i]);

    p2p_log_info( "%s %u.%u.%u.%u %u", ext_digest, ipp[0], ipp[1], ipp[2], ipp[3], section->timestamp );
  }

  return 0;
}

int main(int argc, char **argv){

  int fd, i;
  x509_crt crt;
	pk_context pk;
  unsigned char digest[20];
  char ext_digest[41];



  if( argc != 2 ){
    p2p_log_error("Bad CLI arguments");
    return -1;
  }


  if(	net_connect(&fd, argv[1], 8090) != 0 ){
    p2p_log_error("Can't connect to remote server");
    return -2;
  }

  p2p_log_info("Generating private key...");

  p2p_rsa_key_gen(&pk, 2048);
	p2p_x509_cert_gen(&crt, &pk);
	p2p_rsa_key_digest(&crt.pk, digest);

  memset( ext_digest, 0, sizeof(ext_digest) );
  for(i=0; i<20; i++)
    sprintf(ext_digest, "%s%.2x", ext_digest, digest[i]);

  p2p_log_info("Key digest: %s", ext_digest);

  if( p2p_net_authenticate( net_recv, net_send, &fd, (p2p_desc_peer_min*) digest ) != 0 ){
    p2p_log_error( "Can't authenticate" );
    return -1;
  }


  //p2p_peer_cache( &ctx, cb_cache_section, 0 );

  return 0;
}
