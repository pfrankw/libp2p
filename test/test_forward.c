#include <polarssl/net.h>
#include <p2p/p2p.h>
#include <p2p/utils.h>
/*

	Usage: test_foward mode server [digest]

	mode:	Can be s[erver] or c[lient]
	server: The server to connect to
	digest: Only in client mode

	Example: test/test_forward.out client 2 23b0b81d6fc315258f451f864c147060e61bad46
*/



int main(int argc, char **argv){

	int fd, i;
	x509_crt crt;
	pk_context pk;

	unsigned char digest[20], connect_digest[21];
	char ext_digest[41], *fwd_str = "hJ7K5nhAZEwkBoBzQRlZhYG6Pwmrimc7Sg5mMDi6U9uRtWFA7PKkiiGlaBGKXX1";

	if(argc < 3){
		p2p_log_error("Too less arguments");
		return 1;
	}

	if( argv[1][0] != 'c' && argv[1][0] != 's' ){
		p2p_log_error("Invalid mode");
		return 2;
	}

	if( argv[1][0] == 'c' && argc < 4){
		p2p_log_error("Connect digest not present in client mode");
		return 3;
	}

	if( argc > 3 && strlen(argv[3]) != 40 ){
		p2p_log_error("Invalid digest length");
		return 4;
	}


	p2p_log_info("Generating private key...");

	p2p_rsa_key_gen(&pk, 2048);
	p2p_x509_cert_gen(&crt, &pk);
	p2p_rsa_key_digest(&crt.pk, digest);

	memset( ext_digest, 0, sizeof(ext_digest) );
  for(i=0; i<20; i++)
    sprintf(ext_digest, "%s%.2x", ext_digest, digest[i]);

	p2p_log_info("Key digest: %s", ext_digest);

	if( net_connect(&fd, argv[2], 8090) != 0 ){
		p2p_log_error( "Can't connect to the server ");
		return -100;
	}

	if( argv[1][0] == 'c' ){
		p2p_utils_string2digest( connect_digest, argv[3] );
		if( p2p_net_authenticate( net_recv, net_send, &fd, (p2p_desc_peer_min*)digest ) != 0 ){
			p2p_log_error( "Can't authenticate" );
			return -1;
		}
		p2p_log_info( "Authenticated!" );
		if( p2p_net_forward( net_recv, net_send, &fd, (p2p_desc_peer_min*)connect_digest ) != 0 ){
			p2p_log_error( "Can't forward" );
			return -2;
		}
		p2p_log_info( "Forwarded!" );

		net_send( &fd, (unsigned char*)fwd_str, strlen(fwd_str) + 1 );


	} else {

		char buf[sizeof(p2p_net_packet_header)+P2P_MAX_PACKET_SIZE];
		p2p_net_packet_header *ph;

		if( p2p_net_authenticate( net_recv, net_send, &fd, (p2p_desc_peer_min*) digest ) != 0 ){
			p2p_log_error( "Can't authenticate" );
			return -1;
		}
		p2p_log_info( "Authenticated!" );
		p2p_log_info( "Waiting for connections" );

		if( p2p_net_recv_packet( net_recv, &fd, (unsigned char*)buf, sizeof(buf) ) <= 0 ){
			p2p_log_error( "Error during receiving the first packet" );
			return -2;
		}
		ph = (p2p_net_packet_header*) buf;

		if( ph->type == P2P_NET_TYPE_FORWARD ){

			p2p_net_send_packet( net_send, &fd, 0, 0, P2P_NET_TYPE_ACK );
			p2p_log_info( "Forwarding request accepted, waiting for the verify packet" );

			if( net_recv( &fd, (unsigned char*)buf, sizeof(buf) ) <= 0 ){
				p2p_log_error( "Error during receiving the verify packet" );
				return -3;
			}

			if( strncmp( buf, fwd_str, strlen(fwd_str) ) == 0 ){
				p2p_log_info( "fwd_str was correctly received" );
				return 0;
			} else {
				p2p_log_info( "fwd_str was not received correctly" );
				return -3;
			}

		}
	}


	return 0;
}
