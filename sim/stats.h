#ifndef STATS_H
#define STATS_H

#include <cstdio>

namespace stats
{
#define STAT_DEF(var, desc) extern unsigned long long var;
#include "stats_list.h"
#undef STAT_DEF

void PrintStatistics(FILE *f);
}

#endif
