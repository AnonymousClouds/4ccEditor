//==========================================================================
// AATF ruleset configuration files: loader and generic ruleset checker
//
// The checks below reproduce what the old hardcoded 4CC/VGL routines did,
// but every value comes from the selected .cfg file. See ruleset.h for the
// data model and the example rulesets for the file syntax.
//==========================================================================

#include "editor.h"
#include "resource.h"
#include "window.h"
#include "ruleset.h"

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <algorithm>

namespace aatf_ruleset
{
	//----------------------------------------------------------------------
	//Static data

	//The class keys as they appear in the cfg file, indexed by ClassId
	static const TCHAR* const g_class_names[CLASS_COUNT] =
	{
		_T("REGULAR"), _T("BRONZE"), _T("SILVER"), _T("GOLD"), _T("GOALKEEPER")
	};

	//Ability names as they appear in AATF_BONUS_xx and AATF_GK_xx keys
	static const TCHAR* const g_ability_key_names[ABILITY_COUNT] =
	{
		_T("OFFENSIVE_AWARENESS"), _T("BALL_CONTROL"), _T("DRIBBLING"), _T("LOW_PASS"), _T("LOFTED_PASS"),
		_T("FINISHING"), _T("PLACE_KICKING"), _T("CURL"), _T("HEADER"), _T("DEFENSIVE_AWARENESS"),
		_T("BALL_WINNING"), _T("KICKING_POWER"), _T("SPEED"), _T("ACCELERATION"), _T("BALANCE"),
		_T("PHYSICAL_CONTACT"), _T("JUMP"), _T("STAMINA"), _T("GK_AWARENESS"), _T("CATCHING"),
		_T("CLEARING"), _T("REFLEXES"), _T("GK_REACH"), _T("TIGHT_POSSESSION"), _T("AGGRESSION")
	};

	//Ability names as they are printed in the AATF report
	static const TCHAR* const g_ability_names[ABILITY_COUNT] =
	{
		_T("Attacking Prowess"), _T("Ball Control"), _T("Dribbling"), _T("Low Pass"), _T("Lofted Pass"),
		_T("Finishing"), _T("Place Kicking"), _T("Curl"), _T("Header"), _T("Defensive Prowess"),
		_T("Ball Winning"), _T("Kicking Power"), _T("Speed"), _T("Explosive Power"), _T("Balance"),
		_T("Physical Contact"), _T("Jump"), _T("Stamina"), _T("Goalkeeping"), _T("Catching"),
		_T("Clearing"), _T("Reflexes"), _T("Coverage"), _T("Tight Possession"), _T("Aggression")
	};

	//Player skill names, used when reporting a banned card
	static const TCHAR* const g_skill_card_names[41] =
	{
		_T("Scissors Feint"), _T("Flip Flap"), _T("Marseille Turn"), _T("Sombrero"), _T("Cut Behind & Turn"),
		_T("Scotch Move"), _T("Heading"), _T("Long Range Drive"), _T("Knuckle Shot"), _T("Acro Finishing"),
		_T("Heel Trick"), _T("First Time Shot"), _T("One Touch Pass"), _T("Weighted Pass"), _T("Pinpoint Crossing"),
		_T("Outside Curler"), _T("Rabona"), _T("Low Lofted Pass"), _T("Low Punt Trajectory"), _T("Long Throw"),
		_T("GK Long Throw"), _T("Malicia"), _T("Man Marking"), _T("Track Back"), _T("Acro Clear"),
		_T("Captaincy"), _T("Super Sub"), _T("Fighting Spirit"), _T("Double Touch"), _T("Crossover Turn"),
		_T("Step on Skill"), _T("Chip Shot"), _T("Dipping Shots"), _T("Rising Shots"), _T("No Look Pass"),
		_T("GK High Punt Trajectory"), _T("Penalty Specialist"), _T("GK Penalty Specialist"), _T("Interception"),
		_T("Long Range Shooting"), _T("Through Passing")
	};

	//The registered position in player_export's reg_pos field maps to this
	//  index in the play_pos array (same table the editor uses elsewhere).
	static const int g_regPosToPlayPos[13] = { 12, 9, 10, 11, 5, 6, 7, 8, 4, 2, 3, 1, 0 };

	//----------------------------------------------------------------------
	//Small string helpers

	static void trim(RString& rs)
	{
		size_t first = rs.find_first_not_of(_T(" \t\r\n"));
		if(first == RString::npos)
		{
			rs.clear();
			return;
		}
		size_t last = rs.find_last_not_of(_T(" \t\r\n"));
		rs = rs.substr(first, last - first + 1);
	}

	static RString to_upper(const RString& rs)
	{
		RString out = rs;
		for(size_t ii = 0; ii < out.size(); ii++)
			out[ii] = (TCHAR)_totupper(out[ii]);
		return out;
	}

	static bool ends_with(const RString& rs, const TCHAR* pc_suffix)
	{
		size_t len = _tcslen(pc_suffix);
		return rs.size() >= len && _tcsicmp(rs.c_str() + rs.size() - len, pc_suffix) == 0;
	}

	static std::vector<RString> split(const RString& rs, TCHAR c_sep)
	{
		std::vector<RString> out;
		size_t start = 0;
		for(;;)
		{
			size_t pos = rs.find(c_sep, start);
			RString part = (pos == RString::npos) ? rs.substr(start) : rs.substr(start, pos - start);
			trim(part);
			out.push_back(part);
			if(pos == RString::npos) break;
			start = pos + 1;
		}
		return out;
	}

	static bool parse_int(const RString& rs, int& n_out)
	{
		if(rs.empty()) return false;
		const TCHAR* pc_start = rs.c_str();
		TCHAR* pc_end = NULL;
		long value = _tcstol(pc_start, &pc_end, 10);
		if(pc_end == pc_start) return false;
		while(*pc_end && _istspace(*pc_end)) pc_end++;
		if(*pc_end) return false;
		n_out = (int)value;
		return true;
	}

	static RString line_prefix(int lineNo)
	{
		TCHAR cs_buf[32];
		_stprintf_s(cs_buf, 32, _T("Line %d: "), lineNo);
		return RString(cs_buf);
	}

	//Finds an ability by its cfg key name; -1 when the name is unknown
	static int ability_from_key_name(const RString& name)
	{
		for(int ii = 0; ii < ABILITY_COUNT; ii++)
			if(_tcsicmp(name.c_str(), g_ability_key_names[ii]) == 0)
				return ii;
		return -1;
	}

	//Maps "GK" or a class name to a ClassId; -1 when the name is unknown
	static int class_from_key_suffix(const RString& name)
	{
		if(_tcsicmp(name.c_str(), _T("GK")) == 0) return CLASS_GOALKEEPER;
		for(int ii = 0; ii < CLASS_COUNT; ii++)
			if(_tcsicmp(name.c_str(), g_class_names[ii]) == 0)
				return ii;
		return -1;
	}

	//Finds a height class in a bracket, adding an empty one when it is the
	//  first key for that class
	static HeightClass* find_or_add_class(HeightBracket& bracket, const RString& name)
	{
		for(size_t ii = 0; ii < bracket.classes.size(); ii++)
			if(bracket.classes[ii].name == name)
				return &bracket.classes[ii];
		bracket.classes.push_back(HeightClass());
		bracket.classes.back().name = name;
		return &bracket.classes.back();
	}

