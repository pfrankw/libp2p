#ifndef P2P_SERVER_H
#define P2P_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>



#include <p2p/config.h>
#include <p2p/err.h>
#include <p2p/log.h>
#include <p2p/net.h>
#include <p2p/cache.h>
#include <p2p/desc.h>

#include <p2p/base32.h>
#include <polarssl/net.h>

typedef struct _p2p_server_peer_ctx {
  int fd;
  p2p_desc_peer_min desc;
  int forward_i;

} p2p_server_peer_ctx;


typedef struct _p2p_server_cbs {
  int (*new_client)(int i, int ip, void *cb_param);
  int (*packet_recv)(int i, int size, int type, void *cb_param);
  int (*peer_auth)(int i, p2p_desc_peer_min *desc, void *cb_param);
  int (*peer_forward)(int i, int fwd_i, void *cb_param);
  int (*peer_disconnect)(int i, void *cb_param);
  int (*invalid_type)(int i, void *cb_param);
  int (*recv_error)(int i, int e, void *cb_param);
} p2p_server_cbs;

typedef struct _p2p_server_ctx {
  int lfd; /* Listen FD */
  int port;
  int run;
  pthread_t manager_tid; /* Manager thread ID */
  pthread_t listener_tid; /* Listener thread ID */
  //int (*p2p_new_server_peer_cb)(p2p_server_peer_ctx *);

  void *cbs_param;
  p2p_server_cbs cbs;
  p2p_server_peer_ctx *sp[P2P_SERVER_MAX_PEERS]; /* Remote connected peers */
  p2p_cache_ctx *cache; /* Initialized cache context or NULL if not supported */

} p2p_server_ctx;

int p2p_server_init(p2p_server_ctx *ctx, int port);
int p2p_server_listen(p2p_server_ctx *ctx);
void p2p_server_stop(p2p_server_ctx *ctx);

int p2p_server_set_cache(p2p_server_ctx *ctx, p2p_cache_ctx *cache);  /* cache can be 0 */
int p2p_server_set_cbs(p2p_server_ctx *ctx, p2p_server_cbs *cbs, void *cbs_param);

int p2p_server_get_peer_by_digest(p2p_server_ctx *ctx, unsigned char *pk_digest, int *index);

int p2p_server_alloc_peer(p2p_server_ctx *ctx, int fd);
int p2p_server_free_peer(p2p_server_ctx *ctx, int remote_peer_ctx_i);
int p2p_server_manager(p2p_server_ctx *ctx);

#endif
