#pragma once
//==========================================================================
// AATF ruleset configuration files
//
// AATF no longer has any tournament rules compiled in: the rules live in a
// .cfg file (see the example rulesets in the project root) and this module
// loads one and runs the checks from its values. Select the file with
// File > Settings... > "AATF Ruleset"; AATF refuses to run without one.
//==========================================================================

#include "editor.h"
#include <string>
#include <vector>

//Convenience aliases. The codebase is TCHAR based; the ruleset code keeps
//  that convention even though the project is built as Unicode.
typedef std::basic_string<TCHAR> RString;

namespace aatf_ruleset
{
	//The 25 player abilities, in the order they are listed in the cfg file
	enum Ability
	{
		AB_OFFENSIVE_AWARENESS, AB_BALL_CONTROL, AB_DRIBBLING, AB_LOW_PASS, AB_LOFTED_PASS,
		AB_FINISHING, AB_PLACE_KICKING, AB_CURL, AB_HEADER, AB_DEFENSIVE_AWARENESS,
		AB_BALL_WINNING, AB_KICKING_POWER, AB_SPEED, AB_ACCELERATION, AB_BALANCE,
		AB_PHYSICAL_CONTACT, AB_JUMP, AB_STAMINA, AB_GK_AWARENESS, AB_CATCHING,
		AB_CLEARING, AB_REFLEXES, AB_GK_REACH, AB_TIGHT_POSSESSION, AB_AGGRESSION,
		ABILITY_COUNT
	};

	//The player classes a squad member can be judged against. A goalkeeper is
	//  counted as a REGULAR but is judged against the GOALKEEPER allowances.
	//  The order is the low-to-high rating order; GOALKEEPER sits apart.
	enum ClassId
	{
		CLASS_REGULAR, CLASS_BRONZE, CLASS_SILVER, CLASS_GOLD, CLASS_GOALKEEPER,
		CLASS_COUNT
	};

	//A height class inside a bracket, e.g. "TALL = 185 : 189 : 5"
	struct HeightClass
	{
		RString name;		//Stored uppercase, printed in the report
		int height;			//Exact height, or upper bound when max-only
		int gkHeight;		//Goalkeeper variant height, 0 if none
		int numPlayers;		//Exact head count for this class
		int cardBonus;		//Extra counting cards allowed
		int comBonus;		//Extra free COM Playing Styles
		int aPosBonus;		//Extra free 'A' positions
		int weakUse;		//Weak Foot Usage override, 0 = none
		int weakAcc;		//Weak Foot Accuracy override, 0 = none

		HeightClass()
		{
			height = 0;
			gkHeight = 0;
			numPlayers = 0;
			cardBonus = 0;
			comBonus = 0;
			aPosBonus = 0;
			weakUse = 0;
			weakAcc = 0;
		}
	};

	//One height bracket. AATF tests brackets in order and uses the first one
	//  whose trigger height is met by any squad player; the last bracket is
	//  the fallback when no trigger is met.
	struct HeightBracket
	{
		RString name;
		bool manletBuff;		//Whether <TYPE>_MANLET_BUFF applies in this bracket
		bool hasTrigger;		//False when the trigger value is left empty
		int trigger;
		std::vector<HeightClass> classes;

		HeightBracket()
		{
			manletBuff = false;
			hasTrigger = false;
			trigger = 0;
		}
	};

	//Allowances for one player class (REGULAR, BRONZE, SILVER, GOLD, GOALKEEPER)
	struct PlayerClass
	{
		int count;				//Exact number of players of this class on the squad
		int form;				//Exact required Form / Condition
		int injuryResistance;	//Maximum allowed Injury Resistance
		int weakUse;			//Maximum allowed Weak Foot Usage
		int weakAcc;			//Maximum allowed Weak Foot Accuracy
		int skills;				//Maximum counting (class 1) cards
		int tricks;				//Free (class 2) cards before they start counting
		int coms;				//Free COM Playing Styles
		int aPos;				//Free 'A' positions
		int baseStat;			//Ability value every ability must equal
		int manletBuff;			//Added to baseStat for manlet-class players
		int haAllowed;			//Whether an HA-height player is allowed
		int haNerf;				//Subtracted from baseStat for HA players

		PlayerClass()
		{
			count = 0;
			form = 0;
			injuryResistance = 0;
			weakUse = 0;
			weakAcc = 0;
			skills = 0;
			tricks = 0;
			coms = 0;
			aPos = 0;
			baseStat = 0;
			manletBuff = 0;
			haAllowed = 1;
			haNerf = 0;
		}
	};

