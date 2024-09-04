#ifndef HERO_BREAKPOINTS_H
#define HERO_BREAKPOINTS_H

#include <unordered_map>
#include <vector>
#include <utility> 

enum ClassId {
    AMAZON = 0,
    SORCERESS = 1,
    NECROMANCER = 2,
    PALADIN = 3,
    BARBARIAN = 4,
    DRUID = 5,
    ASSASSIN = 6
};

struct Breakpoints {
    std::vector<std::pair<int, int>> fasterCastRate;
    std::vector<std::pair<int, int>> fasterBlockRate;
    std::vector<std::pair<int, int>> fasterHitRecovery;
};

extern std::unordered_map<int, Breakpoints> classBreakpoints;

int getCurrentFCRFrame(int classNum, int currentFCR);

int getCurrentFBRFrame(int classNum, int currentFBR);

int getCurrentFHRFrame(int classNum, int currentFHR);

std::pair<int, int> getHighestBreakpoint(const std::vector<std::pair<int, int>>& breakpoints, int currentStat);

#endif
