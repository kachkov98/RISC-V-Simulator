#include "stats.h"

namespace stats {
#define STAT_DEF(var, desc) unsigned long long var = 0;
#include "stats_list.h"
#undef STAT_DEF

void PrintStatistics(FILE *f) {
#define STAT_DEF(var, desc) fprintf(f, "%s: %llu\n", desc, var);
#include "stats_list.h"
#undef STAT_DEF
}
} // namespace stats
