#include <volt/plugin/diamond_structure_identifier_log.h>
#include <volt/core/type_counts.h>

#include <spdlog/spdlog.h>

#include <utility>

namespace Volt{

void DiamondStructureIdentifierLog::dump(const std::vector<DiamondStructureType>& types){
    spdlog::info("Classified {} atoms in {}", types.size(), phaseElapsed());
}

void DiamondStructureIdentifierLog::dump(const std::map<int, int>& counts){
    static const std::pair<DiamondStructureType, const char*> listed[] = {
        {DiamondStructureType::CUBIC_DIAMOND, "Cubic diamond"},
        {DiamondStructureType::CUBIC_DIAMOND_FIRST_NEIGH, "1st neighbors"},
        {DiamondStructureType::CUBIC_DIAMOND_SECOND_NEIGH, "2nd neighbors"},
        {DiamondStructureType::HEX_DIAMOND, "Hexagonal diamond"},
        {DiamondStructureType::HEX_DIAMOND_FIRST_NEIGH, "1st neighbors"},
        {DiamondStructureType::HEX_DIAMOND_SECOND_NEIGH, "2nd neighbors"},
        {DiamondStructureType::OTHER, "Other"},
    };
    TypeCounts line{"Structure types"};
    for(const auto& [type, name] : listed){
        const auto count = counts.find(static_cast<int>(type));
        line.counts.emplace_back(name, count != counts.end() ? count->second : 0);
        line.atoms += line.counts.back().second;
    }
    dump(line);
}

}
