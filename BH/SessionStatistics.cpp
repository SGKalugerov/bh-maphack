#include "SessionStatistics.h"
#include <fstream>

std::unordered_set<uint64_t> SessionStatistics::killedUnits;
uint64_t SessionStatistics::lifetimeKills = 0;  

void SessionStatistics::addKilledUnit(uint64_t unitId) {
    if (!hasUnitBeenKilled(unitId)) {
        killedUnits.insert(unitId);
        lifetimeKills++;  
    }
}

bool SessionStatistics::hasUnitBeenKilled(uint64_t unitId) {
    return killedUnits.find(unitId) != killedUnits.end();
}

uint64_t SessionStatistics::getCurrentKillCount() {
    return killedUnits.size();
}

uint64_t SessionStatistics::getLifetimeKillCount() {
    return lifetimeKills;
}

nlohmann::json SessionStatistics::statisticsToJson() {
    nlohmann::json j;
    j["lifetimeKills"] = lifetimeKills;
    return j;
}

void SessionStatistics::statisticsFromJson(const nlohmann::json& j) {
    if (j.contains("lifetimeKills")) {
        lifetimeKills = j["lifetimeKills"].get<uint64_t>();
    }
    else {
        lifetimeKills = 0;
    }
}

void SessionStatistics::saveStatistics(const std::string& filename) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        nlohmann::json j = statisticsToJson();
        outFile << j.dump(4);  
        outFile.close();
    }
    else {
        printf("Failed to open %s for writing\n", filename.c_str());
    }
}

void SessionStatistics::resetCurrentKillCount() {
    killedUnits.clear(); 
}

void SessionStatistics::loadStatistics(const std::string& filename) {
    std::ifstream inFile(filename);
    if (inFile.is_open()) {
        nlohmann::json j;
        inFile >> j;
        statisticsFromJson(j);
        inFile.close();
    }
    else {
        lifetimeKills = 0; 
        printf("Failed to open %s for reading\n", filename.c_str());
    }
}
