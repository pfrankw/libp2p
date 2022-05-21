#ifndef P2P_CONFIG_H
#define P2P_CONFIG_H

#define P2P_VERSION 0x00000001 /* 0.0.1 */

#define P2P_ADVERTISE_CONNECTIONS 5

#define P2P_CACHE_MAX_SECTIONS 10000
#define P2P_CACHE_REMOVE_TIME 60 /* Seconds needed to consider a p2p_cache_section too old */

#define P2P_SERVER_MAX_PEERS 100
#define P2P_MAX_PACKET_SIZE 4096
#define P2P_DIGEST_SIZE 20

#define P2P_FORWARD_SIZE 4096



#define P2P_CACHE_FILE "cache.hex"

#endif
