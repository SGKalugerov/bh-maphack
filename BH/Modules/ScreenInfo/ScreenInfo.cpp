#include "Screeninfo.h"
#include "../../BH.h"
#include "../../D2Ptrs.h"
#include "../../D2Stubs.h"
#include "../Item/ItemDisplay.h"
#include "../../MPQReader.h"
#include "../../D2Version.h"
#include "../../D2Helpers.h"
#include <time.h>
#include <iomanip>
#include <numeric>
#include <filesystem>

using namespace Drawing;

map<std::string, Toggle> ScreenInfo::Toggles;

void ScreenInfo::OnLoad() {
	LoadConfig();
	

	//buffs
	buffs = { STATE_QUICKNESS,STATE_FADE,STATE_CLOAKED,STATE_VENOMCLAWS,STATE_SHOUT,STATE_BATTLEORDERS,STATE_BATTLECOMMAND,STATE_OAKSAGE,STATE_CYCLONEARMOR,STATE_HURRICANE,STATE_BONEARMOR,STATE_HOLYSHIELD,STATE_FROZENARMOR,STATE_SHIVERARMOR,STATE_CHILLINGARMOR,STATE_ENCHANT,STATE_ENERGYSHIELD,STATE_THUNDERSTORM, STATE_SHRINE_EXPERIENCE,
	//auras
	STATE_MIGHT, STATE_PRAYER, STATE_RESISTFIRE, STATE_HOLYFIRE, STATE_THORNS, STATE_DEFIANCE, STATE_RESISTCOLD, STATE_BLESSEDAIM, STATE_STAMINA, STATE_RESISTLIGHT, STATE_CONCENTRATION, STATE_HOLYWIND, STATE_CLEANSING, STATE_HOLYSHOCK, STATE_SANCTUARY, STATE_MEDITATION, STATE_FANATICISM, STATE_REDEMPTION, STATE_CONVICTION, STATE_RESISTALL,
	//debuffs
	STATE_AMPLIFYDAMAGE, STATE_WEAKEN, STATE_DECREPIFY, STATE_LOWERRESIST, STATE_POISON, STATE_COLD };

	buffNames = { L"Burst of Speed", L"Fade", L"Cloak of Shadows", L"Venom", L"Shout", L"Battle Orders", L"Battle Command", L"Oak Sage", L"Cyclone Armor", L"Hurricane", L"Bone Armor", L"Holy Shield", L"Frozen Armor", L"Shiver Armor", L"Chilling Armor", L"Enchant", L"Energy Shield", L"Thunder Storm", L"Experience Shrine",
	L"Might", L"Prayer", L"Resist Fire", L"Holy Fire", L"Thorns", L"Defiance", L"Resist Cold", L"Blessed Aim", L"Vigor", L"Resist Lightning", L"Concentration", L"Holy Freeze", L"Cleansing", L"Holy Shock", L"Sanctuary", L"Meditation", L"Fanaticism", L"Redemption", L"Conviction", L"Salvation", 
	L"Amplify Damage", L"Weaken", L"Decrepify", L"Lower Resist", L"Poisoned", L"Frozen"	};
	

	d2VersionText = new Texthook(OutOfGame, 795, 18, D2Version::GetHumanReadableVersion());
	d2VersionText->SetAlignment(Right);
	d2VersionText->SetColor(White);
	d2VersionText->SetFont(1);

	if (BH::cGuardLoaded) {
		Texthook* cGuardText = new Texthook(Perm, 790, 23, "�c4cGuard Loaded");
		cGuardText->SetAlignment(Right);
	}
	gameTimer = GetTickCount();
	nTotalGames = 0;
	szGamesToLevel = "N/A";
	szTimeToLevel = "N/A";
	szLastXpGainPer = "N/A";
	szLastXpGainPer = "N/A";
	szLastGameTime = "N/A";
	automap["GAMESTOLVL"] = szGamesToLevel;
	automap["TIMETOLVL"] = szTimeToLevel;
	automap["LASTXPPERCENT"] = szLastXpGainPer;
	automap["LASTXPPERSEC"] = szLastXpPerSec;
	automap["LASTGAMETIME"] = szLastGameTime;
	automap["SESSIONGAMECOUNT"] = to_string(nTotalGames);
}

void ScreenInfo::LoadConfig() {
	BH::config->ReadToggle("Experience Meter", "VK_NUMPAD7", false, Toggles["Experience Meter"]);

	BH::config->ReadArray("AutomapInfo", automapInfo);
	
	BH::config->ReadToggle("Run Details On Join", "None", false, Toggles["Run Details On Join"]);
	BH::config->ReadToggle("Save Run Details", "None", false, Toggles["Save Run Details"]);
	BH::config->ReadString("Save Run Details Location", szSavePath);

	runDetailsColumns.clear();
	BH::config->ReadMapList("Run Details", runDetailsColumns);

	const string delimiter = ",";
	szColumnHeader = accumulate(runDetailsColumns.begin(), runDetailsColumns.end(), string(),
		[delimiter](const string& s, const pair<const string, const string>& p) {
		return s + (s.empty() ? string() : delimiter) + p.first;
	});
	szColumnData = accumulate(runDetailsColumns.begin(), runDetailsColumns.end(), string(),
		[delimiter](const string& s, const pair<const string, const string>& p) {
		return s + (s.empty() ? string() : delimiter) + p.second;
	});

	BH::config->ReadAssoc("Skill Warning", SkillWarnings);
	SkillWarningMap.clear();
	for (auto it = SkillWarnings.cbegin(); it != SkillWarnings.cend(); it++) {
		if (StringToBool((*it).second)) {
			// If the key is a number, it means warn when that state expires
			DWORD stateId = 0;
			stringstream ss((*it).first);
			if (!(ss >> stateId).fail()) {
				SkillWarningMap[stateId] = GetStateCode(stateId).name;
			}
		}
	}
}