	//Parses a "<height> : <gkHeight> : <numPlayers>" entry
	static bool parse_height_entry(const RString& rs, int& h, int& gk, int& num)
	{
		std::vector<RString> parts = split(rs, _T(':'));
		if(parts.size() != 3) return false;
		return parse_int(parts[0], h) && parse_int(parts[1], gk) && parse_int(parts[2], num);
	}

	//----------------------------------------------------------------------
	//Ruleset loader

	bool load(const TCHAR* pc_path, Ruleset& rs, std::vector<RString>& v_warnings, RString& rs_error)
	{
		FILE* pFile = _tfopen(pc_path, _T("r"));
		if(!pFile)
		{
			rs_error = _T("ERROR: Could not open ruleset file '");
			rs_error += pc_path;
			rs_error += _T("'.");
			return false;
		}

		TCHAR cs_line[1024];
		int lineNo = 0;
		while(_fgetts(cs_line, 1024, pFile))
		{
			lineNo++;
			RString line(cs_line);
			if(lineNo == 1 && !line.empty() && line[0] == 0xFEFF)
				line.erase(0, 1);

			//Strip the comment, then the whitespace
			size_t cut = line.find_first_of(_T(";#"));
			if(cut != RString::npos) line = line.substr(0, cut);
			trim(line);
			if(line.empty()) continue;

			size_t eq = line.find(_T('='));
			if(eq == RString::npos)
			{
				v_warnings.push_back(line_prefix(lineNo) + _T("line is not KEY=VALUE, ignored"));
				continue;
			}

			RString key = to_upper(line.substr(0, eq));
			trim(key);
			RString value = line.substr(eq + 1);
			trim(value);
			if(key.empty())
			{
				v_warnings.push_back(line_prefix(lineNo) + _T("line has an empty key, ignored"));
				continue;
			}

			RString lp = line_prefix(lineNo);
			bool handled = true;

			//--------------------------------------------------------------
			//Simple settings
			if(key == _T("LEAGUE_TYPE"))
			{
				rs.leagueType = to_upper(value);
			}
			else if(key == _T("PES_VERSION"))
			{
				if(!parse_int(value, rs.pesVersion)) v_warnings.push_back(lp + _T("value for PES_VERSION is not a number, ignored"));
			}
			else if(key == _T("AATF_SQUAD_SIZE"))
			{
				if(!parse_int(value, rs.squadSize)) v_warnings.push_back(lp + _T("value for AATF_SQUAD_SIZE is not a number, ignored"));
			}
			else if(key == _T("AATF_MIN_GK"))
			{
				if(!parse_int(value, rs.minGK)) v_warnings.push_back(lp + _T("value for AATF_MIN_GK is not a number, ignored"));
			}
			else if(key == _T("AATF_REQUIRE_CAPTAIN"))
			{
				int v; if(parse_int(value, v)) rs.requireCaptain = (v != 0);
				else v_warnings.push_back(lp + _T("value for AATF_REQUIRE_CAPTAIN is not a number, ignored"));
			}
			else if(key == _T("AATF_ALLOW_B_POSITION"))
			{
				int v; if(parse_int(value, v)) rs.allowBPosition = (v != 0);
				else v_warnings.push_back(lp + _T("value for AATF_ALLOW_B_POSITION is not a number, ignored"));
			}
			else if(key == _T("AATF_GK_AS_SECOND_A"))
			{
				int v; if(parse_int(value, v)) rs.gkAsSecondA = (v != 0);
				else v_warnings.push_back(lp + _T("value for AATF_GK_AS_SECOND_A is not a number, ignored"));
			}
			else if(key == _T("AATF_MEDALS_CAN_BE_GK"))
			{
				int v; if(parse_int(value, v)) rs.medalsCanBeGK = (v != 0);
				else v_warnings.push_back(lp + _T("value for AATF_MEDALS_CAN_BE_GK is not a number, ignored"));
			}
			else if(key == _T("AATF_MIN_REG_POS"))
			{
				if(!parse_int(value, rs.minRegPos)) v_warnings.push_back(lp + _T("value for AATF_MIN_REG_POS is not a number, ignored"));
			}
			else if(key == _T("AATF_MAX_REG_POS"))
			{
				if(!parse_int(value, rs.maxRegPos)) v_warnings.push_back(lp + _T("value for AATF_MAX_REG_POS is not a number, ignored"));
			}
			else if(key == _T("AATF_MIN_PLAY_STYLE"))
			{
				if(!parse_int(value, rs.minPlayStyle)) v_warnings.push_back(lp + _T("value for AATF_MIN_PLAY_STYLE is not a number, ignored"));
			}
			else if(key == _T("AATF_MAX_PLAY_STYLE"))
			{
				if(!parse_int(value, rs.maxPlayStyle)) v_warnings.push_back(lp + _T("value for AATF_MAX_PLAY_STYLE is not a number, ignored"));
			}
			else if(key == _T("AATF_CHECK_FIRST_PRESET"))
			{
				int v; if(parse_int(value, v)) rs.checkFirstPreset = (v != 0);
				else v_warnings.push_back(lp + _T("value for AATF_CHECK_FIRST_PRESET is not a number, ignored"));
			}
			else if(key == _T("AATF_MAX_SKILL_CARDS"))
			{
				if(!parse_int(value, rs.maxSkillCards)) v_warnings.push_back(lp + _T("value for AATF_MAX_SKILL_CARDS is not a number, ignored"));
			}
			else if(key == _T("AATF_HEIGHT_BRACKETS"))
			{
				if(!parse_int(value, rs.heightBrackets)) v_warnings.push_back(lp + _T("value for AATF_HEIGHT_BRACKETS is not a number, ignored"));
			}
			else if(key == _T("AATF_MANLET_CLASS"))
			{
				rs.manletClass = to_upper(value);
			}
			else if(key == _T("AATF_HA_HEIGHT"))
			{
				if(!parse_int(value, rs.haHeight)) v_warnings.push_back(lp + _T("value for AATF_HA_HEIGHT is not a number, ignored"));
			}
			else if(key == _T("AATF_HEIGHT_MAX_ONLY_CLASSES"))
			{
				rs.maxOnlyClasses.clear();
				std::vector<RString> parts = split(value, _T(','));
				for(size_t ii = 0; ii < parts.size(); ii++)
					if(!parts[ii].empty()) rs.maxOnlyClasses.push_back(to_upper(parts[ii]));
			}
			else if(key == _T("AATF_HEIGHT_POOL_TOTAL"))
			{
				if(!parse_int(value, rs.poolTotal)) v_warnings.push_back(lp + _T("value for AATF_HEIGHT_POOL_TOTAL is not a number, ignored"));
			}
			else if(key == _T("AATF_HEIGHT_POOL_MIN_HEIGHT"))
			{
				if(!parse_int(value, rs.poolMinHeight)) v_warnings.push_back(lp + _T("value for AATF_HEIGHT_POOL_MIN_HEIGHT is not a number, ignored"));
			}
			else if(key == _T("AATF_HEIGHT_POOL_MAX_HEIGHT"))
			{
				if(!parse_int(value, rs.poolMaxHeight)) v_warnings.push_back(lp + _T("value for AATF_HEIGHT_POOL_MAX_HEIGHT is not a number, ignored"));
			}
			else if(key == _T("AATF_HEIGHT_POOL_GK_STRICT"))
			{
				int v; if(parse_int(value, v)) rs.poolGKStrict = (v != 0);
				else v_warnings.push_back(lp + _T("value for AATF_HEIGHT_POOL_GK_STRICT is not a number, ignored"));
			}
			else if(key == _T("AATF_HEIGHT_POOL_GK_HEIGHT"))
			{
				if(!parse_int(value, rs.poolGKHeight)) v_warnings.push_back(lp + _T("value for AATF_HEIGHT_POOL_GK_HEIGHT is not a number, ignored"));
			}
			else if(key == _T("AATF_UNIQUE_GK_STATS"))
			{
				int v; if(parse_int(value, v)) rs.uniqueGKStats = (v != 0);
				else v_warnings.push_back(lp + _T("value for AATF_UNIQUE_GK_STATS is not a number, ignored"));
			}
			else if(key == _T("AATF_CAPTAIN_FREE_CARD"))
			{
				if(!parse_int(value, rs.captainFreeCard)) v_warnings.push_back(lp + _T("value for AATF_CAPTAIN_FREE_CARD is not a number, ignored"));
			}
			else if(key == _T("AATF_CAPTAIN_EXTRA_A_POS"))
			{
				if(!parse_int(value, rs.captainExtraAPos)) v_warnings.push_back(lp + _T("value for AATF_CAPTAIN_EXTRA_A_POS is not a number, ignored"));
			}
			else if(key == _T("AATF_CAPTAIN_EXTRA_COM"))
			{
				if(!parse_int(value, rs.captainExtraCom)) v_warnings.push_back(lp + _T("value for AATF_CAPTAIN_EXTRA_COM is not a number, ignored"));
			}
			//--------------------------------------------------------------
			//Height brackets: AATF_BRACKET_<n>_...
			else if(_tcsnicmp(key.c_str(), _T("AATF_BRACKET_"), 13) == 0)
			{
				const TCHAR* pc_rest = key.c_str() + 13;
				TCHAR* pc_end = NULL;
				long bracketNo = _tcstol(pc_rest, &pc_end, 10);
				if(bracketNo < 1 || bracketNo > 99 || !pc_end || *pc_end != _T('_'))
				{
					v_warnings.push_back(lp + _T("malformed bracket key '") + key + _T("', ignored"));
					continue;
				}

				RString rest(pc_end + 1);
				RString upperRest = to_upper(rest);
				int bracketIndex = (int)bracketNo - 1;
				if(rs.brackets.size() < (size_t)bracketIndex + 1)
					rs.brackets.resize(bracketIndex + 1);
				HeightBracket& bracket = rs.brackets[bracketIndex];

				if(upperRest == _T("NAME"))
				{
					bracket.name = value;
				}
				else if(upperRest == _T("MANLET_BUFF"))
				{
					int v;
					if(parse_int(value, v)) bracket.manletBuff = (v != 0);
					else v_warnings.push_back(lp + _T("value for ") + key + _T(" is not a number, ignored"));
				}
				else if(upperRest == _T("TRIGGER"))
				{
					if(value.empty())
					{
						bracket.hasTrigger = false;
						bracket.trigger = 0;
					}
					else
					{
						int v;
						if(parse_int(value, v)) { bracket.hasTrigger = true; bracket.trigger = v; }
						else v_warnings.push_back(lp + _T("value for ") + key + _T(" is not a number, ignored"));
					}
				}
				else
				{
					//Optional per-class bonuses end in one of these suffixes
					enum { HF_NONE, HF_CARD, HF_COM, HF_APOS, HF_WEAK_USE, HF_WEAK_ACC };
					int field = HF_NONE;
					RString className = upperRest;
					if(ends_with(upperRest, _T("_CARD_BONUS"))) { field = HF_CARD; className = upperRest.substr(0, upperRest.size() - 11); }
					else if(ends_with(upperRest, _T("_COM_BONUS"))) { field = HF_COM; className = upperRest.substr(0, upperRest.size() - 10); }
					else if(ends_with(upperRest, _T("_A_POS_BONUS"))) { field = HF_APOS; className = upperRest.substr(0, upperRest.size() - 12); }
					else if(ends_with(upperRest, _T("_WEAK_USE"))) { field = HF_WEAK_USE; className = upperRest.substr(0, upperRest.size() - 9); }
					else if(ends_with(upperRest, _T("_WEAK_ACC"))) { field = HF_WEAK_ACC; className = upperRest.substr(0, upperRest.size() - 9); }

					if(className.empty())
					{
						v_warnings.push_back(lp + _T("malformed bracket class key '") + key + _T("', ignored"));
						continue;
					}

					HeightClass* phc = find_or_add_class(bracket, className);
					if(field == HF_NONE)
					{
						int h, gk, num;
						if(parse_height_entry(value, h, gk, num))
						{
							phc->height = h;
							phc->gkHeight = gk;
							phc->numPlayers = num;
						}
						else
						{
							v_warnings.push_back(lp + _T("value for ") + key + _T(" is not \"height : gkHeight : numPlayers\", ignored"));
						}
					}
					else
					{
						int v;
						if(parse_int(value, v))
						{
							switch(field)
							{
								case HF_CARD: phc->cardBonus = v; break;
								case HF_COM: phc->comBonus = v; break;
								case HF_APOS: phc->aPosBonus = v; break;
								case HF_WEAK_USE: phc->weakUse = v; break;
								case HF_WEAK_ACC: phc->weakAcc = v; break;
							}
						}
						else v_warnings.push_back(lp + _T("value for ") + key + _T(" is not a number, ignored"));
					}
				}
			}
			//--------------------------------------------------------------
			//AATF_SKILL_CARD_00 .. AATF_SKILL_CARD_40
			else if(_tcsnicmp(key.c_str(), _T("AATF_SKILL_CARD_"), 16) == 0)
			{
				int index;
				if(!parse_int(key.substr(16), index) || index < 0 || index > 40)
				{
					v_warnings.push_back(lp + _T("malformed skill card key '") + key + _T("', ignored"));
					continue;
				}
				int v;
				if(parse_int(value, v) && v >= 0 && v <= 2) rs.skillCard[index] = v;
				else v_warnings.push_back(lp + _T("value for ") + key + _T(" must be 0, 1 or 2, ignored"));
			}
			//--------------------------------------------------------------
			//AATF_HA_NERF_<CLASS>
			else if(_tcsnicmp(key.c_str(), _T("AATF_HA_NERF_"), 13) == 0)
			{
				int ci = class_from_key_suffix(key.substr(13));
				int v;
				if(ci < 0) v_warnings.push_back(lp + _T("unknown class in '") + key + _T("', ignored"));
				else if(!parse_int(value, v)) v_warnings.push_back(lp + _T("value for ") + key + _T(" is not a number, ignored"));
				else rs.classes[ci].haNerf = v;
			}
			//--------------------------------------------------------------
			//AATF_BONUS_AFFECT_<CLASS>
			else if(_tcsnicmp(key.c_str(), _T("AATF_BONUS_AFFECT_"), 18) == 0)
			{
				int ci = class_from_key_suffix(key.substr(18));
				int v;
				if(ci < 0) v_warnings.push_back(lp + _T("unknown class in '") + key + _T("', ignored"));
				else if(!parse_int(value, v)) v_warnings.push_back(lp + _T("value for ") + key + _T(" is not a number, ignored"));
				else rs.bonusAffects[ci] = (v != 0);
			}
			//--------------------------------------------------------------
			//AATF_BONUS_<ABILITY>
			else if(_tcsnicmp(key.c_str(), _T("AATF_BONUS_"), 11) == 0)
			{
				int ab = ability_from_key_name(key.substr(11));
				int v;
				if(ab < 0) v_warnings.push_back(lp + _T("unknown ability in '") + key + _T("', ignored"));
				else if(!parse_int(value, v)) v_warnings.push_back(lp + _T("value for ") + key + _T(" is not a number, ignored"));
				else rs.abilityBonus[ab] = v;
			}
			//--------------------------------------------------------------
			//AATF_GK_<ABILITY> (unique goalkeeper stats)
			else if(_tcsnicmp(key.c_str(), _T("AATF_GK_"), 8) == 0)
			{
				int ab = ability_from_key_name(key.substr(8));
				int v;
				if(ab < 0) v_warnings.push_back(lp + _T("unknown ability in '") + key + _T("', ignored"));
				else if(!parse_int(value, v)) v_warnings.push_back(lp + _T("value for ") + key + _T(" is not a number, ignored"));
				else rs.gkStats[ab] = v;
			}
			//--------------------------------------------------------------
			//AATF_<CLASS>_<ATTRIBUTE>
			else
			{
				bool classHandled = false;
				for(int ci = 0; ci < CLASS_COUNT && !classHandled; ci++)
				{
					RString prefix = RString(_T("AATF_")) + g_class_names[ci] + _T("_");
					if(_tcsnicmp(key.c_str(), prefix.c_str(), prefix.size()) != 0) continue;
					classHandled = true;

					RString attr = key.substr(prefix.size());
					int v;
					if(!parse_int(value, v))
					{
						v_warnings.push_back(lp + _T("value for ") + key + _T(" is not a number, ignored"));
						break;
					}

					PlayerClass& pc = rs.classes[ci];
					if(attr == _T("COUNT")) pc.count = v;
					else if(attr == _T("FORM")) pc.form = v;
					else if(attr == _T("INJURY_RESISTANCE")) pc.injuryResistance = v;
					else if(attr == _T("WEAK_FOOT_USAGE")) pc.weakUse = v;
					else if(attr == _T("WEAK_FOOT_ACCURACY")) pc.weakAcc = v;
					else if(attr == _T("SKILLS")) pc.skills = v;
					else if(attr == _T("TRICKS")) pc.tricks = v;
					else if(attr == _T("COMS")) pc.coms = v;
					else if(attr == _T("A_POS")) pc.aPos = v;
					else if(attr == _T("BASE_STAT")) pc.baseStat = v;
					else if(attr == _T("MANLET_BUFF")) pc.manletBuff = v;
					else if(attr == _T("HA_ALLOWED")) pc.haAllowed = (v != 0);
					else v_warnings.push_back(lp + _T("unknown attribute '") + attr + _T("' in ") + key + _T(", ignored"));
				}
				if(!classHandled) handled = false;
			}

			if(!handled)
				v_warnings.push_back(lp + _T("unknown key '") + key + _T("', ignored"));
		}
		fclose(pFile);

		//--------------------------------------------------------------
		//Sanity checks on the loaded values (warnings only)
		if(rs.heightBrackets > 0 && (size_t)rs.heightBrackets > rs.brackets.size())
		{
			if(rs.brackets.empty())
				v_warnings.push_back(_T("AATF_HEIGHT_BRACKETS is set but no height brackets are defined"));
			else
				v_warnings.push_back(_T("AATF_HEIGHT_BRACKETS asks for more brackets than are defined; the extra ones are ignored"));
			rs.heightBrackets = (int)rs.brackets.size();
		}
		if(rs.heightBrackets > 0 && rs.squadSize > 0)
		{
			int limit = (rs.heightBrackets < (int)rs.brackets.size()) ? rs.heightBrackets : (int)rs.brackets.size();
			for(int bi = 0; bi < limit; bi++)
			{
				int sum = 0;
				for(size_t ci = 0; ci < rs.brackets[bi].classes.size(); ci++)
					sum += rs.brackets[bi].classes[ci].numPlayers;
				if(sum != rs.squadSize)
				{
					TCHAR cs_buf[128];
					_stprintf_s(cs_buf, 128, _T("The height classes in bracket %d add up to %d, not AATF_SQUAD_SIZE (%d)"), bi + 1, sum, rs.squadSize);
					v_warnings.push_back(cs_buf);
				}
			}
		}
		if(rs.classes[CLASS_REGULAR].baseStat == 0)
			v_warnings.push_back(_T("AATF_REGULAR_BASE_STAT is not set"));

		return true;
	}

