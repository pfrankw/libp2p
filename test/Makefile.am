AM_CFLAGS = -Wall -static -I$(top_srcdir)/include
AM_LDFLAGS = -L/usr/local/lib -L$(top_srcdir)/src

noinst_PROGRAMS = test1 test2 test_cache test_forward test_rcache
test1_SOURCES = test1.c
test1_LDADD = -lp2p -lmbedtls -lpolarssl -lpthread
test2_SOURCES = test2.c
test2_LDADD = -lp2p -lmbedtls -lpolarssl -lpthread
test_cache_SOURCES = test_cache.c
test_cache_LDADD = -lp2p -lmbedtls -lpolarssl -lpthread
test_forward_SOURCES = test_forward.c
test_forward_LDADD = -lp2p -lmbedtls -lpolarssl -lpthread
test_rcache_SOURCES = test_rcache.c
test_rcache_LDADD = -lp2p -lmbedtls -lpolarssl -lpthread
