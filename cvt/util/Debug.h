#ifndef CVT_DEBUG_H
#define CVT_DEBUG_H

#include <execinfo.h>

namespace Debug {

    inline void printBacktrace()
    {
        const size_t SIZE = 200;
        void* traceptr[ SIZE ];
        size_t size;

        size = backtrace( traceptr, SIZE );
        backtrace_symbols_fd( traceptr, size, fileno( stderr ) );
    }
}

#endif