void ScreenInfo::MpqLoaded() {
	mpqVersionText = new Texthook(Perm, 5, 589, MpqVersion);
	mpqVersionText->SetColor(Gold);
}

void ScreenInfo::OnGameJoin() {	
	BnetData* pInfo = (*p_D2LAUNCH_BnData);
	UnitAny* unit = D2CLIENT_GetPlayerUnit();
	if (unit) {
		std::string title = (std::string)"Diablo II - ";
		if (strlen(pInfo->szAccountName) > 0) {
			title += (std::string)pInfo->szAccountName + " - ";
		}
		title += unit->pPlayerData->szName;
		if (!SetWindowText(D2GFX_GetHwnd(), title.c_str())) {
			printf("Failed setting window text, error: %d\n\n", GetLastError());
		}
	}	
	manageConv = false;
	manageBuffs = true;
	//}
	activeBuffs = {};

	if (bFailedToWrite) {
		bFailedToWrite = false;
		string path = ReplaceAutomapTokens(szSavePath);
		for (int i = 0; i < 5; i++) {
			PrintText(Red, "FILE \"%s\" IS LOCKED BY ANOTHER PROCESS! LAST RUN DATA WAS NOT SAVED!", ReplaceAutomapTokens(szSavePath));
		}
	}

	gameTimer = GetTickCount();
	UnitAny* pUnit = D2CLIENT_GetPlayerUnit();
	startExperience = (int)D2COMMON_GetUnitStat(pUnit, STAT_EXP, 0);
	if (currentPlayer.compare(0, 16, pUnit->pPlayerData->szName) != 0) {
		szGamesToLevel = "N/A";
		szTimeToLevel = "N/A";
		szLastXpGainPer = "N/A";
		szLastXpPerSec = "N/A";
		szLastGameTime = "N/A";
	}
	fill_n(aPlayerCountAverage, 8, 0);

	BnetData* pData = (*p_D2LAUNCH_BnData);

	char* szDiff[3] = { "Normal", "Nightmare", "Hell" };
	currentPlayer = string(pUnit->pPlayerData->szName);
	startLevel = (int)D2COMMON_GetUnitStat(pUnit, STAT_LEVEL, 0);
	double startPctExp = ((double)startExperience - ExpByLevel[startLevel - 1]) / (ExpByLevel[startLevel] - ExpByLevel[startLevel - 1]) * 100.0;

	time_t t
		= chrono::system_clock::to_time_t(chrono::system_clock::now());

	string path = ReplaceAutomapTokens(szSavePath);

	automap["JOINDATE"] = FormatTime(t, "%F");
	automap["JOINTIME"] = FormatTime(t, "%T%z");
	automap["CHARLEVEL"] = to_string(startLevel);
	automap["CHARLEVELPERCENT"] = to_string(static_cast<double>(startLevel) + (startPctExp / 100.0));
	automap["CHARXPPERCENT"] = to_string(startPctExp);
	automap["CHARXP"] = to_string(startExperience);
	automap["GAMENAME"] = pData->szGameName;
	string runname = SimpleGameName(pData->szGameName);
	automap["RUNNAME"] = runname;
	if (runcounter.find(runname) == runcounter.end()) {
		runcounter[runname] = 0;
	}
	automap["GAMEPASS"] = pData->szGamePass;
	automap["GAMEDESC"] = pData->szGameDesc;
	automap["GAMEIP"] = pData->szGameIP;
	automap["GAMEDIFF"] = szDiff[D2CLIENT_GetDifficulty()];
	automap["ACCOUNTNAME"] = pData->szAccountName;
	automap["CHARNAME"] = pUnit->pPlayerData->szName;
	automap["SESSIONGAMECOUNT"] = to_string(++nTotalGames);

	/*
	string p = ReplaceAutomapTokens(szSavePath);
	cRunData = new Config(p + ".dat");
	if (!cRunData->Parse()) {
		cRunData->SetConfigName(p + ".dat");
		std::ofstream os;
		os.open(cRunData->GetConfigName(), std::ios_base::out);
		os << endl;
		os.close();
	}
	cRunData->ReadAssoc(pUnit->pPlayerData->szName, runs);
	if (runs.find(runname) == runs.end()) {
		runs[runname] = 0;
		std::ofstream os;
		os.open(cRunData->GetConfigName(), std::ios_base::app);
		os << pUnit->pPlayerData->szName << "[" << runname << "]: 0" << endl;
		os.close();
		cRunData->Parse();
		cRunData->ReadAssoc(pUnit->pPlayerData->szName, runs);
	}
	runs[runname]++;
	*/
	runcounter[runname]++;	

	if (!Toggles["Run Details On Join"].state) {
		return;
	}
	if (runname.length() > 0) {
		//mp
		PrintText(Orange, "%d games played this session.", nTotalGames);
		PrintText(Orange, "%d \"%s\" games played this session.", runcounter[runname], runname.c_str());
		//PrintText(Orange, "%d \"%s\" games played total.", runs[runname], runname.c_str());
	} else {
		//sp
		PrintText(Orange, "%d games played this session.", nTotalGames);
		PrintText(Orange, "%d single player games played this session.", runcounter[runname]);
		//PrintText(Orange, "%d single player games played on this character.", runs[runname], runname.c_str());
	}
}

