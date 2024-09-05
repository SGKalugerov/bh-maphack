#pragma once
#ifndef SESSIONSTATISTICS_H
#define SESSIONSTATISTICS_H

#include <unordered_set>
#include <cstdint>  
#include <string>
#include "json.hpp" 

class SessionStatistics {
public:
    SessionStatistics();

    static void addKilledUnit(uint64_t unitId);

    static bool hasUnitBeenKilled(uint64_t unitId);

    static uint64_t getCurrentKillCount();

    static uint64_t getLifetimeKillCount();

    static void loadStatistics(const std::string& filename);

    static void saveStatistics(const std::string& filename);

    static void resetCurrentKillCount();

private:
    static std::unordered_set<uint64_t> killedUnits;

    static uint64_t lifetimeKills;

    static nlohmann::json statisticsToJson();
    static void statisticsFromJson(const nlohmann::json& j);
};

#endif 
