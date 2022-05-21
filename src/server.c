#include <p2p/server/server.h>



int p2p_server_init(p2p_server_ctx *ctx, int port){

  int i, ret = 0;

  if(!ctx || !port){
    ret = P2P_ERR_NULL_PARAM;
    goto exit;
  }

  ctx->lfd = -1;
  ctx->port = port;
  ctx->run = 1;
  ctx->listener_tid = 0;

  for(i=0; i<P2P_SERVER_MAX_PEERS; i++)
    ctx->sp[i] = 0;

exit:
  return ret;
}

void p2p_server_stop(p2p_server_ctx *ctx){

  int i;
  if( !ctx )
    return;

  ctx->run = 0;
  sleep(1);
  for(i=0; i<P2P_SERVER_MAX_PEERS; i++){
    p2p_server_free_peer( ctx, i );
  }

}

int p2p_server_set_cache(p2p_server_ctx *ctx, p2p_cache_ctx *cache){

  if( !ctx )
    return P2P_ERR_NULL_PARAM;

  ctx->cache = cache;

  return 0;

}

int p2p_server_set_cbs(p2p_server_ctx *ctx, p2p_server_cbs *cbs, void *cbs_param){

  if( !ctx || !cbs )
    return P2P_ERR_NULL_PARAM;

  ctx->cbs_param = cbs_param;

  memcpy( &ctx->cbs, cbs, sizeof(ctx->cbs) );

  return 0;
}


int p2p_server_listen(p2p_server_ctx *ctx){

  int ret = 0,
      sp_fd = -1,   /* Server peer FD */
      sp_ip = 0;    /* Server peer IP */

  if( !ctx ){
    ret = P2P_ERR_NULL_PARAM;
    goto exit;
  }

  if( ctx->listener_tid == 0 ){ /* Restarts itself as thread */
    pthread_create( &ctx->listener_tid, 0, (void*)p2p_server_listen, ctx );
    pthread_detach( ctx->listener_tid );

    return 0;
  }

  pthread_create( &ctx->manager_tid, 0, (void*)p2p_server_manager, ctx );
  pthread_detach( ctx->manager_tid );

  if( ( ret = net_bind( &ctx->lfd, 0, ctx->port ) )  != 0 )
    goto exit;


  while( ctx->run ){

    if( net_accept( ctx->lfd, &sp_fd, &sp_ip ) == 0 ){



      net_set_nonblock(sp_fd);

      int sp_i = 0;

      if( ( sp_i = p2p_server_alloc_peer(ctx, sp_fd) ) < 0 ){
        net_close( sp_fd );
        continue;
      }


      if( ctx->cbs.new_client ) ctx->cbs.new_client( sp_i, sp_ip, ctx->cbs_param );

    }
  }



exit:
  net_close( ctx->lfd );
  return ret;
}

int p2p_server_alloc_peer(p2p_server_ctx *ctx, int fd){

  int i, ret = 0;

  if( !ctx ){
    ret = P2P_ERR_NULL_PARAM;
    goto exit;
  }

  for(i=0; ctx->sp[i] && i<P2P_SERVER_MAX_PEERS; i++);

  if( i == P2P_SERVER_MAX_PEERS ){
    ret = P2P_ERR_FULL_PEERS;
    goto exit;
  }

  ctx->sp[i] = (p2p_server_peer_ctx*) calloc( 1, sizeof( p2p_server_peer_ctx ) );
  ctx->sp[i]->fd = fd;
  ctx->sp[i]->forward_i = -1;
  ret = i;

exit:
  return ret;

}

int p2p_server_free_peer(p2p_server_ctx *ctx, int sp_i){

  int ret = 0;

  if( !ctx ){
    ret = P2P_ERR_NULL_PARAM;
    goto exit;
  }

  if( sp_i < 0 || sp_i >= P2P_SERVER_MAX_PEERS ){
    ret = P2P_ERR_INVALID_PARAM;
    goto exit;
  }

  if( ctx->sp[sp_i] != 0 ){
    net_close( ctx->sp[sp_i]->fd );
    free( ctx->sp[sp_i] );
    ctx->sp[sp_i] = 0;
  }

exit:
  return ret;
}