	//Load the ruleset picked in Settings; false when there is none or it
	//  cannot be read
	bool load_selected(Ruleset& rs)
	{
		if(g_tc_ruleset_file[0] == 0) return false;

		TCHAR cs_path[MAX_PATH];
		make_work_path(cs_path, MAX_PATH, g_tc_ruleset_file);
		std::vector<RString> warnings;
		RString error;
		return load(cs_path, rs, warnings, error);
	}

	//----------------------------------------------------------------------
	//Checker helpers

	static int ability_value(const player_entry& player, int ab)
	{
		switch(ab)
		{
			case AB_OFFENSIVE_AWARENESS: return player.atk;
			case AB_BALL_CONTROL: return player.ball_ctrl;
			case AB_DRIBBLING: return player.drib;
			case AB_LOW_PASS: return player.lowpass;
			case AB_LOFTED_PASS: return player.loftpass;
			case AB_FINISHING: return player.finish;
			case AB_PLACE_KICKING: return player.place_kick;
			case AB_CURL: return player.swerve;
			case AB_HEADER: return player.header;
			case AB_DEFENSIVE_AWARENESS: return player.def;
			case AB_BALL_WINNING: return player.ball_win;
			case AB_KICKING_POWER: return player.kick_pwr;
			case AB_SPEED: return player.speed;
			case AB_ACCELERATION: return player.exp_pwr;
			case AB_BALANCE: return player.body_ctrl;
			case AB_PHYSICAL_CONTACT: return player.phys_cont;
			case AB_JUMP: return player.jump;
			case AB_STAMINA: return player.stamina;
			case AB_GK_AWARENESS: return player.gk;
			case AB_CATCHING: return player.catching;
			case AB_CLEARING: return player.clearing;
			case AB_REFLEXES: return player.reflex;
			case AB_GK_REACH: return player.cover;
			case AB_TIGHT_POSSESSION: return player.tight_pos;
			case AB_AGGRESSION: return player.aggres;
		}
		return 0;
	}