int	ScreenInfo::GetPlayerCount() {
	int i = 0;
	for (RosterUnit* pRoster = *p_D2CLIENT_PlayerUnitList; pRoster; pRoster = pRoster->pNext)
		++i;
	return i;
}

string ScreenInfo::SimpleGameName(const string& gameName) {
	std::smatch match;
	if (regex_search(gameName, match, regex("^(.*?)(\\d+)$")) && match.size() == 3) {
		return match.format("$1");
	}
	return gameName;
}

string ScreenInfo::FormatTime(time_t t, const char* format) {
	stringstream ss;
	ss << put_time(std::localtime(&t), format);
	return ss.str();
}

void ScreenInfo::OnKey(bool up, BYTE key, LPARAM lParam, bool* block) {
	for (map<string,Toggle>::iterator it = Toggles.begin(); it != Toggles.end(); it++) {
		if (key == (*it).second.toggle) {
			*block = true;
			if (up) {
				(*it).second.state = !(*it).second.state;
			}
			return;
		}
	}
	return;
}


// Right-clicking in the chat console pastes from the clipboard
void ScreenInfo::OnRightClick(bool up, int x, int y, bool* block) {
	if (up)
		return;	

	int left = 130, top = 500, width = 540, height = 42;
	if (D2CLIENT_GetUIState(0x05) && x >= left && x <= (left + width) && y >= top && y <= (top + height)) {
		*block = true;

		if (IsClipboardFormatAvailable(CF_TEXT)) {
			OpenClipboard(NULL);
			HGLOBAL glob = GetClipboardData(CF_TEXT);
			size_t size = GlobalSize(glob);
			char* cbtext = (char *)glob;

			std::vector<INPUT> events;
			char buffer[120] = {0};
			GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILANGUAGE, buffer, sizeof(buffer));
			HKL hKeyboardLayout = LoadKeyboardLayout(buffer, KLF_ACTIVATE);

			for (unsigned int i = 0; i < size-1; i++) {
				INPUT keyEvent = {0};
				const SHORT Vk = VkKeyScanEx(cbtext[i], hKeyboardLayout);
				const UINT VKey = MapVirtualKey(LOBYTE(Vk), 0);

				if (HIBYTE(Vk) == 1) {  // shift key must be pressed
					ZeroMemory(&keyEvent, sizeof(keyEvent));
					keyEvent.type = INPUT_KEYBOARD;
					keyEvent.ki.dwFlags = KEYEVENTF_SCANCODE;
					keyEvent.ki.wScan = MapVirtualKey(VK_LSHIFT, 0);
					events.push_back(keyEvent);
				}

				ZeroMemory(&keyEvent, sizeof(keyEvent));
				keyEvent.type = INPUT_KEYBOARD;
				keyEvent.ki.dwFlags = KEYEVENTF_SCANCODE;
				keyEvent.ki.wScan = VKey;
				events.push_back(keyEvent);

				ZeroMemory(&keyEvent, sizeof(keyEvent));
				keyEvent.type = INPUT_KEYBOARD;
				keyEvent.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
				keyEvent.ki.wScan = VKey;
				events.push_back(keyEvent);

				if (HIBYTE(Vk) == 1) {  // release shift key
					ZeroMemory(&keyEvent, sizeof(keyEvent));
					keyEvent.type = INPUT_KEYBOARD;
					keyEvent.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
					keyEvent.ki.wScan = MapVirtualKey(VK_LSHIFT, 0);
					events.push_back(keyEvent);
				}
			}
			CloseClipboard();

			if(hKeyboardLayout) {
				UnloadKeyboardLayout(hKeyboardLayout);
			}
			int retval = SendInput(events.size(), &events[0], sizeof(INPUT));
		}
	}
}

