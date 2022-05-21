#include <p2p/rsa.h>
#include <p2p/err.h>

int p2p_rsa_key_gen(pk_context *key, int bits){


	entropy_context entropy;
	ctr_drbg_context ctr_drbg;
	int ret = 0;

	pk_init(key);
	entropy_init( &entropy );

	if( ctr_drbg_init( &ctr_drbg, entropy_func, &entropy, 0, 0 )  != 0 ){
		ret = P2P_ERR_NORANDOM;
		goto exit;
	}

	if( pk_init_ctx( key, pk_info_from_type( POLARSSL_PK_RSA ) ) != 0 ){
		ret = P2P_ERR_RSA_INIT;
		goto exit;
	}

	if( rsa_gen_key( pk_rsa( *key ), ctr_drbg_random, &ctr_drbg, bits, 65537 ) != 0){
		ret = P2P_ERR_RSA_GEN;
		goto exit;
	}

exit:

	ctr_drbg_free( &ctr_drbg );
	entropy_free( &entropy );
	return ret;
}


int p2p_rsa_key_exp(pk_context *key, unsigned char *buf, size_t size){

	int ret = 0;
	if( !key || !buf || !size ){
		ret = P2P_ERR_NULL_PARAM;
		goto exit;
	}

	if( pk_write_key_pem(key, buf, size) != 0 ){
		ret = P2P_ERR_RSA_EXP;
		goto exit;
	}

exit:
	return ret;

}


int p2p_rsa_key_write(pk_context *key, char *output_file){

	int ret = 0;
	FILE *f = 0;
	unsigned char buf[20000];
	size_t size = sizeof(buf);


	if( !key || !output_file ){
		ret = P2P_ERR_NULL_PARAM;
		goto exit;
	}

	if( ( size = p2p_rsa_key_exp(key, buf, size) ) != 0 ){
		ret = P2P_ERR_RSA_EXP;
		goto exit;
	}

	f = fopen(output_file, "wb");

	if(!f){
		ret = P2P_ERR_DENIED;
		goto exit;
	}

	fwrite(buf, 1, strlen((char*)buf), f);

exit:
	if(f)fclose(f);
	return ret;

}


int p2p_rsa_key_digest(pk_context *key, unsigned char *digest){

	int ret = 0;
	unsigned char buf[4096];

	if( !key || !digest ){
		ret = P2P_ERR_NULL_PARAM;
		goto exit;
	}

	if( ( ret = pk_write_pubkey_der(key, buf, sizeof(buf)) ) < 0 )
		goto exit;

	sha1( buf + ( sizeof(buf) - ret ), ret, digest );


	ret = 0;
exit:
	return ret;
}
