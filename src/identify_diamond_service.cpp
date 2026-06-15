#include <volt/plugin/identify_diamond_service.h>
#include <volt/analysis/nearest_neighbor_finder.h>
#include <volt/analysis/cna_classifier.h>
#include <volt/structures/neighbor_bond_array.h>
#include <volt/core/frame_adapter.h>
#include <volt/core/analysis_result.h>
#include <volt/plugin/output_serializer.h>
#include <spdlog/spdlog.h>

#include <array>
#include <cmath>

namespace Volt {

json IdentifyDiamondService::compute(const LammpsParser::Frame& frame, const std::string& outputBase) {
    if (frame.natoms <= 0)
        return AnalysisResult::failure("Invalid number of atoms");

    auto positions = FrameAdapter::createPositionPropertyShared(frame);
    if (!positions)
        return AnalysisResult::failure("Failed to create position property");

    // 4 nearest neighbours per atom (diamond is 4-coordinate).
    NearestNeighborFinder neighFinder(4);
    if (!neighFinder.prepare(positions.get(), frame.simulationCell))
        return AnalysisResult::failure("NearestNeighborFinder::prepare failed");

    const size_t N = static_cast<size_t>(frame.natoms);

    struct NeighInfo {
        Vector3 vec;
        int     index;
    };
    std::vector<std::array<NeighInfo, 4>> neighLists(N);

    // Build 4-NN lists.
    for (size_t idx = 0; idx < N; ++idx) {
        NearestNeighborFinder::Query<4> q(neighFinder);
        q.findNeighbors(idx);
        const int found = static_cast<int>(q.results().size());
        for (int i = 0; i < found; ++i) {
            neighLists[idx][i].vec   = q.results()[i].delta;
            neighLists[idx][i].index = static_cast<int>(q.results()[i].index);
        }
        for (int i = found; i < 4; ++i) {
            neighLists[idx][i].vec.setZero();
            neighLists[idx][i].index = -1;
        }
    }

    std::vector<DiamondStructureType> types(N, DiamondStructureType::OTHER);

    // Pass 1: identify atoms in cubic/hex diamond cores via 2nd-shell CNA.
    for (size_t idx = 0; idx < N; ++idx) {
        const auto& nlist = neighLists[idx];

        // Generate 12 second-nearest-neighbour vectors.
        std::array<Vector3, 12> secondNeighbors;
        auto vout = secondNeighbors.begin();
        bool ok = true;
        for (int i = 0; i < 4; ++i) {
            if (nlist[i].index < 0) { ok = false; break; }
            const Vector3& v0 = nlist[i].vec;
            const auto& nlist2 = neighLists[static_cast<size_t>(nlist[i].index)];
            for (int j = 0; j < 4; ++j) {
                Vector3 v = v0 + nlist2[j].vec;
                if (v.isZero(1e-2f)) continue;
                if (vout == secondNeighbors.end()) { ok = false; break; }
                *vout++ = v;
            }
            if (!ok) break;
            if (vout != secondNeighbors.begin() + i * 3 + 3) { ok = false; break; }
        }
        if (!ok) continue;

        // Local CNA cutoff from average second-neighbour distance.
        double sum = 0.0;
        for (const Vector3& v : secondNeighbors)
            sum += v.length();
        sum /= 12.0;
        const double localCutoffSq = (sum * 1.2071068) * (sum * 1.2071068);

        // Build neighbour bond array for 12 second-shell atoms.
        NeighborBondArray nbArray;
        for (int ni1 = 0; ni1 < 12; ++ni1) {
            nbArray.setNeighborBond(ni1, ni1, false);
            for (int ni2 = ni1 + 1; ni2 < 12; ++ni2)
                nbArray.setNeighborBond(ni1, ni2,
                    (secondNeighbors[ni1] - secondNeighbors[ni2]).squaredLength() <= localCutoffSq);
        }

        // CNA on each of the 12 second-shell bonds; count 4-2-1 and 4-2-2.
        int n421 = 0, n422 = 0;
        bool valid = true;
        for (int ni = 0; ni < 12; ++ni) {
            unsigned int commonNeighbors = 0;
            int numCN = CommonNeighborAnalysis::findCommonNeighbors(nbArray, ni, commonNeighbors, 12);
            if (numCN != 4) { valid = false; break; }

            CommonNeighborAnalysis::CNAPairBond neighborBonds[144];
            int numBonds = CommonNeighborAnalysis::findNeighborBonds(nbArray, commonNeighbors, 12, neighborBonds);
            if (numBonds != 2) { valid = false; break; }

            int chainLen = CommonNeighborAnalysis::calcMaxChainLength(neighborBonds, numBonds);
            if      (chainLen == 1) ++n421;
            else if (chainLen == 2) ++n422;
            else                  { valid = false; break; }
        }
        if (!valid) continue;

        if (n421 == 12)              types[idx] = DiamondStructureType::CUBIC_DIAMOND;
        else if (n421 == 6 && n422 == 6) types[idx] = DiamondStructureType::HEX_DIAMOND;
    }

    // Pass 2: mark first-shell neighbours of crystalline cores.
    for (size_t idx = 0; idx < N; ++idx) {
        const auto ct = types[idx];
        if (ct != DiamondStructureType::CUBIC_DIAMOND && ct != DiamondStructureType::HEX_DIAMOND) continue;
        for (int i = 0; i < 4; ++i) {
            int j = neighLists[idx][i].index;
            if (j < 0 || types[static_cast<size_t>(j)] != DiamondStructureType::OTHER) continue;
            types[static_cast<size_t>(j)] =
                (ct == DiamondStructureType::CUBIC_DIAMOND)
                    ? DiamondStructureType::CUBIC_DIAMOND_FIRST_NEIGH
                    : DiamondStructureType::HEX_DIAMOND_FIRST_NEIGH;
        }
    }

    // Pass 3: mark second-shell neighbours.
    for (size_t idx = 0; idx < N; ++idx) {
        const auto ct = types[idx];
        if (ct != DiamondStructureType::CUBIC_DIAMOND_FIRST_NEIGH &&
            ct != DiamondStructureType::HEX_DIAMOND_FIRST_NEIGH) continue;
        for (int i = 0; i < 4; ++i) {
            int j = neighLists[idx][i].index;
            if (j < 0 || types[static_cast<size_t>(j)] != DiamondStructureType::OTHER) continue;
            types[static_cast<size_t>(j)] =
                (ct == DiamondStructureType::CUBIC_DIAMOND_FIRST_NEIGH)
                    ? DiamondStructureType::CUBIC_DIAMOND_SECOND_NEIGH
                    : DiamondStructureType::HEX_DIAMOND_SECOND_NEIGH;
        }
    }

    // Tally counts.
    std::map<int, int> counts;
    for (auto t : types)
        counts[static_cast<int>(t)]++;

    json result;
    result["main_listing"] = {
        {"total_atoms",                    frame.natoms},
        {"OTHER",                          counts[0]},
        {"CUBIC_DIAMOND",                  counts[static_cast<int>(DiamondStructureType::CUBIC_DIAMOND)]},
        {"CUBIC_DIAMOND_FIRST_NEIGH",      counts[static_cast<int>(DiamondStructureType::CUBIC_DIAMOND_FIRST_NEIGH)]},
        {"CUBIC_DIAMOND_SECOND_NEIGH",     counts[static_cast<int>(DiamondStructureType::CUBIC_DIAMOND_SECOND_NEIGH)]},
        {"HEX_DIAMOND",                    counts[static_cast<int>(DiamondStructureType::HEX_DIAMOND)]},
        {"HEX_DIAMOND_FIRST_NEIGH",        counts[static_cast<int>(DiamondStructureType::HEX_DIAMOND_FIRST_NEIGH)]},
        {"HEX_DIAMOND_SECOND_NEIGH",       counts[static_cast<int>(DiamondStructureType::HEX_DIAMOND_SECOND_NEIGH)]},
    };

    if (!outputBase.empty()) {
        Plugin::serializePluginOutput(outputBase, frame, result, {
            .summaryFileSuffix  = "_identify_diamond",
            .bucketResolver     = [&types](size_t i) -> std::string {
                return diamondStructureName(types[i]);
            },
            .perAtomColumnWriter = [&types](ColumnarAtomWriter& w, size_t i) {
                w.field("structure_type", static_cast<int64_t>(types[i]));
            },
        });
    }

    return result;
}

} // namespace Volt