void ScreenInfo::OnDraw() {
	int yOffset = 1;
	BnetData* pData = (*p_D2LAUNCH_BnData);
	void *quests = D2CLIENT_GetQuestInfo();
	if (!pData || !quests) {
		return;
	}
	if (mpqH == NULL) {
		mpqH = D2WIN_LoadMpq(5000, "BH.dll", "buffs.mpq", "buffs", 0, 0);
	}
	if (!cellLoaded) {
		cf = D2WIN_LoadCellFile("data\\global\\ui\\spells\\buffs24", 0);
		cellLoaded = true;
	}

	ULONGLONG ticks = BHGetTickCount();
	ULONGLONG ms = ticks - packetTicks;
	if (!ReceivedQuestPacket && packetRequests < 6 && ms > 5000) {
		// Ask for quest information from the server; server will respond with packet 0x52.
		// (In case we inject mid-game and miss the packets sent upon game creation/joining)
		BYTE RequestQuestData[1] = {0x40};
		D2NET_SendPacket(1, 0, RequestQuestData);
		packetTicks = ticks;
		packetRequests++;
	}

	while (!CurrentWarnings.empty()) {
		StateWarning *curr = CurrentWarnings.front();
		if (ticks - curr->startTicks > 5000) {
			CurrentWarnings.pop_front();
			delete curr;
		} else {
			break;
		}
	}

	for (std::deque<StateWarning*>::iterator it = CurrentWarnings.begin(); it != CurrentWarnings.end(); ++it) {
		unsigned int posX = *p_D2CLIENT_ScreenSizeX / 2;
		unsigned int posY = *p_D2CLIENT_ScreenSizeY / 2 + 34 * yOffset++;
		Texthook::Draw(posX, posY, Center, 3, Red, "%s has expired!", (*it)->name.c_str());
	}

	// It's a kludge to peek into other modules for config info, but it just seems silly to
	// create a new UI tab for each module with config parameters.
	if ((*BH::MiscToggles)["Quest Drop Warning"].state) {
		char *bossNames[3] = {"Mephisto", "Diablo", "Baal"};
		int xpac = pData->nCharFlags & PLAYER_TYPE_EXPANSION;
		int doneDuriel = D2COMMON_GetQuestFlag2(quests, THE_SEVEN_TOMBS, QFLAG_REWARD_GRANTED);
		int doneMephisto = D2COMMON_GetQuestFlag2(quests, THE_GUARDIAN, QFLAG_REWARD_GRANTED);
		int doneDiablo = D2COMMON_GetQuestFlag2(quests, TERRORS_END, QFLAG_REWARD_GRANTED);
		int doneBaal = D2COMMON_GetQuestFlag2(quests, EVE_OF_DESTRUCTION, QFLAG_REWARD_GRANTED);
		int startedMephisto = D2COMMON_GetQuestFlag2(quests, THE_GUARDIAN, QFLAG_QUEST_STARTED);
		int startedDiablo = D2COMMON_GetQuestFlag2(quests, TERRORS_END, QFLAG_QUEST_STARTED);
		int startedBaal = D2COMMON_GetQuestFlag2(quests, EVE_OF_DESTRUCTION, QFLAG_QUEST_STARTED);

		int warning = -1;
		if (doneDuriel && startedMephisto && !doneMephisto && !MephistoBlocked) {
			warning = 0;
		} else if (doneMephisto && startedDiablo && !doneDiablo && !DiabloBlocked) {
			warning = 1;
		} else if (xpac && doneDiablo && startedBaal && !doneBaal && !BaalBlocked) {
			warning = 2;
		}
		if (warning >= 0) {
			ms = ticks - warningTicks;
			if (ms > 2000) {
				warningTicks = ticks;
			} else if (ms > 500) {
				Texthook::Draw(*p_D2CLIENT_ScreenSizeX / 2, 30,
					Center, 3, Red, "%s Quest Active", bossNames[warning]);
			}
		}
	}	

	UnitAny* pUnit = D2CLIENT_GetPlayerUnit();
	currentExperience = (int)D2COMMON_GetUnitStat(pUnit, STAT_EXP, 0);
	currentLevel = (int)D2COMMON_GetUnitStat(pUnit, STAT_LEVEL, 0);

	endTimer = ((GetTickCount() - gameTimer) / 1000);
	if (startLevel == 0) { startLevel = currentLevel; }

	char sExp[255] = { 0 };
	double oldPctExp = ((double)startExperience - ExpByLevel[startLevel - 1]) / (ExpByLevel[startLevel] - ExpByLevel[startLevel - 1]) * 100.0;
	double pExp = ((double)currentExperience - ExpByLevel[currentLevel - 1]) / (ExpByLevel[currentLevel] - ExpByLevel[currentLevel - 1]) * 100.0;
	
	currentExpGainPct = pExp - oldPctExp;
	if (currentLevel > startLevel) {
		currentExpGainPct = (100 - oldPctExp) + pExp + ((currentLevel - startLevel) - 1) * 100;
	}
	currentExpPerSecond = endTimer > 0 ? (currentExperience - startExperience) / (double)endTimer : 0;

	double remainingExpToLevel = ExpByLevel[currentLevel] - currentExperience;

	double timeToLevelInSeconds = currentExpPerSecond > 0 ? remainingExpToLevel / currentExpPerSecond : 0;

	int hours = static_cast<int>(timeToLevelInSeconds / 3600);
	int minutes = static_cast<int>((timeToLevelInSeconds / 60)) % 60;
	int seconds = static_cast<int>(timeToLevelInSeconds) % 60;

	char timeToLevelFormatted[64];
	sprintf_s(timeToLevelFormatted, sizeof(timeToLevelFormatted), "%02d:%02d:%02d", hours, minutes, seconds);

	char xpPerSec[32];
	FormattedXPPerSec(xpPerSec, currentExpPerSecond);

	if (Toggles["Experience Meter"].state) {
		sprintf_s(sExp, "%00.2f%% (%s%00.2f%%) | Time to level: %s",
			pExp, currentExpGainPct >= 0 ? "+" : "", currentExpGainPct, timeToLevelFormatted);
		Texthook::Draw((*p_D2CLIENT_ScreenSizeX / 2) - 150, *p_D2CLIENT_ScreenSizeY - 60, Center, 6, White, "%s", sExp);
	}

	char gameTime[20];
	sprintf_s(gameTime, 20, "%.2d:%.2d:%.2d", endTimer / 3600, (endTimer / 60) % 60, endTimer % 60);

	time_t tTime;
	time(&tTime);
	CHAR szTime[128] = "";
	struct tm time;
	localtime_s(&time, &tTime);
	strftime(szTime, sizeof(szTime), "%I:%M:%S %p", &time);

	if (cf) {
		//dc6 is loaded!		
		if (manageBuffs) {
			//received packet 0xA8 or 0xA9. Change on one of players states.
			for (unsigned int i = 0; i < buffs.size(); i++) {
				int state = D2COMMON_GetUnitState(pUnit, buffs[i]);
				BOOL buffFound = false;
				int pos = 0;				
				for (unsigned j = 0; j < activeBuffs.size(); j++) {
					if (activeBuffs[j].state == buffs[i]) {
						buffFound = true;
						pos = j;
						break;
					}
				}			
				if (state != 0 && !buffFound) {
					//add buff to activeBuffs
					Buff newBuff = {};
					newBuff.state = buffs[i];
					newBuff.index = i;
					if (manageConv && buffs[i] == STATE_CONVICTION) {
						newBuff.isBuff = (int)D2COMMON_GetUnitStat(pUnit, STAT_FIRERESIST, 0) < resTracker ? false : true;
						manageConv = false;
					}
					else {
						newBuff.isBuff = i < 39 ? true : false;
					}					
					activeBuffs.push_back(newBuff);
				} else if (state == 0 && buffFound) {
					//remove buff from activeBuffs
					activeBuffs.erase(activeBuffs.begin() + pos);
				}			
			}
			manageBuffs = false;			
		}
		DWORD mouseX = *p_D2CLIENT_MouseX;
		DWORD mouseY = *p_D2CLIENT_MouseY;
		int screenX = *p_D2CLIENT_ScreenSizeX;
		int screenY = *p_D2CLIENT_ScreenSizeY;
		int buffX = 117;
		int debuffX = screenX - 117 - cf->cells[0]->width;
		int buffY = screenY - 49;
		int debuffY = screenY - 49;
		int totalBuffs = 0;	

		for (unsigned i = 0; i < activeBuffs.size(); i++) {
			if (totalBuffs % 9 == 0 && totalBuffs != 0) {
				buffX = 117;
				buffY -= cf->cells[0]->height + 1;
			}			
			CellContext buffContext = {};
			buffContext.nCellNo = activeBuffs[i].index;
			buffContext.pCellFile = cf;
			int x = activeBuffs[i].isBuff ? buffX : debuffX;
			int y = activeBuffs[i].isBuff ? buffY : debuffY;
			int col = activeBuffs[i].isBuff ? 3 : 1; //3=Blue, 1=Red;
			D2GFX_DrawCellContextEx(&buffContext, x, y, -1, DRAW_MODE_NORMAL, col);
			if (mouseX > x && mouseX < x + cf->cells[0]->width && mouseY > y - cf->cells[0]->height && mouseY < y) {
				DrawPopup(buffNames[activeBuffs[i].index], x + cf->cells[0]->width / 2, y - cf->cells[0]->height);
			}
			if (activeBuffs[i].isBuff) {
				buffX += cf->cells[0]->width + 1;
				totalBuffs++;
			}
			else {
				debuffX -= cf->cells[0]->width + 1;				
			}			
		}
		resTracker = (int)D2COMMON_GetUnitStat(pUnit, STAT_FIRERESIST, 0);
	}
	
	// The call to GetLevelName somehow invalidates pUnit. This is only observable in O2 builds. The game
	// will crash when you attempt to open the map (which calls OnAutomapDraw function). We need to get the player unit
	// again after calling this function. It may be a good idea in general not to store the return value of
	// GetPlayerUnit.
	char* level = UnicodeToAnsi(D2CLIENT_GetLevelName(pUnit->pPath->pRoom1->pRoom2->pLevel->dwLevelNo));
	pUnit = D2CLIENT_GetPlayerUnit();
	if (!pUnit) return;

	CHAR szPing[10] = "";
	sprintf_s(szPing, sizeof(szPing), "%d", *p_D2CLIENT_Ping);

	DWORD levelId = pUnit->pPath->pRoom1->pRoom2->pLevel->dwLevelNo;
	LevelsTxt* levelTxt = &(*p_D2COMMON_sgptDataTable)->pLevelsTxt[levelId];
	WORD areaLevel = 0;
	if (pData->nCharFlags & PLAYER_TYPE_EXPANSION) {
		areaLevel = levelTxt->wMonLvlEx[D2CLIENT_GetDifficulty()];
	} else {
		areaLevel = levelTxt->wMonLvl[D2CLIENT_GetDifficulty()];
	}

	automap["CURRENTCHARLEVEL"] = to_string(currentLevel);
	automap["CURRENTCHARLEVELPERCENT"] = to_string(static_cast<double>(currentLevel) + (pExp / 100.0));
	automap["CURRENTCHARXPPERCENT"] = to_string(pExp);
	automap["CURRENTCHARXP"] = to_string(currentExperience);
	automap["LEVEL"] = level;
	automap["PING"] = szPing;
	automap["GAMETIME"] = gameTime;
	automap["REALTIME"] = szTime;
	automap["AREALEVEL"] = to_string(areaLevel);
	aPlayerCountAverage[GetPlayerCount() - 1]++;

	delete [] level;	
	
}