	//Whether an ability exists in the given PES version
	static bool ability_available(int ab, int pesVersion)
	{
		switch(ab)
		{
			case AB_CLEARING:
			case AB_REFLEXES:
			case AB_GK_REACH:
				return pesVersion > 15;
			case AB_PHYSICAL_CONTACT:
				return pesVersion > 16;
			case AB_TIGHT_POSSESSION:
			case AB_AGGRESSION:
				return pesVersion > 19;
		}
		return true;
	}

	//Auto-Manlet: decide whether the manlet buff applies for the entered
	//  height and fill in the resulting ability values
	bool auto_manlet_values(const Ruleset& rs, const int* an_teamHeights, int n_teamSize,
		int n_editedHeight, int pesVersion, bool b_isGK, const int* an_current,
		int* an_filled, int& n_weakUse, int& n_weakAcc)
	{
		if(rs.heightBrackets <= 0 || rs.brackets.empty()) return false;

		//Pick the bracket the team would use, with the edited height in place
		int limit = rs.heightBrackets < (int)rs.brackets.size() ? rs.heightBrackets : (int)rs.brackets.size();
		int active = -1;
		for(int bi = 0; bi < limit && active < 0; bi++)
		{
			if(!rs.brackets[bi].hasTrigger) continue;
			for(int pi = 0; pi < n_teamSize; pi++)
				if(an_teamHeights[pi] >= rs.brackets[bi].trigger) { active = bi; break; }
		}
		if(active < 0 && limit > 0) active = limit - 1;
		if(active < 0) return false;

		const HeightBracket& bracket = rs.brackets[active];
		if(!bracket.manletBuff) return false;

		//The entered height must be exactly the manlet class height
		const HeightClass* phc = NULL;
		for(size_t ci = 0; ci < bracket.classes.size(); ci++)
		{
			if(_tcsicmp(bracket.classes[ci].name.c_str(), rs.manletClass.c_str()) == 0 &&
				n_editedHeight == bracket.classes[ci].height)
			{
				phc = &bracket.classes[ci];
				break;
			}
		}
		if(!phc) return false;

		int an_classes[CLASS_COUNT];
		int n_classes = 0;
		if(b_isGK) an_classes[n_classes++] = CLASS_GOALKEEPER;
		else
		{
			an_classes[n_classes++] = CLASS_REGULAR;
			an_classes[n_classes++] = CLASS_BRONZE;
			an_classes[n_classes++] = CLASS_SILVER;
			an_classes[n_classes++] = CLASS_GOLD;
		}

		for(int ci = 0; ci < n_classes; ci++)
		{
			int classId = an_classes[ci];
			const PlayerClass& pc = rs.classes[classId];
			if(pc.baseStat <= 0) continue;

			//Unique goalkeeper tables are absolute and are never buffed
			bool b_uniqueGK = (b_isGK && rs.uniqueGKStats);

			//The current values must match the class's unbuffed pattern, so
			//  the buff is only ever added once and cannot stack
			bool b_match = true;
			int an_values[ABILITY_COUNT];
			for(int ab = 0; ab < ABILITY_COUNT; ab++)
			{
				int expected;
				if(b_uniqueGK)
					expected = (rs.gkStats[ab] != 0) ? rs.gkStats[ab] : pc.baseStat;
				else
				{
					expected = pc.baseStat;
					if(rs.bonusAffects[classId]) expected += rs.abilityBonus[ab];
				}
				an_values[ab] = expected;

				if(!ability_available(ab, pesVersion)) continue;
				if(ab == AB_OFFENSIVE_AWARENESS || ab == AB_DEFENSIVE_AWARENESS)
				{
					if(an_current[ab] > expected) { b_match = false; break; }
				}
				else if(an_current[ab] != expected) { b_match = false; break; }
			}
			if(!b_match) continue;

			int n_buff = b_uniqueGK ? 0 : pc.manletBuff;
			for(int ab = 0; ab < ABILITY_COUNT; ab++)
				an_filled[ab] = an_values[ab] + n_buff;
			n_weakUse = phc->weakUse;
			n_weakAcc = phc->weakAcc;
			return true;
		}

		return false;
	}

