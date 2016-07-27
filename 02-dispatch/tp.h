#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER dispatch

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./tp.h"

#if !defined(_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TP_H
#include <lttng/tracepoint.h>
TRACEPOINT_EVENT(
        dispatch,
        entry,
        TP_ARGS(int, begin, int, end, int, color),
        TP_FIELDS(
            ctf_integer(int, begin, begin)
            ctf_integer(int, end, end)
            ctf_integer(int, color, color)
        )
)

TRACEPOINT_EVENT(dispatch, exit, TP_ARGS(), TP_FIELDS())

#endif /* _TP_H */

#include <lttng/tracepoint-event.h>
