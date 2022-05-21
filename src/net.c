#include <p2p/net.h>



int p2p_net_recv(void *ctx, unsigned char *buf, size_t size){
  return net_recv( ctx, buf, size );
}
int p2p_net_send(void *ctx, const unsigned char *buf, size_t size){
  return net_send( ctx, buf, size );
}



int p2p_net_recv_packet( int (*recv_func)(void*, unsigned char*, size_t), void *ctx, unsigned char *buf, int size ){

  int r, r2;
  int rem_size; /* Remaining buffer size after the header packet has been read */
  p2p_net_packet_header *ph;

  if( !recv_func || !ctx || !buf || !size ){
    return P2P_ERR_NULL_PARAM;
  }

  if( size < sizeof(p2p_net_packet_header) ){ /* The buffer is too small to handle the packet header */
    return P2P_ERR_NET_SMALL_BUF;
  }

  r = recv_func( ctx, buf, sizeof(p2p_net_packet_header) );

  if( r < 0 ) /* No data is available to be read */
    return P2P_ERR_NET_NODATA;

  if( r == 0 ) /* Read 0 bytes, this means the connection was closed */
    return P2P_ERR_NET_DISCONNECTED;

  ph = (p2p_net_packet_header*) buf;

  if( !ph->size ) /* No resting data, the packet is formed only by the packet header */
    return r;


  if( ph->size > P2P_MAX_PACKET_SIZE )  /* If the remaining data is too big */
    return P2P_ERR_NET_BAD_PACKET;

  rem_size = size - sizeof(p2p_net_packet_header);

  if( rem_size < ph->size ){ /* The buffer is too small to handle the resting data, so we read it from the socket into a dummy var and exit from the function with an error */

    int i;
    unsigned char c;

    for(i=0; i<ph->size; i++)
      recv_func( ctx, &c, 1 );

    return P2P_ERR_NET_SMALL_BUF;
  }

  r2 = recv_func( ctx, buf + sizeof(p2p_net_packet_header), ph->size );

  if( r2 != ph->size ) /* The packet header lied about the remaining data size */
    return P2P_ERR_NET_BAD_PACKET;


  return r + r2;

}



int p2p_net_send_packet( int(*send_func)(void*, const unsigned char*, size_t), void *ctx, unsigned char *buf, int size, int packet_type ){

  int r, total_size;
  p2p_net_packet_header ph;
  unsigned char *send_buf;

  if( !send_func || !ctx )
    return P2P_ERR_NULL_PARAM;

  if( size > P2P_MAX_PACKET_SIZE ) /* If the buffer size is greater than the max permitted */
    return P2P_ERR_NET_BAD_PACKET;

  total_size = size + sizeof(p2p_net_packet_header);
  send_buf = (unsigned char*) malloc ( total_size ); /* Allocating size of buffer + size of packet header */

  if( !send_buf ) /* Real rare condition */
    return P2P_ERR_DENIED;

  ph.version = P2P_VERSION; /* Static */
  ph.type = packet_type;
  ph.size = size;

  memcpy(send_buf, &ph, sizeof(p2p_net_packet_header) ); /* Copying the the packet header at the start of the send_buf */
  memcpy(send_buf + sizeof(p2p_net_packet_header), buf, size ); /* Appending the buffer to send_buf */

  r = send_func( ctx, send_buf, total_size );

  free( send_buf );

  if( r == 0 ) /* Disconnected */
    return P2P_ERR_NET_DISCONNECTED;

  if( r < 0 ) /* Generic send error */
    return P2P_ERR_NET_SEND;

  return r;
}



int p2p_net_authenticate( int (*recv_func)(void*, unsigned char*, size_t), int (*send_func)(void*, const unsigned char*, size_t), void *ctx, p2p_desc_peer_min *desc ){ /* To be changed with a signature system */

  int r;
  p2p_net_packet_header p;

  if( !recv_func || !send_func || !ctx || !desc )
    return P2P_ERR_NULL_PARAM;


  if( p2p_net_send_packet( send_func, ctx, (unsigned char*)desc, sizeof(p2p_desc_peer_min), P2P_NET_TYPE_AUTH ) <= 0 )
    return P2P_ERR_NET_SEND;

  do {  /* It is blocking, needs to be fixed */
    r = p2p_net_recv_packet( recv_func, ctx, (unsigned char*)&p, sizeof(p2p_net_packet_header) );
  } while ( r == P2P_ERR_NET_NODATA );

  if( r <= 0 )
    return P2P_ERR_NET_RECV;

  if( p.type == P2P_NET_TYPE_ACK )
    return P2P_ERR_OK;

  return P2P_ERR_DENIED;

}


int p2p_net_forward( int (*recv_func)(void*, unsigned char*, size_t), int (*send_func)(void*, const unsigned char*, size_t), void *ctx, p2p_desc_peer_min *desc ){

  int r;
  p2p_net_packet_header p;
  p2p_net_forward_packet fp;

  if( !recv_func || !send_func || !ctx || !desc )
    return P2P_ERR_NULL_PARAM;

  memcpy( &fp.desc, desc, sizeof(p2p_desc_peer_min) );
  if( p2p_net_send_packet( send_func, ctx, (unsigned char*)&fp, sizeof(p2p_net_forward_packet), P2P_NET_TYPE_FORWARD ) <= 0 )
    return P2P_ERR_NET_SEND;

  do {  /* It is blocking, needs to be fixed */
    r = p2p_net_recv_packet( recv_func, ctx, (unsigned char*)&p, sizeof(p2p_net_packet_header) );
  } while ( r == P2P_ERR_NET_NODATA );

  if( r<= 0 )
    return P2P_ERR_NET_RECV;

  if( p.type == P2P_NET_TYPE_ACK )
    return P2P_ERR_OK;

  return P2P_ERR_DENIED;
}
