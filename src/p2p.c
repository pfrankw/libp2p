#include <stdio.h>
#include "p2p/p2p.h"


p2p_ctx *p2p_global_ctx = 0;


int p2p_init(p2p_ctx *ctx, x509_crt *crt, pk_context *pk, void *cb_verify_conn, void *cb_conn, void *cb_param ){

	if( p2p_global_ctx )
		return P2P_ERR_DENIED;


	if( !ctx || !crt || !pk || !cb_conn ){
		return P2P_ERR_NULL_PARAM;
	}

	ctx->crt = crt;
	ctx->pk = pk;
	ctx->cb_verify_conn = cb_verify_conn;
	ctx->cb_conn = cb_conn;
	ctx->cb_param = cb_param;


	if( signal( SIGINT, p2p_cb_sigint ) == SIG_ERR )
		p2p_log_info("Can't catch CTRL+C");

	if( p2p_init_cache( ctx ) != 0 ){
		return P2P_ERR_DENIED;
	}

	if( p2p_init_server( ctx ) != 0 ){
		return P2P_ERR_DENIED;
	}


	p2p_global_ctx = ctx;

	p2p_init_all_conn( ctx );
	pthread_create( &ctx->manager_tid, 0, (void*)p2p_conn_manager, ctx );
	pthread_detach( ctx->manager_tid );


	return 0;
}



int p2p_free(p2p_ctx *ctx){

	p2p_server_stop( &ctx->server );
	p2p_cache_free( &ctx->cache );
	p2p_free_all_conn( ctx );

	return 0;
}

void p2p_cb_sigint( int sig ){
	if( !p2p_global_ctx ) return;

	p2p_free( p2p_global_ctx );

	p2p_log_info("libp2p is closing");
	exit(0);
}


int p2p_init_cache(p2p_ctx *ctx){

	FILE *fp;

	if( !ctx )
		return P2P_ERR_NULL_PARAM;

	if( access(P2P_CACHE_FILE, F_OK ) != -1 ){
    fp = fopen(P2P_CACHE_FILE, "r+");
  } else {
    fp = fopen(P2P_CACHE_FILE, "w+");
  }

	if( p2p_cache_init( &ctx->cache, fp ) != 0 ){
		p2p_log_error("Can't initialize the cache module");
		return P2P_ERR_DENIED;
	}

	return 0;

}


int p2p_init_server(p2p_ctx *ctx){

	p2p_server_cbs cbs;

	if( !ctx )
		return P2P_ERR_NULL_PARAM;

	cbs.new_client = p2p_cb_server_new_client;
	cbs.packet_recv = p2p_cb_server_packet_recv;
	cbs.peer_auth = p2p_cb_server_peer_auth;
	cbs.peer_forward = p2p_cb_server_peer_forward;
	cbs.peer_disconnect = p2p_cb_server_peer_disconnect;
	cbs.invalid_type = p2p_cb_server_invalid_type;
	cbs.recv_error = p2p_cb_server_recv_error;

	if( p2p_server_init( &ctx->server, 8090 ) != 0 ){
		p2p_log_error("Can't initialize the server module");
		return P2P_ERR_DENIED;
	}

	if( p2p_server_set_cbs( &ctx->server, &cbs, &ctx ) != 0 ){
		p2p_log_error("Can't set the server's callbacks");
		return P2P_ERR_DENIED;
	}


	if( p2p_server_set_cache( &ctx->server, &ctx->cache ) != 0 ){
		p2p_log_error("Can't set the server's cache module");
		return P2P_ERR_DENIED;
	}

	p2p_server_listen( &ctx->server );

	return 0;

}


int p2p_init_conn( p2p_ctx *ctx, int i, int ip ){

	int fd;
	char str_ip[25];
	p2p_desc_peer_min desc;

	if( !ctx )
		return P2P_ERR_NULL_PARAM;

	if( ctx->c[i] )
		return P2P_ERR_NOT_EMPTY;

	p2p_rsa_key_digest( ctx->pk, desc.pk_digest );

	if( !ip )
		ip = p2p_cache_get_random_ip( &ctx->cache );

	inet_ntop( AF_INET, &ip, str_ip, sizeof(str_ip) );
	if( net_connect( &fd, str_ip, 8090 ) != 0 )
		return P2P_ERR_RETRY;

	net_set_nonblock( fd );

	if( p2p_net_authenticate( net_recv, net_send, &fd, &desc ) != 0 ){
		net_close( fd );
		return P2P_ERR_DENIED;
	}

	ctx->c[i] = (p2p_conn_ctx*) malloc( sizeof(p2p_conn_ctx) );
	ctx->c[i]->fd = fd;
	ctx->c[i]->crt = ctx->crt;
	ctx->c[i]->pk = ctx->pk;

	return P2P_ERR_OK;
}

