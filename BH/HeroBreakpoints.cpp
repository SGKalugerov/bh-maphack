#include "HeroBreakpoints.h"

std::unordered_map<int, Breakpoints> classBreakpoints = {
    {AMAZON, {{{0, 19}, {7, 18}, {14, 17}, {22, 16}, {32, 15}, {48, 14}, {68, 13}, {99, 12}, {152, 11}},  // FCR
              {{0, 11}, {6, 10}, {13, 9}, {20, 8}, {32, 7}, {52, 6}, {86, 5}, {174, 4}},                 // FHR
              {{0, 5}, {13, 4}, {32, 3}, {86, 2}}}},                                                     // FBR

    {SORCERESS, {{{0, 13}, {9, 12}, {20, 11}, {37, 10}, {63, 9}, {105, 8}, {200, 7}},                     // FCR
                 {{0, 15}, {5, 14}, {9, 13}, {14, 12}, {20, 11}, {30, 10}, {42, 9}, {60, 8}, {86, 7}, {142, 6}, {280, 5}},  // FHR
                 {{0, 9}, {11, 8}, {15, 7}, {27, 6}, {48, 5}, {86, 4}}}},                                 // FBR

    {NECROMANCER, {{{0, 15}, {9, 14}, {18, 13}, {30, 12}, {48, 11}, {75, 10}, {125, 9}},                  // FCR
                   {{0, 13}, {5, 12}, {10, 11}, {16, 10}, {26, 9}, {39, 8}, {56, 7}, {86, 6}, {152, 5}},  // FHR
                   {{0, 11}, {13, 10}, {17, 9}, {27, 8}, {48, 7}, {86, 6}}}},                             // FBR

    {PALADIN, {{{0, 15}, {9, 14}, {18, 13}, {30, 12}, {48, 11}, {75, 10}, {125, 9}},                      // FCR
               {{0, 9}, {7, 8}, {15, 7}, {27, 6}, {48, 5}, {86, 4}},                                      // FHR
               {{0, 5}, {9, 4}, {20, 3}, {42, 2}, {86, 1}}}},                                             // FBR

    {BARBARIAN, {{{0, 13}, {9, 12}, {20, 11}, {37, 10}, {63, 9}, {105, 8}, {200, 7}},                     // FCR
                 {{0, 9}, {7, 8}, {15, 7}, {27, 6}, {48, 5}, {86, 4}, {200, 3}},                          // FHR
                 {{0, 9}, {7, 8}, {15, 7}, {27, 6}, {48, 5}, {86, 4}}}},                                  // FBR

    {DRUID, {{{0, 18}, {11, 17}, {18, 16}, {30, 15}, {46, 14}, {68, 13}, {99, 12}, {163, 11}},            // FCR
             {{0, 13}, {5, 12}, {10, 11}, {16, 10}, {26, 9}, {39, 8}, {56, 7}, {86, 6}, {174, 5}},         // FHR
             {{0, 11}, {14, 10}, {20, 9}, {32, 8}, {52, 7}, {86, 6}}}},                                   // FBR

    {ASSASSIN, {{{0, 16}, {7, 15}, {15, 14}, {27, 13}, {42, 12}, {65, 11}, {102, 10}, {174, 9}},           // FCR
                {{0, 9}, {7, 8}, {15, 7}, {27, 6}, {48, 5}, {86, 4}},                                     // FHR
                {{0, 5}, {8, 4}, {16, 3}, {27, 2}, {86, 1}}}}                                             // FBR
};

std::pair<int, int> getHighestBreakpoint(const std::vector<std::pair<int, int>>& breakpoints, int currentStat) {
    std::pair<int, int> lastHitBreakpoint = { 0, 0 };  
    for (const auto& bp : breakpoints) {
        if (currentStat >= bp.first) {
            lastHitBreakpoint = bp;  
        }
        else {
            break;  
        }
    }
    return lastHitBreakpoint;
}

int getCurrentFCRFrame(int classNum, int currentFCR) {
    std::pair<int, int> fcrBreakpoint = getHighestBreakpoint(classBreakpoints[classNum].fasterCastRate, currentFCR);
    return fcrBreakpoint.second;
}

int getCurrentFBRFrame(int classNum, int currentFBR) {
    std::pair<int, int> fbrBreakpoint = getHighestBreakpoint(classBreakpoints[classNum].fasterBlockRate, currentFBR);
    return fbrBreakpoint.second;
}

int getCurrentFHRFrame(int classNum, int currentFHR) {
    std::pair<int, int> fhrBreakpoint = getHighestBreakpoint(classBreakpoints[classNum].fasterHitRecovery, currentFHR);
    return fhrBreakpoint.second;
}
