#ifdef __linux__
#define _GNU_SOURCE 
#include <sched.h>
#endif
#include <ws_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>




#include <ws_types.h>
#include <ws_mem.h>
#include <ws_time.h>
#include <ws_cycle.h>
#include <ws_log.h>
#include <ws_event.h>
#include <ws_process.h>
#include <ws_process_pipe.h>
#include <ws_os.h>




ws_int32_t ws_cpu_setaffinity()
{
#ifdef __linux__
    cpu_set_t           mask;
    ws_uint32_t     i;
    ws_uint64_t     cpu_affinity;

    cpu_affinity = ws_cycle->cpu_affinity;

    if (cpu_affinity <= 0) {
        return YMZ_OK;
    }

    ws_log_print_debug("sched_setaffinity", cpu_affinity);

    CPU_ZERO(&mask);
    i = 0;
    do {
        if (cpu_affinity & 1) {
            CPU_SET(i, &mask);
        }
        i++;
        cpu_affinity >>= 1;
    } while (cpu_affinity);

    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1) {
        ws_log_print_warn("sched_setaffinity() failed");
    }

#endif

    return YMZ_OK;
}