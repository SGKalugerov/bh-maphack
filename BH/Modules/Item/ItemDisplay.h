#pragma once
#include "../../Constants.h"
#include "../../D2Ptrs.h"
#include "../../Config.h"
#include "../../MPQInit.h"
#include "../../BH.h"
#include <cstdlib>
#include <regex>
#include "../../RuleLookupCache.h"

#define EXCEPTION_INVALID_STAT			1
#define EXCEPTION_INVALID_OPERATION		2
#define EXCEPTION_INVALID_OPERATOR		3
#define EXCEPTION_INVALID_FLAG			4
#define EXCEPTION_INVALID_ITEM_TYPE		5
#define EXCEPTION_INVALID_GOLD_TYPE		6

#define PLAYER_CLASSIC 0
#define PLAYER_XP 1

#define DEAD_COLOR        0xdead
#define UNDEFINED_COLOR   0xbeef

// Properties that can appear on an item from incoming packets
struct ItemProperty {
	unsigned int stat;
	long value;

	unsigned int minimum;
	unsigned int maximum;
	unsigned int length;

	unsigned int level;
	unsigned int characterClass;
	unsigned int skill;
	unsigned int tab;

	unsigned int monster;

	unsigned int charges;
	unsigned int maximumCharges;

	unsigned int skillChance;

	unsigned int perLevel;
};

// Collection of item data from the internal UnitAny structure
struct UnitItemInfo {
	UnitAny *item;
	char itemCode[4];
	ItemAttributes *attrs;
};

// Item data obtained from an incoming 0x9c packet
struct ItemInfo {
	ItemAttributes *attrs;
	char code[4];
	//std::string packet;
	std::string name;
	std::string earName;
	std::string personalizedName;
	unsigned int id;
	unsigned int x;
	unsigned int y;
	unsigned int amount;
	unsigned int prefix;
	unsigned int suffix;
	unsigned int setCode;
	unsigned int uniqueCode;
	unsigned int runewordId;
	unsigned int defense;
	unsigned int action;
	unsigned int category;
	unsigned int version;
	unsigned int directory;
	unsigned int container;
	unsigned int earLevel;
	unsigned int width;
	unsigned int height;
	unsigned int quality;
	unsigned int graphic;
	unsigned int color;
	unsigned int superiority;
	unsigned int runewordParameter;
	unsigned int maxDurability;
	unsigned int durability;
	BYTE usedSockets;
	BYTE level;
	BYTE earClass;
	BYTE sockets;
	bool equipped;
	bool inSocket;
	bool identified;
	bool switchedIn;
	bool switchedOut;
	bool broken;
	bool potion;
	bool hasSockets;
	bool inStore;
	bool notInSocket;
	bool ear;
	bool startItem;
	bool simpleItem;
	bool ethereal;
	bool personalized;
	bool gambling;
	bool runeword;
	bool ground;
	bool unspecifiedDirectory;
	bool isGold;
	bool hasGraphic;
	bool hasColor;
	bool isArmor;
	bool isWeapon;
	bool indestructible;
	std::vector<unsigned long> prefixes;
	std::vector<unsigned long> suffixes;
	std::vector<ItemProperty> properties;
	bool operator<(ItemInfo const & other) const;
};

ItemAttributes ItemAttributeList[];
StatProperties StatPropertiesList[];
extern std::map<std::string, int> UnknownItemCodes;

enum ConditionType {
	CT_None,
	CT_LeftParen,
	CT_RightParen,
	CT_NegationOperator,
	CT_BinaryOperator,
	CT_Operand
};

class Condition
{
public:
	Condition() {}
	virtual ~Condition() {}

	static const string tokenDelims;
	static void BuildConditions(vector<Condition*> &conditions, string token);
	static void ProcessConditions(vector<Condition*> &rawConditions, vector<Condition*> &processedConditions);
	static void AddOperand(vector<Condition*> &conditions, Condition *cond);
	static void AddNonOperand(vector<Condition*> &conditions, Condition *cond);