int p2p_server_get_peers(p2p_server_ctx *ctx, p2p_server_peer_ctx *sp[]){

  int i, k;


  for(i=0, k=0; i<P2P_SERVER_MAX_PEERS; i++){
    if( ctx->sp[i] ){
       *(sp+k) = ctx->sp[i];
      k++;
    }
  }

  return k;

}

int p2p_server_get_peer_by_digest(p2p_server_ctx *ctx, unsigned char *pk_digest, int *index){
  int i;


  for(i=0; i<P2P_SERVER_MAX_PEERS; i++){
    if( ctx->sp[i] && memcmp(ctx->sp[i]->desc.pk_digest, pk_digest, P2P_DIGEST_SIZE) == 0 ){
      //*sp = ctx->sp[i];
      *index = i;
      return 0;
    }
  }

  return P2P_ERR_NOTFOUND;
}


int p2p_server_manager(p2p_server_ctx *ctx){

  int i, ret = 0;


  if( !ctx ){
    ret = P2P_ERR_NULL_PARAM;
    goto exit;
  }

  while ( ctx->run ){

    for( i = 0; i<P2P_SERVER_MAX_PEERS; i++ ){  /* sp[i] is ALWAYS the current server peer */
      p2p_server_peer_ctx *cur_sp; /* Current server peer */

      cur_sp = ctx->sp[i];

      if( cur_sp != 0 ){

        if( cur_sp->forward_i == -1 ){
          int r;
          unsigned char buf[sizeof(p2p_net_packet_header)+P2P_MAX_PACKET_SIZE];
          r = p2p_net_recv_packet( net_recv, &cur_sp->fd, buf, sizeof(buf) );



          if( r == P2P_ERR_NET_NODATA );  /* No data */
          else if( r == P2P_ERR_NET_DISCONNECTED ){ /* Disconnected */

            if( ctx->cbs.peer_disconnect ) ctx->cbs.peer_disconnect( i, ctx->cbs_param );
            p2p_server_free_peer(ctx, i);


          } else if( r < 0 ){ /* Some errors */

            if( ctx->cbs.recv_error ) ctx->cbs.recv_error( i, r, ctx->cbs_param );
            p2p_server_free_peer( ctx, i );
            continue;

          } else { /* Data */

            p2p_net_packet_header *ph = (p2p_net_packet_header*) buf;
            if( ctx->cbs.packet_recv ) ctx->cbs.packet_recv( i, ph->size, ph->type, ctx->cbs_param );


            switch( ph->type ){


              case P2P_NET_TYPE_CACHE:
                p2p_log_info("CACHE");

                if( ctx->cache ){
                  unsigned long is, n_sections;
                  p2p_cache_section s;

                  n_sections = p2p_cache_count_sections( ctx->cache );

                  p2p_net_send_packet( net_send, &cur_sp->fd, (unsigned char*)&n_sections, sizeof(n_sections), P2P_NET_TYPE_NUMBER );
                  for(is=0; is<n_sections; is++){

                    if( p2p_cache_get_section( ctx->cache, is, &s ) == 0 ){
                      p2p_net_send_packet( net_send, &cur_sp->fd, (unsigned char*)&s, sizeof(s), P2P_NET_TYPE_CACHE );
                    }

                  }

                } else {
                  p2p_net_send_packet( net_send, &cur_sp->fd, buf, 0, P2P_NET_TYPE_DENY ); /* Cache module not supported */
                }

              break;

              case P2P_NET_TYPE_AUTH: /* Need a better auth system..a bit more secure, with digital sign of course */

                if( ph->size == P2P_DIGEST_SIZE && memcpy( cur_sp->desc.pk_digest, buf + sizeof(p2p_net_packet_header), P2P_DIGEST_SIZE ) ){
                  if( ctx->cbs.peer_auth ) ctx->cbs.peer_auth( i, &cur_sp->desc, ctx->cbs_param );
                  if( ctx->cache ) p2p_cache_insert( ctx->cache, cur_sp->desc.pk_digest, 0x0100007F, time(0) );
                  p2p_net_send_packet( net_send, &cur_sp->fd, 0, 0, P2P_NET_TYPE_ACK );
                } else {
                  p2p_net_send_packet( net_send, &cur_sp->fd, 0, 0, P2P_NET_TYPE_DENY );
                }


              break;

              case P2P_NET_TYPE_FORWARD: /* A bit blocking, it waits for a packet from fwd_sp. Next ver will be totally async */
                ;

                int r_l, i_l;
                unsigned char buf_l[sizeof(p2p_net_packet_header)+P2P_MAX_PACKET_SIZE]; /* Buffer local */

                p2p_server_peer_ctx *fwd_sp; /* Server Peer local */
                p2p_net_forward_packet *pfp; /* Pointer to Forward Packet */

                pfp = (p2p_net_forward_packet*) ( buf + sizeof(p2p_net_packet_header) );


                if( p2p_server_get_peer_by_digest( ctx, pfp->desc.pk_digest, &i_l ) == 0 ){ /* Searching the "listening peer" the "client peer" asked for */
                  fwd_sp = ctx->sp[i_l];

                  memcpy( &pfp->desc, &cur_sp->desc, sizeof(p2p_desc_peer_min) );
                  p2p_net_send_packet( net_send, &fwd_sp->fd, (unsigned char*)pfp, sizeof(p2p_net_forward_packet), P2P_NET_TYPE_FORWARD ); /* Asking the listening peer to accept a forwarding request */


                  do {
                    r_l = p2p_net_recv_packet( net_recv, &fwd_sp->fd, buf_l, sizeof(buf_l) ); /* BUG DoS: If some client does not send data the program blocks here */
                  } while( r_l == P2P_ERR_NET_NODATA );

                  if(  r_l > 0 ){
                    p2p_net_packet_header *ph;
                    ph = (p2p_net_packet_header*) buf_l;

                    if( ph->type == P2P_NET_TYPE_ACK ){ /* The listening peer ACCEPTED the forwarding request */
                      if( ctx->cbs.peer_forward ) ctx->cbs.peer_forward( i, i_l, ctx->cbs_param );
                      p2p_net_send_packet( net_send, &cur_sp->fd, buf_l, 0, P2P_NET_TYPE_ACK );
                      cur_sp->forward_i = i_l;
                      fwd_sp->forward_i = i;
                      /* Successfully forwarded */
                    } else {
                      p2p_net_send_packet( net_send, &cur_sp->fd, buf_l, 0, P2P_NET_TYPE_DENY );
                    }
                  }


                } else {
                  p2p_net_send_packet( net_send, &cur_sp->fd, buf_l, 0, P2P_NET_TYPE_DENY );
                }

              break;

              default:
                ;
                if( ctx->cbs.invalid_type ) ctx->cbs.invalid_type( i, ctx->cbs_param );
              break;

            } /* END CMD CASE */

          } /* END IF DATA */

        } else { /* END IF NOT FORWARD */
          int r;
          unsigned char buf[P2P_FORWARD_SIZE];
          p2p_server_peer_ctx *in, *out;
          in = (p2p_server_peer_ctx*) cur_sp;
          out = (p2p_server_peer_ctx*) ctx->sp[in->forward_i];
          r = net_recv( &in->fd, buf, sizeof(buf) );
          if( r < 0 ); /* No data available */
          else if ( r == 0 ){ /* Disconnected */
            if( ctx->cbs.peer_disconnect ){
              ctx->cbs.peer_disconnect( i, ctx->cbs_param );
              ctx->cbs.peer_disconnect( in->forward_i, ctx->cbs_param );
            }
            p2p_server_free_peer(ctx, i);
            p2p_server_free_peer(ctx, in->forward_i);
          } else {
            net_send( &out->fd, buf, r );
          }


        }

      }
    }

  }

exit:
  return ret;
}