	//The player's overall rating: the highest ability they must have at the
	//  exact target. Attacking/Defensive Prowess are excluded because they
	//  are only capped, and may be lower.
	static int player_rating(const player_entry& player, int pesVersion)
	{
		int rating = 0;
		for(int ab = 0; ab < ABILITY_COUNT; ab++)
		{
			if(!ability_available(ab, pesVersion)) continue;
			if(ab == AB_OFFENSIVE_AWARENESS || ab == AB_DEFENSIVE_AWARENESS) continue;
			int value = ability_value(player, ab);
			if(value > rating) rating = value;
		}
		return rating;
	}

	//Classifies a rating against the bronze/silver/gold base stats. A player
	//  at HA height may also be judged against the nerfed value of a class
	//  that allows height abuse; the nerf only applies to HA players so a
	//  short buffed player can never be mistaken for a nerfed medal player.
	//  Returns CLASS_REGULAR for anything below the lowest medal, or -1 when
	//  the rating falls between classes and matches none of them.
	static int class_from_rating(const Ruleset& rs, int rating, bool isHA)
	{
		struct candidate { int value; int cls; };
		candidate candidates[8];
		int count = 0;

		for(int cls = CLASS_BRONZE; cls <= CLASS_GOLD; cls++)
		{
			int base = rs.classes[cls].baseStat;
			if(base <= 0) continue;
			candidates[count].value = base;
			candidates[count].cls = cls;
			count++;
			if(isHA && rs.classes[cls].haAllowed && rs.classes[cls].haNerf > 0)
			{
				candidates[count].value = base - rs.classes[cls].haNerf;
				candidates[count].cls = cls;
				count++;
			}
		}

		if(count == 0) return CLASS_REGULAR;

		//Sort ascending (at most 6 entries)
		for(int ii = 0; ii < count; ii++)
			for(int jj = 0; jj + 1 < count - ii; jj++)
				if(candidates[jj].value > candidates[jj + 1].value)
				{
					candidate tmp = candidates[jj];
					candidates[jj] = candidates[jj + 1];
					candidates[jj + 1] = tmp;
				}

		for(int ii = 0; ii < count; ii++)
			if(rating == candidates[ii].value)
				return candidates[ii].cls;

		if(rating < candidates[0].value) return CLASS_REGULAR;
		return -1;
	}

	static bool is_max_only_class(const Ruleset& rs, const RString& name)
	{
		for(size_t ii = 0; ii < rs.maxOnlyClasses.size(); ii++)
			if(_tcsicmp(rs.maxOnlyClasses[ii].c_str(), name.c_str()) == 0)
				return true;
		return false;
	}

	//Finds the height class that describes the player's height inside a
	//  bracket. Goalkeepers may match a class's goalkeeper height as well as
	//  its normal height. Returns -1 when the height is not legal.
	static int match_height_class(const Ruleset& rs, const HeightBracket& bracket, const player_entry& player, bool isGK)
	{
		//Exact heights first so a class with a fixed height beats a
		//  "height <= value" class that would also match
		for(size_t ii = 0; ii < bracket.classes.size(); ii++)
		{
			const HeightClass& hc = bracket.classes[ii];
			if(isGK && hc.gkHeight != 0 && (int)player.height == hc.gkHeight)
				return (int)ii;
			if((int)player.height == hc.height)
				return (int)ii;
		}

		for(size_t ii = 0; ii < bracket.classes.size(); ii++)
		{
			const HeightClass& hc = bracket.classes[ii];
			if(is_max_only_class(rs, hc.name) && (int)player.height <= hc.height)
				return (int)ii;
		}

		return -1;
	}

