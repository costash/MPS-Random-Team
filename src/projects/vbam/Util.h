
#if defined ( WIN32 )
#define __func__ __FUNCTION__
#endif

#define DBG_LEVEL 1
#define dbg( level, format, ...) if( level >= DBG_LEVEL ) fprintf (stderr, format, ## __VA_ARGS__)

typedef unsigned int uint;

#define _assert( condition, call_descr, ... )								    \
{																				\
	if( !(condition) )															\
	{																			\
		fprintf(stderr, "%s: %s()-> line %d\n", __FILE__, __func__, __LINE__);	\
		fprintf(stderr, call_descr, ## __VA_ARGS__ );							\
		perror("");						 									    \
		exit( EXIT_FAILURE );													\
	}																			\
}