void ScreenInfo::OnOOGDraw() {
	if (cellLoaded) {
		D2WIN_UnloadCellFile(cf);
		cellLoaded = false;
	}
}

void ScreenInfo::FormattedXPPerSec(char* buffer, double xpPerSec) {
	char* unit = "";
	if (xpPerSec > 1E9) {
		xpPerSec /= 1E9;
		unit = "B";
	}
	else if (xpPerSec > 1E6) {
		xpPerSec /= 1E6;
		unit = "M";
	}
	else if (xpPerSec > 1E3) {
		xpPerSec /= 1E3;
		unit = "K";
	}
	sprintf(buffer, "%s%.2f%s/s", xpPerSec >= 0 ? "+" : "", xpPerSec, unit);
}

std::string ScreenInfo::ReplaceAutomapTokens(std::string& v) {
	std::string result;
	result.assign(v.c_str());

	for (auto const& am : automap) {
		if (result.find("%" + am.first + "%") == string::npos)
			continue;
		if (am.second.length() == 0)
			result.replace(result.find("%" + am.first + "%"), am.first.length() + 2, "");
		else
			result.replace(result.find("%" + am.first + "%"), am.first.length() + 2, am.second);
	}
	return result;
}

void ScreenInfo::OnAutomapDraw() {		
	int y = 6+(BH::cGuardLoaded?16:0);

	for (vector<string>::iterator it = automapInfo.begin(); it < automapInfo.end(); it++) {
		string key = ReplaceAutomapTokens(*it);
		if (key.length() > 0) {
			Texthook::Draw(*p_D2CLIENT_ScreenSizeX - 10, y, Right,0,Gold,"%s", key.c_str());
			y += 16;
		}
	}	
}

