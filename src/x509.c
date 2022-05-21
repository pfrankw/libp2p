#include <p2p/x509.h>

int p2p_x509_cert_gen(x509_crt *crt, pk_context *key){


	int ret = 0;
	mpi serial;
	entropy_context entropy;
	ctr_drbg_context ctr_drbg;
	x509write_cert write_cert;
	unsigned char buf[8192];
	char *subject_name = "CN=Cert,O=libp2p TLS,C=UK";

	if(!crt || !key){
		ret = P2P_ERR_NULL_PARAM;
		goto exit;
	}

	x509_crt_init( crt );
	x509write_crt_init( &write_cert );
	x509write_crt_set_md_alg( &write_cert, POLARSSL_MD_SHA256 );

	mpi_init( &serial );
	entropy_init( &entropy );

	if( ( ret = ctr_drbg_init( &ctr_drbg, entropy_func, &entropy, 0, 0 ) ) != 0 )
		goto exit;


	if( ( ret = mpi_read_string( &serial, 10, "1" ) ) != 0 )
		goto exit;


	x509write_crt_set_subject_key( &write_cert, key );
	x509write_crt_set_issuer_key( &write_cert, key );

	if( ( ret = x509write_crt_set_subject_name( &write_cert, subject_name ) ) != 0 )
		goto exit;

	if( ( ret = x509write_crt_set_issuer_name( &write_cert, subject_name ) ) != 0 )
		goto exit;


	if( ( ret = x509write_crt_set_serial( &write_cert, &serial ) ) != 0 )
		goto exit;

	if( ( ret = x509write_crt_set_validity( &write_cert, "19700101000000", "20160101000000") ) != 0)
		goto exit;

	if( ( ret = x509write_crt_set_basic_constraints( &write_cert, 0, -1 ) ) != 0)
		goto exit;

	if( ( ret = x509write_crt_set_subject_key_identifier( &write_cert ) ) != 0)
		goto exit;

	if( ( ret = x509write_crt_set_authority_key_identifier( &write_cert ) ) != 0)
		goto exit;

	if( ( ret = x509write_crt_der( &write_cert, buf, sizeof(buf),0, 0 ) ) < 0 )
		goto exit;

	if( ( ret = x509_crt_parse(crt, buf + ( sizeof(buf) - ret ), ret) ) != 0 )
		goto exit;




	ret = 0;
exit:
	ctr_drbg_free( &ctr_drbg );
	entropy_free( &entropy );
	mpi_free( &serial );
	x509write_crt_free( &write_cert );
	return ret;
}


int p2p_x509_cert_digest(x509_crt *crt, unsigned char digest[20]){

	if( !crt || !digest )
		return P2P_ERR_NULL_PARAM;

	sha1(crt->tbs.p, crt->tbs.len, digest);

	return 0;
}
