#ifndef __DEBUG_H
#define __DEBUG_H

// debugging functionality
#ifndef NDEBUG
void _debug_level(char*, observable_t, int);
#define _debug(t,o) _debug_level(t,o,0)
#else
#define _debug(t,o) ((void)0)
#endif

#endif
