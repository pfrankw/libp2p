#include <p2p/log.h>



void p2p_log_info(const char* format, ...){
    va_list args;
    va_start( args, format );
    fprintf( stderr, "P2P_LOG_INFO: " );
    vfprintf( stderr, format, args );
    fprintf( stderr, "\n" );
    va_end( args );
}


void p2p_log_error(const char* format, ...){
    va_list args;
    va_start( args, format );
    fprintf( stderr, "P2P_LOG_ERROR: " );
    vfprintf( stderr, format, args );
    fprintf( stderr, "\n" );
    va_end( args );
}
