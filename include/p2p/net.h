#ifndef P2P_NET_H
#define P2P_NET_H


#include <string.h>
#include <stdlib.h>

#include <polarssl/net.h>

#include <p2p/config.h>
#include <p2p/err.h>
#include <p2p/log.h>
#include <p2p/desc.h>



#define P2P_NET_TYPE_AUTH 0
#define P2P_NET_TYPE_LIST_PEERS 1
#define P2P_NET_TYPE_FORWARD 2
#define P2P_NET_TYPE_NUMBER 3
#define P2P_NET_TYPE_CACHE 4

#define P2P_NET_TYPE_ACK 10
#define P2P_NET_TYPE_DENY 11


typedef struct _p2p_net_packet_header {
  unsigned int version;
  int type;
  unsigned int size;
} p2p_net_packet_header;


typedef struct _p2p_net_forward_packet {
  p2p_desc_peer_min desc;
} p2p_net_forward_packet;


int p2p_net_recv( void *ctx, unsigned char *buf, size_t size );
int p2p_net_send( void *ctx, const unsigned char *buf, size_t size );

int p2p_net_recv_packet( int (*recv_func)(void*, unsigned char*, size_t), void *ctx, unsigned char *buf, int size ); /* Receives and parses a p2p packet */

int p2p_net_send_packet( int (*send_func)(void*, const unsigned char*, size_t), void *ctx, unsigned char *buf, int size, int packet_type ); /* Crafts and sends a p2p packet */


/* Client functions */
int p2p_net_authenticate( int (*recv_func)(void*, unsigned char*, size_t), int (*send_func)(void*, const unsigned char*, size_t), void *ctx, p2p_desc_peer_min *desc );
int p2p_net_forward( int (*recv_func)(void*, unsigned char*, size_t), int (*send_func)(void*, const unsigned char*, size_t), void *ctx, p2p_desc_peer_min *desc );

#endif
