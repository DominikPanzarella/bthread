#include "thelper.h"
#include <sys/time.h>
#include <stdlib.h>

double get_current_time_millis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
}
