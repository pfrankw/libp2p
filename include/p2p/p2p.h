#ifndef P2P_H
#define P2P_H


#include <signal.h>
#include <string.h>
#include <arpa/inet.h>

#include <p2p/config.h>
#include <p2p/server/server.h>
#include <p2p/cache.h>
#include <p2p/x509.h>
#include <p2p/rsa.h>
#include <p2p/desc.h>

/*

  This is the "bird's eye view" part of the code.
  There are two main uses of the library.
  The first is to connect to N random super nodes (P2P Nodes with opened ports),
  to fetch their cache and advertise of their presence in the network.

  The second is to help the developer by giving the easier way to connect to another peer
  by only supplying the respective digest.

*/

typedef struct _p2p_conn_ctx {
  int fd;
  x509_crt *crt;
  pk_context *pk;

} p2p_conn_ctx;


typedef struct _p2p_ctx {
  p2p_conn_ctx *c[P2P_ADVERTISE_CONNECTIONS];
  p2p_server_ctx server;
  p2p_cache_ctx cache;

  x509_crt *crt;
  pk_context *pk;
  pthread_t manager_tid;

  int cm_run; /* p2p_conn_manager run flag */
  int (*cb_verify_conn)(p2p_desc_peer_min *desc, void *param);
  int (*cb_conn)(p2p_desc_peer_min *desc, p2p_conn_ctx *conn, void *param);
  void *cb_param;

} p2p_ctx;



int p2p_init(p2p_ctx *ctx, x509_crt *crt, pk_context *pk, void *cb_verify_conn, void *cb_conn, void *cb_param );
int p2p_free(p2p_ctx *ctx);

void p2p_cb_sigint( int sig );

int p2p_init_cache(p2p_ctx *ctx);
int p2p_init_server(p2p_ctx *ctx);

int p2p_init_conn( p2p_ctx *ctx, int i, int ip );
int p2p_free_conn( p2p_ctx *ctx, int i );
int p2p_free_ext_conn( p2p_conn_ctx *conn ); /* Frees an external conn_ctx */
int p2p_detach_conn(p2p_ctx *ctx, int i, p2p_conn_ctx **conn);
int p2p_init_all_conn(p2p_ctx *ctx);
int p2p_free_all_conn(p2p_ctx *ctx);

int p2p_conn_manager(p2p_ctx *ctx);

int p2p_cb_server_new_client(int i, int ip, void *cb_param);
int p2p_cb_server_packet_recv(int i, int size, int type, void *cb_param);
int p2p_cb_server_peer_auth(int i, p2p_desc_peer_min *desc, void *cb_param);
int p2p_cb_server_peer_forward(int i, int fwd_i, void *cb_param);
int p2p_cb_server_peer_disconnect(int i, void *cb_param);
int p2p_cb_server_invalid_type(int i, void *cb_param);
int p2p_cb_server_recv_error(int i, int e, void *cb_param);





#endif
