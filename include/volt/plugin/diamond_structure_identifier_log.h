#pragma once

#include <volt/core/run_log.h>
#include <volt/plugin/identify_diamond_service.h>

#include <map>
#include <vector>

namespace Volt{

// The execution log of one diamond structure identification.
class DiamondStructureIdentifierLog : public RunLog{
public:
    using RunLog::dump;

    void dump(const std::vector<DiamondStructureType>& types);
    // The atom count of each structure type, keyed by type.
    void dump(const std::map<int, int>& counts);
};

}