void ScreenInfo::AddDrop(UnitAny* pItem) {
	ScreenInfo::AddDrop(GetItemName(pItem), pItem->pItemPath->dwPosX, pItem->pItemPath->dwPosY);
}

void ScreenInfo::AddDrop(const string& name, unsigned int x, unsigned int y) {
	size_t h = 0;
	hash_combine(h, hash<string>{}(name));
	hash_combine(h, hash<long>{}(x << 8 | y));
	BH::drops[h] = name;
}

void ScreenInfo::OnGamePacketRecv(BYTE* packet, bool* block) {
	UnitAny* pUnit = D2CLIENT_GetPlayerUnit();

	// 0x29 and 0x52 tell us which quests are blocked for us because they were
	// completed by the player who created the game. Packet 29 is sent at game
	// startup and quest events; likewise with packet 52, but also we can trigger
	// packet 52 by sending the server packet 40.

	// 0xA8 and 0xA9 are received when effects (e.g. battle orders) begin or end
	// on players.

	switch (packet[0])
	{
	case 0x29:
		{
			// The packet consists of two-byte pairs for each of the 41 quests,
			// starting at the third byte. The high bit of the first byte in the pair
			// (corresponding to QFLAG_QUEST_COMPLETED_BEFORE) is always set when the
			// quest was previously completed. QFLAG_PRIMARY_GOAL_ACHIEVED is often
			// set as well.
			// Packet received at game start, and upon talking to quest NPCs.
			int packetLen = 97;
			MephistoBlocked = (packet[2 + (THE_GUARDIAN * 2)] & 0x80) > 0;
			DiabloBlocked = (packet[2 + (TERRORS_END * 2)] & 0x80) > 0;
			BaalBlocked = (packet[2 + (EVE_OF_DESTRUCTION * 2)] & 0x80) > 0;
			ReceivedQuestPacket = true;  // fixme: want this here?
			break;

			// TODO: does it get cleared when quest completed while we are in game in different act?
		}
	case 0x52:
		{
			// We have one byte for each of the 41 quests: zero if the quest is blocked,
			// and nonzero if we can complete it.
			// Packet received upon opening quest log, and after sending 0x40 to server.
			int packetLen = 42;
			MephistoBlocked = packet[1 + THE_GUARDIAN] == 0;
			DiabloBlocked = packet[1 + TERRORS_END] == 0;
			BaalBlocked = packet[1 + EVE_OF_DESTRUCTION] == 0;
			ReceivedQuestPacket = true;
			break;
		}
	case 0xA8:
		{
	    	// Packet received when the character begins a new state (i.e. buff/effect received).
			//BYTE unitType = packet[1];
			//DWORD unitId = *(DWORD*)&packet[2];
			//BYTE packetLen = packet[6];
			DWORD state = packet[7];
			//DWORD effects = packet[8];
			//DWORD me = pUnit ? pUnit->dwUnitId : 0;
			if (state == 28) {
				manageConv = true;
			}
			manageBuffs = true;
			break;
		}
	case 0xA9:
		{
			// Packet received when the character ends a state (i.e. buff runs out).
			BYTE unitType = packet[1];
			DWORD unitId = *(DWORD*)&packet[2];
			DWORD state = packet[6];
			DWORD me = pUnit ? pUnit->dwUnitId : 0;
			if (unitType == UNIT_PLAYER && unitId == me) {
				if (SkillWarningMap.find(state) != SkillWarningMap.end()) {
					CurrentWarnings.push_back(new StateWarning(SkillWarningMap[state], BHGetTickCount()));
				}
			}
			manageBuffs = true;
			break;
		}
	default:
		break;
	}
	return;
}

void ScreenInfo::OnGameExit() {
	DWORD xpGained = (currentExperience - startExperience);
	double gamesToLevel = (ExpByLevel[currentLevel] - currentExperience) / (1.0 * xpGained);
	double lastExpGainPct = currentExpGainPct;
	double lastExpPerSecond = currentExpPerSecond;
	int lastGameLength = endTimer;
	int timeToLevel = gamesToLevel * lastGameLength;

	char buffer[128];
	sprintf_s(buffer, sizeof(buffer), "%.2f", gamesToLevel);
	szGamesToLevel = string(buffer);

	sprintf_s(buffer, sizeof(buffer), "%d:%.2d:%.2d", timeToLevel / 3600, (timeToLevel / 60) % 60, timeToLevel % 60);
	szTimeToLevel = string(buffer);

	sprintf_s(buffer, sizeof(buffer), "%s%00.2f%%", lastExpGainPct >= 0 ? "+" : "", lastExpGainPct);
	szLastXpGainPer = string(buffer);

	FormattedXPPerSec(buffer, lastExpPerSecond);
	szLastXpPerSec = string(buffer);

	sprintf_s(buffer, sizeof(buffer), "%.2d:%.2d:%.2d", lastGameLength / 3600, (lastGameLength / 60) % 60, lastGameLength % 60);
	szLastGameTime = string(buffer);

	const string delimiter = ", ";
	string drops = accumulate(BH::drops.begin(), BH::drops.end(), string(),
	[delimiter](const string& s, const pair<const size_t, string>& p) {
		return s + (s.empty() ? string() : delimiter) + p.second;
	});
	BH::drops.clear();

	drops = regex_replace(drops, regex("\xFF" "c."), "");
	drops = regex_replace(drops, regex("\n"), " ");

	automap["GAMESTOLVL"] = szGamesToLevel;
	automap["TIMETOLVL"] = szTimeToLevel;
	automap["LASTXPGAINED"] = to_string(xpGained);
	automap["LASTXPPERCENTGAINED"] = szLastXpGainPer;
	automap["LASTXPPERSEC"] = szLastXpPerSec;
	automap["LASTXPPERSECLONG"] = to_string(lastExpPerSecond);
	automap["LASTGAMETIME"] = szLastGameTime;
	automap["LASTGAMETIMESEC"] = to_string(lastGameLength);
	automap["DROPS"] = regex_replace(drops, regex("\xFF" "c."), "");

	int idx = 0;
	for (int i = 0; i < 8; i++) {
		if (aPlayerCountAverage[i] > aPlayerCountAverage[idx]) {
			idx = i;
		}
	}
	automap["AVGPLAYERCOUNT"] = to_string(idx + 1);

	MephistoBlocked = false;
	DiabloBlocked = false;
	BaalBlocked = false;
	ReceivedQuestPacket = false;

	if (Toggles["Save Run Details"].state) {
		WriteRunTrackerData();
	}	
	/*
	cRunData->Write();
	delete cRunData;
	*/
}

