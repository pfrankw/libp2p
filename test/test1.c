#include <stdio.h>

#include <p2p/rsa.h>
#include <p2p/x509.h>
#include <p2p/p2p.h>
#include <p2p/utils.h>

int cb_conn( p2p_desc_peer_min *desc, p2p_conn_ctx *conn, void *cb_param ){

	char ext_digest[50];

	p2p_utils_digest2string( ext_digest, desc->pk_digest );
	p2p_log_info( "New connection from %s", ext_digest );

	p2p_free_ext_conn( conn );
	return 0;
}

int main(int argc, char **argv){

	p2p_ctx ctx;
	x509_crt crt;
	pk_context pk;
	unsigned char digest[20];
	char ext_digest[41];

	p2p_rsa_key_gen(&pk, 2048);
	p2p_x509_cert_gen(&crt, &pk);
	p2p_rsa_key_digest(&pk, digest);
	p2p_utils_digest2string( ext_digest, digest );

	p2p_log_info( "Key digest: %s", ext_digest );

	p2p_init(&ctx, &crt, &pk, 0, cb_conn, 0);

	while(1) sleep(1);

	//printf("%s", buf);


	return 0;
}
