#ifndef __unimpl__
#define __unimpl__

#include <stdio.h>

#define UNIMPL if (1) { \
        static int unimpl_warn_once=0; \
        if (!unimpl_warn_once) { \
            printf("MGL warning '%s' is not implemented\n", __FUNCTION__); \
            unimpl_warn_once = 1; \
        } \
    }

#endif