void ScreenInfo::WriteRunTrackerData() {
	namespace fs = std::filesystem;
	fs::path path(ReplaceAutomapTokens(szSavePath));
	bool exist = fs::exists(path);

	fs::create_directories(path.parent_path());

	std::ofstream os;
	os.open(path, std::ios_base::app);
	if (os.fail()) {
		bFailedToWrite = true;
		return;
	}
	if (!exist) {
		os << ReplaceAutomapTokens(szColumnHeader) << endl; 
	}
	os << ReplaceAutomapTokens(szColumnData) << endl;
}

void ScreenInfo::DrawPopup(wchar_t* buffName, int x, int y) {	
	int textWidth = D2WIN_GetTextWidth(buffName);
	D2WIN_DrawRectText(buffName, x - textWidth / 2, y - 2, White, DRAW_MODE_ALPHA_50, White);
}

vector<wstring> ScreenInfo::strBreakApart(wstring str, wchar_t delimiter) {
	wstring temp;
	vector<wstring> parts;
	wstringstream wss(str);
	while (getline(wss, temp, delimiter))
		parts.push_back(temp);

	return parts;
}

// The states players can receive via packets 0xA8/0xA9
StateCode StateCodes[] = {
	{"NONE", 0},
	{"FREEZE", 1},
	{"POISON", 2},
	{"RESISTFIRE", 3},
	{"RESISTCOLD", 4},
	{"RESISTLIGHT", 5},
	{"RESISTMAGIC", 6},
	{"PLAYERBODY", 7},
	{"RESISTALL", 8},
	{"AMPLIFYDAMAGE", 9},
	{"FROZENARMOR", 10},
	{"COLD", 11},
	{"INFERNO", 12},
	{"BLAZE", 13},
	{"BONEARMOR", 14},
	{"CONCENTRATE", 15},
	{"ENCHANT", 16},
	{"INNERSIGHT", 17},
	{"SKILL_MOVE", 18},
	{"WEAKEN", 19},
	{"CHILLINGARMOR", 20},
	{"STUNNED", 21},
	{"SPIDERLAY", 22},
	{"DIMVISION", 23},
	{"SLOWED", 24},
	{"FETISHAURA", 25},
	{"SHOUT", 26},
	{"TAUNT", 27},
	{"CONVICTION", 28},
	{"CONVICTED", 29},
	{"ENERGYSHIELD", 30},
	{"VENOMCLAWS", 31},
	{"BATTLEORDERS", 32},
	{"MIGHT", 33},
	{"PRAYER", 34},
	{"HOLYFIRE", 35},
	{"THORNS", 36},
	{"DEFIANCE", 37},
	{"THUNDERSTORM", 38},
	{"LIGHTNINGBOLT", 39},
	{"BLESSEDAIM", 40},
	{"STAMINA", 41},
	{"CONCENTRATION", 42},
	{"HOLYWIND", 43},
	{"HOLYWINDCOLD", 44},
	{"CLEANSING", 45},
	{"HOLYSHOCK", 46},
	{"SANCTUARY", 47},
	{"MEDITATION", 48},
	{"FANATICISM", 49},
	{"REDEMPTION", 50},
	{"BATTLECOMMAND", 51},
	{"PREVENTHEAL", 52},
	{"CONVERSION", 53},
	{"UNINTERRUPTABLE", 54},
	{"IRONMAIDEN", 55},
	{"TERROR", 56},
	{"ATTRACT", 57},
	{"LIFETAP", 58},
	{"CONFUSE", 59},
	{"DECREPIFY", 60},
	{"LOWERRESIST", 61},
	{"OPENWOUNDS", 62},
	{"DOPPLEZON", 63},
	{"CRITICALSTRIKE", 64},
	{"DODGE", 65},
	{"AVOID", 66},
	{"PENETRATE", 67},
	{"EVADE", 68},
	{"PIERCE", 69},
	{"WARMTH", 70},
	{"FIREMASTERY", 71},
	{"LIGHTNINGMASTERY", 72},
	{"COLDMASTERY", 73},
	{"SWORDMASTERY", 74},
	{"AXEMASTERY", 75},
	{"MACEMASTERY", 76},
	{"POLEARMMASTERY", 77},
	{"THROWINGMASTERY", 78},
	{"SPEARMASTERY", 79},
	{"INCREASEDSTAMINA", 80},
	{"IRONSKIN", 81},
	{"INCREASEDSPEED", 82},
	{"NATURALRESISTANCE", 83},
	{"FINGERMAGECURSE", 84},
	{"NOMANAREGEN", 85},
	{"JUSTHIT", 86},
	{"SLOWMISSILES", 87},
	{"SHIVERARMOR", 88},
	{"BATTLECRY", 89},
	{"BLUE", 90},
	{"RED", 91},
	{"DEATH_DELAY", 92},
	{"VALKYRIE", 93},
	{"FRENZY", 94},
	{"BERSERK", 95},
	{"REVIVE", 96},
	{"ITEMFULLSET", 97},
	{"SOURCEUNIT", 98},
	{"REDEEMED", 99},
	{"HEALTHPOT", 100},
	{"HOLYSHIELD", 101},
	{"JUST_PORTALED", 102},
	{"MONFRENZY", 103},
	{"CORPSE_NODRAW", 104},
	{"ALIGNMENT", 105},
	{"MANAPOT", 106},
	{"SHATTER", 107},
	{"SYNC_WARPED", 108},
	{"CONVERSION_SAVE", 109},
	{"PREGNANT", 110},
	{"111", 111},
	{"RABIES", 112},
	{"DEFENSE_CURSE", 113},
	{"BLOOD_MANA", 114},
	{"BURNING", 115},
	{"DRAGONFLIGHT", 116},
	{"MAUL", 117},
	{"CORPSE_NOSELECT", 118},
	{"SHADOWWARRIOR", 119},
	{"FERALRAGE", 120},
	{"SKILLDELAY", 121},
	{"PROGRESSIVE_DAMAGE", 122},
	{"PROGRESSIVE_STEAL", 123},
	{"PROGRESSIVE_OTHER", 124},
	{"PROGRESSIVE_FIRE", 125},
	{"PROGRESSIVE_COLD", 126},
	{"PROGRESSIVE_LIGHTNING", 127},
	{"SHRINE_ARMOR", 128},
	{"SHRINE_COMBAT", 129},
	{"SHRINE_RESIST_LIGHTNING", 130},
	{"SHRINE_RESIST_FIRE", 131},
	{"SHRINE_RESIST_COLD", 132},
	{"SHRINE_RESIST_POISON", 133},
	{"SHRINE_SKILL", 134},
	{"SHRINE_MANA_REGEN", 135},
	{"SHRINE_STAMINA", 136},
	{"SHRINE_EXPERIENCE", 137},
	{"FENRIS_RAGE", 138},
	{"WOLF", 139},
	{"BEAR", 140},
	{"BLOODLUST", 141},
	{"CHANGECLASS", 142},
	{"ATTACHED", 143},
	{"HURRICANE", 144},
	{"ARMAGEDDON", 145},
	{"INVIS", 146},
	{"BARBS", 147},
	{"WOLVERINE", 148},
	{"OAKSAGE", 149},
	{"VINE_BEAST", 150},
	{"CYCLONEARMOR", 151},
	{"CLAWMASTERY", 152},
	{"CLOAK_OF_SHADOWS", 153},
	{"RECYCLED", 154},
	{"WEAPONBLOCK", 155},
	{"CLOAKED", 156},
	{"QUICKNESS", 157},
	{"BLADESHIELD", 158},
	{"FADE", 159}
};