	bool Evaluate(UnitItemInfo *uInfo, ItemInfo *info, Condition *arg1, Condition *arg2);

	BYTE conditionType;
private:
	virtual bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2) { return false; }
	virtual bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2) { return false; }
};

class TrueCondition : public Condition
{
public:
	TrueCondition() { conditionType = CT_Operand; };
private:
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class FalseCondition : public Condition
{
public:
	FalseCondition() { conditionType = CT_Operand; };
private:
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class NegationOperator : public Condition
{
public:
	NegationOperator() { conditionType = CT_NegationOperator; };
private:
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class LeftParen : public Condition
{
public:
	LeftParen() { conditionType = CT_LeftParen; };
private:
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class RightParen : public Condition
{
public:
	RightParen() { conditionType = CT_RightParen; };
private:
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class AndOperator : public Condition
{
public:
	AndOperator() { conditionType = CT_BinaryOperator; };
private:
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class OrOperator : public Condition
{
public:
	OrOperator() { conditionType = CT_BinaryOperator; };
private:
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class ItemCodeCondition : public Condition
{
public:
	ItemCodeCondition(const char *code) {
		targetCode[0] = code[0];
		targetCode[1] = code[1];
		targetCode[2] = code[2];
		targetCode[3] = 0;
		conditionType = CT_Operand;
	};
private:
	char targetCode[4];
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class FlagsCondition : public Condition
{
public:
	FlagsCondition(unsigned int flg) : flag(flg) { conditionType = CT_Operand; };
private:
	unsigned int flag;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class PlayerTypeCondition : public Condition
{
public:
	PlayerTypeCondition(unsigned int m) : mode(m) { conditionType = CT_Operand; };
private:
	unsigned int mode;
	bool EvaluateInternal(UnitItemInfo* uInfo, Condition* arg1, Condition* arg2);
	bool EvaluateInternalFromPacket(ItemInfo* info, Condition* arg1, Condition* arg2);
};

class QualityCondition : public Condition
{
public:
	QualityCondition(unsigned int qual) : quality(qual) { conditionType = CT_Operand; };
private:
	unsigned int quality;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class NonMagicalCondition : public Condition
{
public:
	NonMagicalCondition() { conditionType = CT_Operand; };
private:
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class QualityIdCondition : public Condition
{
public:
	QualityIdCondition(unsigned int quality, unsigned int id) : quality(quality), id(id){ conditionType = CT_Operand; };
private:
	unsigned int quality;
	unsigned int id;
	bool EvaluateInternal(UnitItemInfo* uInfo, Condition* arg1, Condition* arg2);
	bool EvaluateInternalFromPacket(ItemInfo* info, Condition* arg1, Condition* arg2);
};

class GemLevelCondition : public Condition
{
public:
	GemLevelCondition(BYTE op, BYTE gem) : gemLevel(gem), operation(op) { conditionType = CT_Operand; };
private:
	BYTE operation;
	BYTE gemLevel;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class GemTypeCondition : public Condition
{
public:
	GemTypeCondition(BYTE op, BYTE gType) : gemType(gType), operation(op) { conditionType = CT_Operand; };
private:
	BYTE operation;
	BYTE gemType;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class RuneCondition : public Condition
{
public:
	RuneCondition(BYTE op, BYTE rune) : runeNumber(rune), operation(op) { conditionType = CT_Operand; };
private:
	BYTE operation;
	BYTE runeNumber;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class GoldCondition : public Condition
{
public:
	GoldCondition(BYTE op, unsigned int amt) : goldAmount(amt), operation(op) { conditionType = CT_Operand; };
private:
	BYTE operation;
	unsigned int goldAmount;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class ItemLevelCondition : public Condition
{
public:
	ItemLevelCondition(BYTE op, BYTE ilvl) : itemLevel(ilvl), operation(op) { conditionType = CT_Operand; };
private:
	BYTE operation;
	BYTE itemLevel;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class QualityLevelCondition : public Condition
{
public:
	QualityLevelCondition(BYTE op, BYTE qlvl) : qualityLevel(qlvl), operation(op) { conditionType = CT_Operand; };
private:
	BYTE operation;
	BYTE qualityLevel;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class AffixLevelCondition : public Condition
{
public:
	AffixLevelCondition(BYTE op, BYTE alvl) : affixLevel(alvl), operation(op) { conditionType = CT_Operand; };
private:
	BYTE operation;
	BYTE affixLevel;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class CraftAffixLevelCondition : public Condition
{
public:
	CraftAffixLevelCondition(BYTE op, BYTE alvl) : affixLevel(alvl), operation(op) { conditionType = CT_Operand; };
private:
	BYTE operation;
	BYTE affixLevel;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class RequiredLevelCondition : public Condition
{
public:
	RequiredLevelCondition(BYTE op, BYTE rlvl) : requiredLevel(rlvl), operation(op) { conditionType = CT_Operand; };
private:
	BYTE operation;
	BYTE requiredLevel;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class ItemGroupCondition : public Condition
{
public:
	ItemGroupCondition(unsigned int group) : itemGroup(group) { conditionType = CT_Operand; };
private:
	unsigned int itemGroup;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class EDCondition : public Condition
{
public:
	EDCondition(BYTE op, unsigned int target) : operation(op), targetED(target) { conditionType = CT_Operand; };
private:
	BYTE operation;
	unsigned int targetED;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
	bool EvaluateED(unsigned int flags);
};

class DurabilityCondition : public Condition
{
public:
	DurabilityCondition(BYTE op, unsigned int target) : operation(op), targetDurability(target) { conditionType = CT_Operand; };
private:
	BYTE operation;
	unsigned int targetDurability;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class ChargedCondition : public Condition
{
public:
	ChargedCondition(BYTE op, unsigned int sk, unsigned int target) : operation(op), skill(sk), targetLevel(target) { conditionType = CT_Operand; };
private:
	BYTE operation;
	unsigned int skill;
	unsigned int targetLevel;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class FoolsCondition : public Condition
{
public:
	FoolsCondition() { conditionType = CT_Operand; };
private:
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class SkillListCondition : public Condition
{
public:
	SkillListCondition(BYTE op, unsigned int t, unsigned int target) : operation(op), type(t), targetStat(target) {
		conditionType = CT_Operand;
		Init();
	};
private:
	BYTE operation;
	unsigned int type;
	unsigned int targetStat;
	std::map<string, string> classSkillList;
	std::map<string, string> skillList;
	vector<unsigned int> goodClassSkills;
	vector<unsigned int> goodTabSkills;
	void Init();
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class CharStatCondition : public Condition
{
public:
	CharStatCondition(unsigned int stat, unsigned int stat2, BYTE op, unsigned int target)
		: stat1(stat), stat2(stat2), operation(op), targetStat(target) { conditionType = CT_Operand; };
private:
	unsigned int stat1;
	unsigned int stat2;
	BYTE operation;
	unsigned int targetStat;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class DifficultyCondition : public Condition
{
public:
	DifficultyCondition(BYTE op, unsigned int target)
		: operation(op), targetDiff(target) { conditionType = CT_Operand; };
private:
	BYTE operation;
	unsigned int targetDiff;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class FilterLevelCondition : public Condition
{
public:
	FilterLevelCondition(BYTE op, unsigned int target)
		: operation(op), filterLevel(target) { conditionType = CT_Operand; };
private:
	BYTE operation;
	unsigned int filterLevel;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class ItemStatCondition : public Condition
{
public:
	ItemStatCondition(unsigned int stat, unsigned int stat2, BYTE op, unsigned int target)
		: itemStat(stat), itemStat2(stat2), operation(op), targetStat(target) { conditionType = CT_Operand; };
private:
	unsigned int itemStat;
	unsigned int itemStat2;
	BYTE operation;
	unsigned int targetStat;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class PartialCondition : public Condition
{
public:
	PartialCondition(BYTE op, int target_count, vector<string> tokens)
		: operation(op), target_count(target_count) {
		for (auto token : tokens) {
			make_count_subrule(token);	
		}
		conditionType = CT_Operand;
	};
	
	// make_count_subrule calls BuildConditon, which creates new Conditions. We free these here.
	~PartialCondition();
private:
	BYTE operation;
	const int target_count;
	vector<Rule> rules; // TODO: should be const, but Rule::Evalate needs to be modified
	void make_count_subrule(string token);
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class ItemPriceCondition : public Condition
{
public:
	ItemPriceCondition(BYTE op, unsigned int target)
		: operation(op), targetStat(target) {
		conditionType = CT_Operand;
	};
private:
	BYTE operation;
	unsigned int targetStat;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class ResistAllCondition : public Condition
{
public:
	ResistAllCondition(BYTE op, unsigned int target) : operation(op), targetStat(target) { conditionType = CT_Operand; };
private:
	BYTE operation;
	unsigned int targetStat;
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

class AddCondition : public Condition
{
public:
	AddCondition(string& k, BYTE op, unsigned int target) : key(k), operation(op), targetStat(target) { 
		conditionType = CT_Operand;
		Init();
	};
private:
	BYTE operation;
	vector<string> codes;
	vector<DWORD> stats;
	unsigned int targetStat;
	string key;
	void Init();
	bool EvaluateInternal(UnitItemInfo *uInfo, Condition *arg1, Condition *arg2);
	bool EvaluateInternalFromPacket(ItemInfo *info, Condition *arg1, Condition *arg2);
};

extern TrueCondition *trueCondition;
extern FalseCondition *falseCondition;

struct ActionReplace {
	string key;
	string value;
};

struct ColorReplace {
	string key;
	int value;
};

struct SkillReplace {
	string key;
	int value;
};

struct Action {
	bool stopProcessing;
	string name;
	string description;
	int colorOnMap;
	int borderColor;
	int dotColor;
	int pxColor;
	int lineColor;
	int notifyColor;
	bool noTracking;
	unsigned int pingLevel;
	Action() :
		colorOnMap(UNDEFINED_COLOR),
		borderColor(UNDEFINED_COLOR),
		dotColor(UNDEFINED_COLOR),
		pxColor(UNDEFINED_COLOR),
		lineColor(UNDEFINED_COLOR),
		notifyColor(UNDEFINED_COLOR),
		pingLevel(0),
		stopProcessing(true),
		noTracking(false),
		name(""),
		description("") {}
};

struct Rule {
	vector<Condition*> conditions;
	Action action;
	vector<Condition*> conditionStack;

	Rule(vector<Condition*> &inputConditions, string *str);

	// TODO: Should this really be defined in the header? This will force it to be inlined AFAIK. -ybd
	// Evaluate conditions which are in Reverse Polish Notation
	bool Evaluate(UnitItemInfo *uInfo, ItemInfo *info) {
		if (conditions.size() == 0) {
			return true;  // a rule with no conditions always matches
		}

		bool retval;
		try {
			// conditionStack was previously declared on the stack within this function. This caused
			// excessive allocaiton calls and was limiting the speed of the item display (causing
			// frame rate drops with complex item displays while ALT was held down). Moving this vector
			// to the object level, preallocating size, and using the resize(0) method to clear avoids
			// excessive reallocation.
			conditionStack.resize(0); 
			for (unsigned int i = 0; i < conditions.size(); i++) {
				Condition *input = conditions[i];
				if (input->conditionType == CT_Operand) {
					conditionStack.push_back(input);
				} else if (input->conditionType == CT_BinaryOperator || input->conditionType == CT_NegationOperator) {
					Condition *arg1 = NULL, *arg2 = NULL;
					if (conditionStack.size() < 1) {
						return false;
					}
					arg1 = conditionStack.back();
					conditionStack.pop_back();
					if (input->conditionType == CT_BinaryOperator) {
						if (conditionStack.size() < 1) {
							return false;
						}
						arg2 = conditionStack.back();
						conditionStack.pop_back();
					}
					if (input->Evaluate(uInfo, info, arg1, arg2)) {
						conditionStack.push_back(trueCondition);
					} else {
						conditionStack.push_back(falseCondition);
					}
				}
			}
			if (conditionStack.size() == 1) {
				retval = conditionStack[0]->Evaluate(uInfo, info, NULL, NULL);
			} else {
				retval = false;
			}
		} catch (...) {
			retval = false;
		}
		return retval;
	}
};

class ItemDescLookupCache : public RuleLookupCache<string> {
	string make_cached_T(UnitItemInfo *uInfo) override;
	string to_str(const string &name) override;

		public:
		ItemDescLookupCache(const std::vector<Rule*> &RuleList) :
			RuleLookupCache<string>(RuleList) {}
};

class ItemNameLookupCache : public RuleLookupCache<string, const string &> {
	string make_cached_T(UnitItemInfo *uInfo, const string &name) override;
	string to_str(const string &name) override;

		public:
		ItemNameLookupCache(const std::vector<Rule*> &RuleList) :
			RuleLookupCache<string, const string&>(RuleList) {}
};

class MapActionLookupCache : public RuleLookupCache<vector<Action>> {
	vector<Action> make_cached_T(UnitItemInfo *uInfo) override;
	string to_str(const vector<Action> &actions);

		public:
		MapActionLookupCache(const std::vector<Rule*> &RuleList) :
			RuleLookupCache<vector<Action>>(RuleList) {}
};

class IgnoreLookupCache : public RuleLookupCache<bool> {
	bool make_cached_T(UnitItemInfo *uInfo) override;
	string to_str(const bool &ignore);

		public:
		IgnoreLookupCache(const std::vector<Rule*> &RuleList) :
			RuleLookupCache<bool>(RuleList) {}
};

extern vector<Rule*> RuleList;
extern vector<Rule*> NameRuleList;
extern vector<Rule*> DescRuleList;
extern vector<Rule*> MapRuleList;
extern vector<Rule*> DoNotBlockRuleList;
extern vector<Rule*> IgnoreRuleList;
extern vector<pair<string, string>> rules;
extern ItemDescLookupCache item_desc_cache;
extern ItemNameLookupCache item_name_cache;
extern MapActionLookupCache map_action_cache;
extern IgnoreLookupCache do_not_block_cache;
extern IgnoreLookupCache ignore_cache;
extern map<string, string> condition_group;

namespace ItemDisplay {
	void InitializeItemRules();
	void UninitializeItemRules();
	bool UntestedSettingsUsed();
}
StatProperties *GetStatProperties(unsigned int stat);
void BuildAction(string *str, Action *act);
string ParseDescription(Action *act);
int ParsePingLevel(Action *act, const string& reg_string);
int ParseMapColor(Action *act, const string& reg_string);
void HandleUnknownItemCode(char *code, char *tag);
BYTE GetOperation(string *op);
inline bool IntegerCompare(unsigned int Lvalue, int operation, unsigned int Rvalue);
void GetItemName(UnitItemInfo *uInfo, string &name);
void SubstituteNameVariables(UnitItemInfo *uInfo, string &name, const string &action_name);
int GetDefense(ItemInfo *item);
BYTE GetAffixLevel(BYTE ilvl, BYTE qlvl, BYTE mlvl);
BYTE GetRequiredLevel(UnitAny* item);
BYTE RuneNumberFromItemCode(char *code);
