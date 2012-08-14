/*
 * $Id$
 *
 * DEBUG: section 54    Interprocess Communication
 *
 */

#include "squid.h"
#include "base/TextException.h"
#include "CpuAffinity.h"
#include "CpuAffinityMap.h"
#include "CpuAffinitySet.h"
#include "Debug.h"
#include "protos.h"
#include "structs.h"

#include <algorithm>

static CpuAffinitySet *TheCpuAffinitySet = NULL;


void
CpuAffinityInit()
{
    Must(!TheCpuAffinitySet);
    if (Config.cpuAffinityMap) {
        const int processNumber = InDaemonMode() ? KidIdentifier : 1;
        TheCpuAffinitySet = Config.cpuAffinityMap->calculateSet(processNumber);
        if (TheCpuAffinitySet)
            TheCpuAffinitySet->apply();
    }
}

void
CpuAffinityReconfigure()
{
    if (TheCpuAffinitySet) {
        TheCpuAffinitySet->undo();
        delete TheCpuAffinitySet;
        TheCpuAffinitySet = NULL;
    }
    CpuAffinityInit();
}

void
CpuAffinityCheck()
{
    if (Config.cpuAffinityMap) {
        Must(!Config.cpuAffinityMap->processes().empty());
        const int maxProcess =
            *std::max_element(Config.cpuAffinityMap->processes().begin(),
                              Config.cpuAffinityMap->processes().end());

        // in no-deamon mode, there is one process regardless of squid.conf
        const int numberOfProcesses = InDaemonMode() ? NumberOfKids() : 1;

        if (maxProcess > numberOfProcesses) {
            debugs(54, DBG_IMPORTANT, "WARNING: 'cpu_affinity_map' has "
                   "non-existing process number(s)");
        }
    }
}