	//Everything a ruleset .cfg file can say
	struct Ruleset
	{
		RString leagueType;		//"4CC" or "VGL"; informational only
		int pesVersion;			//Version the card list is written for
		int squadSize;			//Exact number of players on the squad
		int minGK;				//Minimum registered goalkeepers
		bool requireCaptain;	//Whether a captain must be assigned
		bool allowBPosition;	//Whether playable position 'B' is legal
		bool gkAsSecondA;		//Whether GK may be an extra 'A' position
		bool medalsCanBeGK;		//Whether medal players may be goalkeepers

		int minRegPos;			//-1 = derive from PES version
		int maxRegPos;
		int minPlayStyle;		//-1 = derive from PES version
		int maxPlayStyle;

		bool checkFirstPreset;	//Check the player is where they are registered in preset 1
		int maxSkillCards;		//Maximum non-COM skill cards, 0 = no limit

		int heightBrackets;		//0 = skip height checks, -1 = pooled, n = bracket count
		RString manletClass;	//Height class that gets manlet treatment
		int haHeight;			//Height at which a player is "height abuse", 0 = off
		std::vector<RString> maxOnlyClasses;	//Classes matched with height <= value
		int poolTotal;			//Maximum total height (pooled mode)
		int poolMinHeight;		//Shortest legal height (pooled mode)
		int poolMaxHeight;		//Tallest legal height (pooled mode)
		bool poolGKStrict;		//Pin goalkeepers to poolGKHeight
		int poolGKHeight;
		std::vector<HeightBracket> brackets;

		PlayerClass classes[CLASS_COUNT];

		int abilityBonus[ABILITY_COUNT];		//Per-ability bonus/malus
		bool bonusAffects[CLASS_COUNT];			//Which classes the bonuses reach

		bool uniqueGKStats;						//Use the absolute GK table
		int gkStats[ABILITY_COUNT];				//0 = fall back to the GOALKEEPER base stat

		int captainFreeCard;	//Card index free for the captain, -1 = none
		int captainExtraAPos;	//Extra free 'A' positions for the captain
		int captainExtraCom;	//Extra free COM styles for the captain

		int skillCard[41];		//0 = banned, 1 = counts, 2 = free

		Ruleset()
		{
			memset(abilityBonus, 0, sizeof(abilityBonus));
			memset(bonusAffects, 0, sizeof(bonusAffects));
			memset(gkStats, 0, sizeof(gkStats));
			memset(skillCard, 0, sizeof(skillCard));

			leagueType = _T("");
			pesVersion = 0;
			squadSize = 23;
			minGK = 1;
			requireCaptain = true;
			allowBPosition = false;
			gkAsSecondA = false;
			medalsCanBeGK = false;
			minRegPos = -1;
			maxRegPos = -1;
			minPlayStyle = -1;
			maxPlayStyle = -1;
			checkFirstPreset = false;
			maxSkillCards = 0;
			heightBrackets = 0;
			manletClass = _T("MANLET");
			haHeight = 0;
			poolTotal = 0;
			poolMinHeight = 0;
			poolMaxHeight = 0;
			poolGKStrict = false;
			poolGKHeight = 0;
			uniqueGKStats = false;
			captainFreeCard = -1;
			captainExtraAPos = 0;
			captainExtraCom = 0;
		}
	};

	//Load a ruleset from a .cfg file. Returns false only when the file cannot
	//  be read; recoverable problems (unknown keys, malformed values) are
	//  reported through the warnings and the defaults are kept.
	bool load(const TCHAR* pc_path, Ruleset& rs, std::vector<RString>& v_warnings, RString& rs_error);

	//Load the ruleset picked in Settings. Returns false when none is selected
	//  or the file cannot be read. Used by the Make ... buttons and the button
	//  enable/disable logic as well as the AATF check.
	bool load_selected(Ruleset& rs);

	//Auto-Manlet: works out whether the manlet buff applies when the edited
	//  player's height (n_editedHeight, cm) is entered and the team (its
	//  heights in an_teamHeights) would use a bracket with MANLET_BUFF=1.
	//  an_current holds the current ability values. On success returns true,
	//  fills an_filled with the new ability values and sets n_weakUse/n_weakAcc
	//  to the weak foot values to apply (0 = leave that field alone).
	bool auto_manlet_values(const Ruleset& rs, const int* an_teamHeights, int n_teamSize,
		int n_editedHeight, int pesVersion, bool b_isGK, const int* an_current,
		int* an_filled, int& n_weakUse, int& n_weakAcc);

	//Run the ruleset checks against one team and print the report in the AATF
	//  dialog. When no ruleset has been selected, an error is reported instead.
	void check_team(HWND hAatfbox, int pesVersion, int teamSel, player_entry* gplayers, team_entry* gteams, int gnum_players, bool useSuggestions);
}