int p2p_free_conn( p2p_ctx *ctx, int i ){

	if( !ctx )
		return P2P_ERR_NULL_PARAM;

	if( !ctx->c[i] )
		return P2P_ERR_INVALID_PARAM;


	net_close( ctx->c[i]->fd );
	free( ctx->c[i] );
	ctx->c[i] = 0;

	return P2P_ERR_OK;
}

int p2p_free_ext_conn( p2p_conn_ctx *conn ){

	if( !conn )
		return P2P_ERR_NULL_PARAM;

	net_close( conn->fd );
	free( conn );
	return P2P_ERR_OK;
}

int p2p_detach_conn(p2p_ctx *ctx, int i, p2p_conn_ctx **conn){

	if( !ctx || !conn )
		return P2P_ERR_NULL_PARAM;

	*conn = ctx->c[i];
	ctx->c[i] = 0;
	while( p2p_init_conn( ctx, i, 0 ) != 0 );

	return P2P_ERR_OK;
}

int p2p_init_all_conn(p2p_ctx *ctx){

	int i;

	if( !ctx )
		return P2P_ERR_NULL_PARAM;

	for(i=0; i<P2P_ADVERTISE_CONNECTIONS; i++){
		while( p2p_init_conn( ctx, i, 0 ) != 0 );
	}

	return P2P_ERR_OK;
}

int p2p_free_all_conn(p2p_ctx *ctx){

	int i;

	if( !ctx )
		return P2P_ERR_NULL_PARAM;

	for(i=0; i<P2P_ADVERTISE_CONNECTIONS; i++){
		p2p_free_conn( ctx, i );
	}

	return P2P_ERR_OK;
}


int p2p_conn_manager(p2p_ctx *ctx){

	unsigned char buf[sizeof(p2p_net_packet_header) + P2P_MAX_PACKET_SIZE];

	if( !ctx )
		return P2P_ERR_NULL_PARAM;

	ctx->cm_run = 1;

	while( ctx->cm_run ){

		int i;
		for(i=0; i<P2P_ADVERTISE_CONNECTIONS; i++){
			int r;
			r = p2p_net_recv_packet( net_recv, &ctx->c[i]->fd, buf, sizeof(buf) );

			if( r == P2P_ERR_NET_NODATA );  /* No data */
			else if( r < 0 ){	/* Bye bye */
				p2p_free_conn( ctx, i );
				while( p2p_init_conn( ctx, i, 0 ) != 0 );
				continue;
			} else {

				p2p_net_packet_header *ph = (p2p_net_packet_header*) buf;

				switch( ph->type ){


					case P2P_NET_TYPE_FORWARD:
						{
							int accepted = 0;
							p2p_net_forward_packet *fp;
							fp = (p2p_net_forward_packet*) (buf + sizeof(p2p_net_packet_header));

							if( ctx->cb_verify_conn ){
								int ret;
								ret = ctx->cb_verify_conn( &fp->desc, ctx->cb_param );
								accepted = ret ? 0 : 1;
							} else {
								accepted = 1;
							}

							if( accepted ){
								p2p_conn_ctx *conn;
								if( p2p_net_send_packet( net_send, &ctx->c[i]->fd, 0, 0, P2P_NET_TYPE_ACK ) <= 0 )
									continue;
								if( p2p_detach_conn( ctx, i, &conn ) != 0 )
									continue;

								ctx->cb_conn( &fp->desc, conn, ctx->cb_param );

							} else {
								p2p_net_send_packet( net_send, &ctx->c[i]->fd, 0, 0, P2P_NET_TYPE_DENY );
							}


						}
					break;

					default:
						p2p_free_conn( ctx, i );
						while( p2p_init_conn( ctx, i, 0 ) != 0 );
						continue;
					break;
				}


			}



		}

	}

	return P2P_ERR_OK;
}


int p2p_cb_server_new_client(int i, int ip, void *cb_param){
	char *ipp = (char*)&ip;
	p2p_log_info( "New client! #%d IP: %u.%u.%u.%u", i, ipp[0], ipp[1], ipp[2], ipp[3] );
	return 0;
}

int p2p_cb_server_packet_recv(int i, int size, int type, void *cb_param){
	p2p_log_info( "Packet from #%d Size = %d Type = %d", i, size, type );
	return 0;
}

int p2p_cb_server_peer_auth(int i, p2p_desc_peer_min *desc, void *cb_param){
	p2p_log_info( "Peer authenticated! #%d", i );
	return 0;
}

int p2p_cb_server_peer_forward(int i, int fwd_i, void *cb_param){
	p2p_log_info( "Peer forwarding #%d <=========> #%d", i, fwd_i );
	return 0;
}

int p2p_cb_server_peer_disconnect(int i, void *cb_param){
	p2p_log_info( "Peer disconnected! #%d", i );
	return 0;
}

int p2p_cb_server_invalid_type(int i, void *cb_param){
	return 0;
}

int p2p_cb_server_recv_error(int i, int e, void *cb_param){
	p2p_log_info( "Peer recv error! #%d p2p_net_recv_packet error = %d", i, e );
	return 0;
}
