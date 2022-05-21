#ifndef P2P_DESC_H
#define P2P_DESC_H

#include <p2p/config.h>

/* Minimal peer descriptor: Contains the minimal informations to identify a peer */
typedef struct _p2p_desc_peer_min {
  unsigned char pk_digest[P2P_DIGEST_SIZE];
} p2p_desc_peer_min;

/* Full peer descriptor: Contains all the informations of a peer */
typedef struct _p2p_desc_peer_full {
  
} p2p_desc_peer_full;



#endif