long long ExpByLevel[] = {
	0,
	500,
	1500,
	3750,
	7875,
	14175,
	22680,
	32886,
	44396,
	57715,
	72144,
	90180,
	112725,
	140906,
	176132,
	220165,
	275207,
	344008,
	430010,
	537513,
	671891,
	839864,
	1049830,
	1312287,
	1640359,
	2050449,
	2563061,
	3203826,
	3902260,
	4663553,
	5493363,
	6397855,
	7383752,
	8458379,
	9629723,
	10906488,
	12298162,
	13815086,
	15468534,
	17270791,
	19235252,
	21376515,
	23710491,
	26254525,
	29027522,
	32050088,
	35344686,
	38935798,
	42850109,
	47116709,
	51767302,
	56836449,
	62361819,
	68384473,
	74949165,
	82104680,
	89904191,
	98405658,
	107672256,
	117772849,
	128782495,
	140783010,
	153863570,
	168121381,
	183662396,
	200602101,
	219066380,
	239192444,
	261129853,
	285041630,
	311105466,
	339515048,
	370481492,
	404234916,
	441026148,
	481128591,
	524840254,
	572485967,
	624419793,
	681027665,
	742730244,
	809986056,
	883294891,
	963201521,
	1050299747,
	1145236814,
	1248718217,
	1361512946,
	1484459201,
	1618470619,
	1764543065,
	1923762030,
	2097310703,
	2286478756,
	2492671933,
	2717422497,
	2962400612,
	3229426756,
	3520485254,
	3837739017,
	9999999999
};

StateCode GetStateCode(unsigned int nKey) {
	for (int n = 0; n < (sizeof(StateCodes) / sizeof(StateCodes[0])); n++) {
		if (nKey == StateCodes[n].value) {
			return StateCodes[n];
		}
	}
	return StateCodes[0];
}
StateCode GetStateCode(const char* name) {
	for (int n = 0; n < (sizeof(StateCodes) / sizeof(StateCodes[0])); n++) {
		if (!_stricmp(name, StateCodes[n].name.c_str())) {
			return StateCodes[n];
		}
	}
	return StateCodes[0];
}