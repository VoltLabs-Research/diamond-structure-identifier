#pragma once

#include <volt/core/volt.h>
#include <volt/core/lammps_parser.h>
#include <volt/structures/crystal_structure_types.h>
#include <nlohmann/json.hpp>
#include <string>

namespace Volt {

using json = nlohmann::json;

enum class DiamondStructureType : int {
    OTHER                       = 0,
    CUBIC_DIAMOND               = 5,
    CUBIC_DIAMOND_FIRST_NEIGH   = 9,
    CUBIC_DIAMOND_SECOND_NEIGH  = 10,
    HEX_DIAMOND                 = 6,
    HEX_DIAMOND_FIRST_NEIGH     = 11,
    HEX_DIAMOND_SECOND_NEIGH    = 12,
};

inline const char* diamondStructureName(DiamondStructureType t) {
    switch (t) {
        case DiamondStructureType::CUBIC_DIAMOND:              return "CUBIC_DIAMOND";
        case DiamondStructureType::CUBIC_DIAMOND_FIRST_NEIGH:  return "CUBIC_DIAMOND_FIRST_NEIGH";
        case DiamondStructureType::CUBIC_DIAMOND_SECOND_NEIGH: return "CUBIC_DIAMOND_SECOND_NEIGH";
        case DiamondStructureType::HEX_DIAMOND:                return "HEX_DIAMOND";
        case DiamondStructureType::HEX_DIAMOND_FIRST_NEIGH:    return "HEX_DIAMOND_FIRST_NEIGH";
        case DiamondStructureType::HEX_DIAMOND_SECOND_NEIGH:   return "HEX_DIAMOND_SECOND_NEIGH";
        default:                                                return "OTHER";
    }
}

class IdentifyDiamondService {
public:
    json compute(const LammpsParser::Frame& frame, const std::string& outputBase);
};

}
