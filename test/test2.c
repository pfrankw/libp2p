#include <p2p/p2p.h>
#include <unistd.h>

int main() {

  int i, fd;
  x509_crt crt;
  pk_context pk;
  unsigned char digest[20];
  char ext_digest[41];


  if( pk_parse_keyfile( &pk, "priv.key", 0 ) != 0 ){
    p2p_log_info("Private key not found. Generating one ...");
    p2p_rsa_key_gen(&pk, 2048);
    p2p_rsa_key_write(&pk, "priv.key");
  }
  else
    p2p_log_info("Private key found. Loading it ... ");


  p2p_x509_cert_gen(&crt, &pk);
  p2p_rsa_key_digest(&crt.pk, digest);

  ext_digest[0] = 0;
  for(i=0; i<20; i++)
    sprintf(ext_digest,  "%s%.2x", ext_digest, digest[i]);

  p2p_log_info("Private key digest = %s", ext_digest);

  net_connect(&fd, "127.0.0.1", 8090);

  /* INCOMPLETE */


  sleep(100);
  return 0;
}