	//----------------------------------------------------------------------
	//The generic AATF check

	void check_team(HWND hAatfbox, int pesVersion, int teamSel, player_entry* gplayers, team_entry* gteams, int gnum_players, bool useSuggestions)
	{
		RString msgOut;
		msgOut += _T("Team: ");
		msgOut += gteams[teamSel].name;
		msgOut += _T("\r\n");

		//A ruleset is mandatory: there is nothing left to check against otherwise
		if(g_tc_ruleset_file[0] == 0)
		{
			msgOut += _T("\r\nERROR: No AATF ruleset is selected.\r\n");
			msgOut += _T("Open File > Settings..., pick a ruleset .cfg file in the \"AATF Ruleset\" dropdown and close the window, then run the check again.\r\n");
			SetWindowText(GetDlgItem(hAatfbox, IDT_AATFOUT), msgOut.c_str());
			SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM)_T("No ruleset selected"));
			return;
		}

		Ruleset rs;
		std::vector<RString> warnings;
		RString error;
		TCHAR cs_ruleset_path[MAX_PATH];
		make_work_path(cs_ruleset_path, MAX_PATH, g_tc_ruleset_file);
		if(!load(cs_ruleset_path, rs, warnings, error))
		{
			msgOut += _T("\r\n");
			msgOut += error;
			msgOut += _T("\r\n");
			SetWindowText(GetDlgItem(hAatfbox, IDT_AATFOUT), msgOut.c_str());
			SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM)_T("Ruleset could not be loaded"));
			return;
		}

		if(warnings.size() > 0)
		{
			msgOut += _T("Ruleset warnings:\r\n");
			for(size_t ii = 0; ii < warnings.size(); ii++)
			{
				msgOut += _T("\t");
				msgOut += warnings[ii];
				msgOut += _T("\r\n");
			}
		}

		int rsVersion = (rs.pesVersion > 0) ? rs.pesVersion : pesVersion;
		if(rs.pesVersion > 0 && rs.pesVersion != pesVersion)
		{
			TCHAR cs_note[128];
			_stprintf_s(cs_note, 128, _T("Note: this ruleset is written for PES%d but the loaded EDIT file is PES%d; card and ability checks may not line up.\r\n"), rs.pesVersion, pesVersion);
			msgOut += cs_note;
		}

		//Collect the team's players
		std::vector<player_entry> players;
		for(int ii = 0; ii < gteams[teamSel].num_on_team; ii++)
		{
			for(int jj = 0; jj < gnum_players; jj++)
			{
				if(gplayers[jj].id == gteams[teamSel].players[ii])
				{
					players.push_back(gplayers[jj]);
					break;
				}
			}
		}

		//Pick the height bracket: the first bracket whose trigger is met by
		//  any player, with the last bracket as the fallback
		int activeBracket = -1;
		if(rs.heightBrackets > 0)
		{
			int limit = (rs.heightBrackets < (int)rs.brackets.size()) ? rs.heightBrackets : (int)rs.brackets.size();
			for(int bi = 0; bi < limit && activeBracket < 0; bi++)
			{
				if(!rs.brackets[bi].hasTrigger) continue;
				for(size_t pi = 0; pi < players.size(); pi++)
					if((int)players[pi].height >= rs.brackets[bi].trigger)
					{
						activeBracket = bi;
						break;
					}
			}
			if(activeBracket < 0 && limit > 0) activeBracket = limit - 1;

			if(activeBracket < 0)
			{
				msgOut += _T("\r\nERROR: The ruleset enables height checks but defines no height brackets.\r\n");
				SetWindowText(GetDlgItem(hAatfbox, IDT_AATFOUT), msgOut.c_str());
				SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM)_T("Ruleset has no height brackets"));
				return;
			}
			if(!rs.brackets[activeBracket].name.empty())
			{
				msgOut += _T("Using height bracket: ");
				msgOut += rs.brackets[activeBracket].name;
				msgOut += _T("\r\n");
			}
		}
		else if(rs.heightBrackets == -1)
		{
			msgOut += _T("Using pooled height system\r\n");
		}
		else
		{
			msgOut += _T("Height checks are disabled by the ruleset\r\n");
		}

		const HeightBracket* pcActive = (activeBracket >= 0) ? &rs.brackets[activeBracket] : NULL;
		std::vector<int> heightCounts;
		if(pcActive) heightCounts.resize(pcActive->classes.size(), 0);
		int poolHeight = 0;

		//--------------------------------------------------------------
		//Per-player checks
		int errorTot = 0;
		int numGK = 0, numReg = 0, numBronze = 0, numSilver = 0, numGold = 0;
		bool hasCaptain = false;
		bool captainHasFreeCard = false;

		for(size_t pi = 0; pi < players.size(); pi++)
		{
			player_entry& player = players[pi];
			msgOut += _T("\x2022 Checking ");
			msgOut += player.name;
			msgOut += _T("\r\n");

			std::basic_stringstream<TCHAR> errorMsg;
			bool isGK = (player.reg_pos == 0);
			bool isCaptain = (player.id == gteams[teamSel].players[gteams[teamSel].captain_ind]);
			if(isCaptain) hasCaptain = true;

			//----------------------------------------------------------
			//Registered position and playing style ranges
			int minRP = (rs.minRegPos >= 0) ? rs.minRegPos : 0;
			int maxRP = (rs.maxRegPos >= 0) ? rs.maxRegPos : 12;
			if((int)player.reg_pos < minRP || (int)player.reg_pos > maxRP)
			{
				errorTot++;
				errorMsg << _T("Registered position out of range (") << minRP << _T("-") << maxRP << _T("); ");
			}

			if(rs.minPlayStyle >= 0 || rs.maxPlayStyle >= 0)
			{
				int minPS = (rs.minPlayStyle >= 0) ? rs.minPlayStyle : 0;
				int maxPS = (rs.maxPlayStyle >= 0) ? rs.maxPlayStyle : 21;
				if(player.play_style < minPS || player.play_style > maxPS)
				{
					errorTot++;
					errorMsg << _T("Playing style out of range (") << minPS << _T("-") << maxPS << _T("); ");
				}
			}
			else if(pesVersion <= 16)
			{
				if(player.play_style > 18 || player.play_style == 16)
				{
					errorTot++;
					errorMsg << _T("Playing style out of range (0-18, excluding 16); ");
				}
			}
			else if(pesVersion < 19)
			{
				if(player.play_style > 17)
				{
					errorTot++;
					errorMsg << _T("Playing style out of range (0-17); ");
				}
			}
			else
			{
				if(player.play_style > 21)
				{
					errorTot++;
					errorMsg << _T("Playing style out of range (0-21); ");
				}
			}

			//----------------------------------------------------------
			//Game-defined age and weight limits
			if(player.age < 15 || player.age > 50)
			{
				errorTot++;
				errorMsg << _T("Age out of range (15,50); ");
			}
			int minWeight = max(30, (int)player.height - 129);
			int maxWeight = (int)player.height - 81;
			if((int)player.weight < minWeight || (int)player.weight > maxWeight)
			{
				errorTot++;
				errorMsg << _T("Weight out of range (") << minWeight << _T(",") << maxWeight << _T("); ");
			}

			//----------------------------------------------------------
			//Playable positions
			int countA = 0, countB = 0;
			for(int jj = 0; jj < 13; jj++)
			{
				if(player.play_pos[jj] == 2) countA++;
				else if(player.play_pos[jj] == 1) countB++;
			}
			if(countB > 0 && !rs.allowBPosition)
			{
				errorTot++;
				errorMsg << _T("Has B position; ");
			}
			if(player.reg_pos <= 12 && player.play_pos[g_regPosToPlayPos[player.reg_pos]] != 2)
			{
				errorTot++;
				errorMsg << _T("Doesn't have A in registered position; ");
			}
			if(!rs.gkAsSecondA && player.reg_pos != 0 && player.play_pos[12] == 2)
			{
				errorTot++;
				errorMsg << _T("Has GK as second A position; ");
			}

			//----------------------------------------------------------
			//Cards. Status 1 cards count, status 2 cards are free up to the
			//  class's free card allowance, status 0 cards are banned.
			int numSkill = 28;
			if(rsVersion == 19) numSkill = 39;
			else if(rsVersion > 19) numSkill = 41;
			if(numSkill > 41) numSkill = 41;

			int numSkillSet = 0, numFreeSet = 0, numCom = 0;
			for(int jj = 0; jj < numSkill; jj++)
			{
				if(!player.play_skill[jj]) continue;
				if(rs.skillCard[jj] == 0)
				{
					errorTot++;
					errorMsg << _T("Skill card '") << g_skill_card_names[jj] << _T("' is banned; ");
				}
				else if(jj == rs.captainFreeCard && isCaptain)
				{
					numFreeSet++;
					captainHasFreeCard = true;
				}
				else if(rs.skillCard[jj] == 2) numFreeSet++;
				else numSkillSet++;
			}
			for(int jj = 0; jj < 7; jj++)
				if(player.com_style[jj]) numCom++;

			//----------------------------------------------------------
			//The player must be in their registered position in the first preset
			if(rs.checkFirstPreset && !aatf_check_player_in_pos_first_preset(gteams[teamSel], player, player.reg_pos, false))
			{
				errorTot++;
				errorMsg << _T("Player is a registered ") << aatf_get_position_name_from_byte(player.reg_pos)
					<< _T(" but is not in that position in the first preset; ");
			}

			//----------------------------------------------------------
			//Height class first, so its bonuses can feed the class checks
			const HeightClass* phc = NULL;
			if(pcActive)
			{
				int hcIndex = match_height_class(rs, *pcActive, player, isGK);
				if(hcIndex < 0)
				{
					errorTot++;
					errorMsg << _T("Illegal height (") << (int)player.height << _T(" cm); ");
				}
				else
				{
					phc = &pcActive->classes[hcIndex];
					heightCounts[hcIndex]++;
				}
			}
			else if(rs.heightBrackets == -1)
			{
				//Pooled mode has no height classes: any height in the
				//  configured range is legal and counts at its own value
				bool legal = true;
				if(rs.poolMinHeight > 0 && (int)player.height < rs.poolMinHeight) legal = false;
				if(rs.poolMaxHeight > 0 && (int)player.height > rs.poolMaxHeight) legal = false;
				if(isGK && rs.poolGKStrict && (int)player.height != rs.poolGKHeight) legal = false;
				if(!legal)
				{
					errorTot++;
					errorMsg << _T("Illegal height (") << (int)player.height << _T(" cm); ");
				}
				else
				{
					poolHeight += player.height;
				}
			}

			//----------------------------------------------------------
			//Work out which class the player is judged against
			int rating = player_rating(player, pesVersion);
			bool isHA = (rs.haHeight > 0 && (int)player.height >= rs.haHeight);
			int classId;
			if(isGK)
			{
				int medalClass = class_from_rating(rs, rating, isHA);
				if(medalClass > CLASS_REGULAR && !rs.medalsCanBeGK)
				{
					errorTot++;
					errorMsg << _T("Medals cannot play as GK; ");
				}
				classId = CLASS_GOALKEEPER;
			}
			else
			{
				classId = class_from_rating(rs, rating, isHA);
				if(classId < 0)
				{
					errorTot++;
					errorMsg << _T("Illegal Ability scores, this player's stats do not match any player class; ");
					if(errorMsg.rdbuf()->in_avail())
					{
						msgOut += _T("\t");
						msgOut += errorMsg.str();
						msgOut += _T("\r\n");
					}
					continue;
				}
			}

			const PlayerClass& pc = rs.classes[classId];
			bool haNerfApplies = (isHA && pc.haAllowed && pc.haNerf > 0);
			if(isHA && !pc.haAllowed)
			{
				errorTot++;
				errorMsg << _T("Height abuse is not allowed for this player class; ");
			}

			//Counts used at team level. Goalkeepers count as regular players
			//  for head-count purposes but keep their own allowances.
			if(isGK) numGK++;
			if(classId == CLASS_REGULAR || classId == CLASS_GOALKEEPER) numReg++;
			if(classId == CLASS_BRONZE) numBronze++;
			else if(classId == CLASS_SILVER) numSilver++;
			else if(classId == CLASS_GOLD) numGold++;

			//----------------------------------------------------------
			//Allowances, including the height class bonuses and the captain's
			int freeTricks = pc.tricks;
			int freeComs = pc.coms + (phc ? phc->comBonus : 0) + (isCaptain ? rs.captainExtraCom : 0);
			int freeA = pc.aPos + (phc ? phc->aPosBonus : 0) + (isCaptain ? rs.captainExtraAPos : 0);
			int skillsAllowed = pc.skills + (phc ? phc->cardBonus : 0);
			int weakUse = (phc && phc->weakUse != 0) ? phc->weakUse : pc.weakUse;
			int weakAcc = (phc && phc->weakAcc != 0) ? phc->weakAcc : pc.weakAcc;
			bool manletStatBuff = (phc != NULL && pcActive->manletBuff && !rs.manletClass.empty()
				&& _tcsicmp(phc->name.c_str(), rs.manletClass.c_str()) == 0);

			if(pc.form > 0 && (int)player.form + 1 != pc.form)
			{
				errorTot++;
				errorMsg << _T("Form is ") << (int)player.form + 1 << _T(", should be ") << pc.form << _T("; ");
			}
			if((int)player.injury + 1 > pc.injuryResistance)
			{
				errorTot++;
				errorMsg << _T("Injury resist is ") << (int)player.injury + 1 << _T(", should be ") << pc.injuryResistance << _T("; ");
			}
			if((int)player.weak_use + 1 > weakUse)
			{
				errorTot++;
				errorMsg << _T("Weak foot usage > ") << weakUse << _T("; ");
			}
			if((int)player.weak_acc + 1 > weakAcc)
			{
				errorTot++;
				errorMsg << _T("Weak foot accuracy > ") << weakAcc << _T("; ");
			}

			int countingCards = numSkillSet
				+ max(0, numFreeSet - freeTricks)
				+ max(0, numCom - freeComs)
				+ max(0, countA - freeA);
			if(countingCards > skillsAllowed)
			{
				errorTot++;
				errorMsg << _T("Has ") << countingCards << _T(" cards, only allowed ") << skillsAllowed << _T("; ");
			}
			if(rs.maxSkillCards > 0 && (numSkillSet + numFreeSet) > rs.maxSkillCards)
			{
				errorTot++;
				errorMsg << _T("Has ") << (numSkillSet + numFreeSet) << _T(" skill cards, PES limit is ")
					<< rs.maxSkillCards << _T(", please swap to COM cards or trade for additional A positions; ");
			}

			//----------------------------------------------------------
			//Every ability must match the class target (Attacking and
			//  Defensive Prowess only have to stay at or below it)
			for(int ab = 0; ab < ABILITY_COUNT; ab++)
			{
				if(!ability_available(ab, pesVersion)) continue;

				int target;
				if(isGK && rs.uniqueGKStats)
				{
					target = (rs.gkStats[ab] != 0) ? rs.gkStats[ab] : pc.baseStat;
				}
				else
				{
					target = pc.baseStat;
					if(rs.bonusAffects[classId]) target += rs.abilityBonus[ab];
					if(manletStatBuff) target += pc.manletBuff;
					if(haNerfApplies) target -= pc.haNerf;
				}

				int value = ability_value(player, ab);
				if(ab == AB_OFFENSIVE_AWARENESS || ab == AB_DEFENSIVE_AWARENESS)
				{
					if(value > target)
					{
						errorTot++;
						errorMsg << g_ability_names[ab] << _T(" is ") << value << _T(", should be <= ") << target << _T("; ");
					}
				}
				else if(value != target)
				{
					errorTot++;
					errorMsg << g_ability_names[ab] << _T(" is ") << value << _T(", should be ") << target << _T("; ");
				}
			}

			//----------------------------------------------------------
			//Suggestions are warnings, not errors
			if(useSuggestions)
			{
				if(countingCards < skillsAllowed)
					errorMsg << _T("WARN: Has ") << countingCards << _T(" cards, allowed ") << skillsAllowed << _T("; ");
				if(numCom < freeComs)
					errorMsg << _T("WARN: Has ") << numCom << _T(" COM cards, allowed ") << freeComs << _T("; ");
				if(countA < freeA)
					errorMsg << _T("WARN: Has ") << countA << _T(" A positions, allowed ") << freeA << _T("; ");
				if((int)player.weak_use + 1 < weakUse)
					errorMsg << _T("WARN: Has weak usage ") << (int)player.weak_use + 1 << _T(", allowed ") << weakUse << _T("; ");
				if((int)player.weak_acc + 1 < weakAcc)
					errorMsg << _T("WARN: Has weak accuracy ") << (int)player.weak_acc + 1 << _T(", allowed ") << weakAcc << _T("; ");
				if((int)player.injury + 1 < pc.injuryResistance)
					errorMsg << _T("WARN: Has inj resist ") << (int)player.injury + 1 << _T(", allowed ") << pc.injuryResistance << _T("; ");
			}

			if(errorMsg.rdbuf()->in_avail())
			{
				msgOut += _T("\t");
				msgOut += errorMsg.str();
				msgOut += _T("\r\n");
			}
		}

		//--------------------------------------------------------------
		//Team level checks
		std::basic_stringstream<TCHAR> teamMsg;

		if(rs.requireCaptain && !hasCaptain)
		{
			errorTot++;
			teamMsg << _T("Team must have an assigned Captain; ");
		}
		if(useSuggestions && hasCaptain && rs.captainFreeCard >= 0 && rs.captainFreeCard <= 40 && !captainHasFreeCard)
		{
			teamMsg << _T("WARN: Captain does not have the free Captaincy card; ");
		}
		if(rs.squadSize > 0 && (int)players.size() != rs.squadSize)
		{
			errorTot++;
			teamMsg << _T("Squad has ") << (int)players.size() << _T(" players, should be ") << rs.squadSize << _T("; ");
		}
		if(numGK < rs.minGK)
		{
			errorTot++;
			teamMsg << _T("Team must have at least ") << rs.minGK << _T(" registered GK; ");
		}

		if(numReg != rs.classes[CLASS_REGULAR].count)
		{
			errorTot++;
			teamMsg << _T("Number of Regular players is ") << numReg << _T(", should be ") << rs.classes[CLASS_REGULAR].count << _T("; ");
		}
		if(numBronze != rs.classes[CLASS_BRONZE].count)
		{
			errorTot++;
			teamMsg << _T("Number of Bronze medals is ") << numBronze << _T(", should be ") << rs.classes[CLASS_BRONZE].count << _T("; ");
		}
		if(numSilver != rs.classes[CLASS_SILVER].count)
		{
			errorTot++;
			teamMsg << _T("Number of Silver medals is ") << numSilver << _T(", should be ") << rs.classes[CLASS_SILVER].count << _T("; ");
		}
		if(numGold != rs.classes[CLASS_GOLD].count)
		{
			errorTot++;
			teamMsg << _T("Number of Gold medals is ") << numGold << _T(", should be ") << rs.classes[CLASS_GOLD].count << _T("; ");
		}

		if(pcActive)
		{
			for(size_t ii = 0; ii < pcActive->classes.size(); ii++)
			{
				if(heightCounts[ii] != pcActive->classes[ii].numPlayers)
				{
					errorTot++;
					teamMsg << _T("Has ") << heightCounts[ii] << _T("/") << pcActive->classes[ii].numPlayers << _T(" ")
						<< pcActive->classes[ii].name << _T(" (") << pcActive->classes[ii].height << _T(" cm) players; ");
				}
			}
		}
		if(rs.heightBrackets == -1 && rs.poolTotal > 0 && poolHeight > rs.poolTotal)
		{
			errorTot++;
			teamMsg << _T("Total height is ") << poolHeight << _T(" cm, allowed ") << rs.poolTotal << _T(" cm; ");
		}

		if(teamMsg.rdbuf()->in_avail())
		{
			msgOut += teamMsg.str();
			msgOut += _T("\r\n");
		}
		TCHAR cs_errors[64];
		_stprintf_s(cs_errors, 64, _T("\r\nErrors: %d\r\n"), errorTot);
		msgOut += cs_errors;

		SetWindowText(GetDlgItem(hAatfbox, IDT_AATFOUT), msgOut.c_str());
		if(errorTot)
			SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM)_T("It's all fucked."));
		else
			SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM)_T("Perfect, blaze."));
	}
}

//Global entry point declared in editor.h; all AATF menu commands call this
void aatf_check_ruleset(HWND hAatfbox, int pesVersion, int teamSel, player_entry* gplayers, team_entry* gteams, int gnum_players, bool useSuggestions)
{
	aatf_ruleset::check_team(hAatfbox, pesVersion, teamSel, gplayers, gteams, gnum_players, useSuggestions);
}

