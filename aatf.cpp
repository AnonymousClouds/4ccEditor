#include <string>
#include <sstream>
#include "editor.h"
#include "resource.h"
#include "stats.h"
#include "aatf.h"
#include <list>
#ifndef UNICODE  
typedef std::string tstring;
typedef std::stringstream tstringstream;
#else
typedef std::wstring tstring;
typedef std::wstringstream tstringstream;
#endif

//Struct to hold the necessary data to check if each player skill matches its target rating and, if not, print an error message
struct skillCheck
{
	unsigned char c_skillRate; //The player's rating in a skill
	tstring s_skillName; //The skill name
	int n_minPesVersion; //The PES version in which skill was introduced (0 if in all versions)
	int n_targetRate; //What the player's rating in this skill should be under the rules
};

//This array encodes the mapping between the registered position IDs (0-12) used in the player_export struct's reg_pos field
// and the corresponding index in the playable position array (player_export play_pos).  It is used to check that a player has 
// playable position set to A in the same position in which they are registered.
/* position	reg_pos	play_pos
	GK		0		12
	CB		1		9
	LB		2		10
	RB		3		11
	DMF		4		5
	CMF		5		6
	LMF		6		7
	RMF		7		8
	AMF		8		4
	LWF		9		2
	RWF		10		3
	SS		11		1
	CF		12		0 */
int regPosToPlayPosMap[13] = { 12, 9, 10, 11, 5, 6, 7, 8, 4, 2, 3, 1, 0 };

//The following is a list of all the skills and their corresponding IDs
//An * means this is considered a skill card
/*
ID	Name
0	Scissors Feint *
1	Flip Flap *
2	Marseille Turn *
3	Sombrero *
4	Cut Behind & Turn *
5	Scotch Move *
6	Heading
7	Long Range Drive
8	Knuckle Shot
9	Acrobatic Finishing
10	Heel Trick
11	First Time Shot
12	One Shot Pass
13	Weighted Pass
14	Pinpoint Crossing
15	Outside Curler
16	Rabona *
17	Low Lofted Pass
18 	Low Punt Trajectory
19	Long Throw
20	GK Long Throw
21	Malicia *
22	Man Marking
23	Track Back
24	Acrobatic Clear
25	Captaincy
26	Super Sub
27	Fighting Spirit
28	Double Touch *
29	Crossover Turn *
30	Step On Skill *
31	Chip Shot
32	Dipping Shot
33	Rising Shots
34	No Look Pass *
35	High Punt Trajectory
36	Penalty Specialist
37	GK Penalty Specialist
38	Interception
39	Long Range Shooting
40	Through Passing
*/

//============================
//AATF Settings
int manletBonus = 5;
int silverManletBonus = 0;
int goldManletBonus = 0;
int silverGiantPen = 0;
int goldGiantPen = 0;

int goldRate = 99; //Player skill ratings
int silverRate = 88;
int regRate = 77;
int gkRate = 77;

int reqNumGold = 2; //Numbers of medals
int reqNumSilver = 2;

int goldForm = 8; //possible range 1-8
int silverForm = 8;
int regForm = 4;

int goldIR = 3; //Injury resistence (possible range 1-3)
int silverIR = 3;
int regIR = 1;

int goldWeakFootUse = 2; //Gold medal weak foot usage limit
int silverWeakFootUse = 2;
int regWeakFootUse = 2;

int goldWeakFootAcc = 4; //Gold medal weak foot accuracy limit
int silverWeakFootAcc = 4;
int regWeakFootAcc = 2;

int manletCardBonus = 1; //Manlets get 1 extra card
int manletWeakFootUse = 4; //Manlets get 4/4 weak foot usage/accuracy
int manletWeakFootAcc = 4;
int manletPosBonus = 1; //Manlets get 1 extra double A position

int gkSkillCards = 2; //Skill cards
int regSkillCards = 3;
int silverSkillCards = 4;
int goldSkillCards = 5;

int gkTrickCards = 0; //Trick cards
int regTrickCards = 2;
int silverTrickCards = 3;
int goldTrickCards = 3;

int regCOM = 0; //COM playing styles
int silverCOM = 1;
int goldCOM = 2;

int greenGiga = 0; //Green height bracket
int greenGiant = 5;
int greenTall = 6;
int greenMid = 6;
int greenManlet = 6;

int redGiga = 0; //Red height bracket
int redGiant = 0;
int redTall = 10;
int redMid = 7;
int redManlet = 6;

int heightGiga = 199; //Player heights in each category
int heightGiant = 194;
int heightTall = 185;
int heightTallGK = 189;
int heightMid = 180;
int heightManlet = 175;

/*
//VGL Settings
int manletBonus_vgl = 5;
int silverManletBonus_vgl = 2;
int goldManletBonus_vgl = 2;
int goldGiantPen_vgl = 0;
int silverGiantPen_vgl = 0;
int goldRate_vgl = 99;
int silverRate_vgl = 88;
int regRate_vgl = 77;
int gkRate_vgl = 77;
int reqNumGold_vgl = 2;
int reqNumSilver_vgl = 3;

int goldForm_vgl = 8;
int silverForm_vgl = 8;
int regForm_vgl = 4;

int goldIR_vgl = 2; //Injury resistence
int silverIR_vgl = 2;
int regIR_vgl = 1;

int goldWeakFoot_vgl = 4;
int silverWeakFoot_vgl = 4;
int regWeakFoot_vgl = 2;

int gkSkillCards_vgl = 3;
int regSkillCards_vgl = 3;
int silverSkillCards_vgl = 4;
int goldSkillCards_vgl = 5;

int gkTrickCards_vgl = 99;
int regTrickCards_vgl = 99;
int silverTrickCards_vgl = 99;
int goldTrickCards_vgl = 99;

int gkCOM_vgl = 0;
int regCOM_vgl = 0;
int silverCOM_vgl = 1;
int goldCOM_vgl = 1;

int blueColossal_vgl = 0;
int blueGiant_vgl = 0;
int blueTall_vgl = 0;
int blueMid_vgl = 0;
int blueManlet_vgl = 0;

int purpleColossal_vgl = 0;
int purpleGiant_vgl = 6;
int purpleTall_vgl = 5;
int purpleMid_vgl = 6;
int purpleManlet_vgl = 6;

int heightColossal_vgl = 210;
int heightGiant_vgl = 190;
int heightTall_vgl = 185;
int heightTallGK_vgl = 189;
int heightMid_vgl = 180;
int heightManlet_vgl = 175;

int freeCardOne_vgl = 9;

bool canMedalsBeGiant_vgl = false;
bool canGKBeGiant_vgl = false;

int numGK_vgl = 0;
//Count of player ratings
int numReg_vgl = 0;
int numSilver_vgl = 0;
int numGold_vgl = 0;
//Count of height brackets
int numColossal_vgl = 0;
int numGiant_vgl = 0;
int numTall_vgl = 0;
int numMid_vgl = 0;
int numManlet_vgl = 0;


int goldA_vgl = 2;
int silverA_vgl = 2;
int regA_vgl = 2;
int manletA_vgl = 3;
*/


void aatf_single(HWND hAatfbox, int pesVersion, int teamSel, player_entry* gplayers, team_entry* gteams, int gnum_players)
{
	player_entry player;
	tstring msgOut;
	msgOut += _T("Team: ");
	msgOut += gteams[teamSel].name;
	msgOut += _T("\r\n");

	//============================

	bool hasCaptain = false;
	int numGK = 0;
	//Count of player ratings
	int numReg = 0;
	int numSilver = 0;
	int numGold = 0;
	//Count of height brackets
	int numGiga = 0;
	int numGiant = 0;
	int numTall = 0;
	int numMid = 0;
	int numManlet = 0;
	bool usingRed = true;
	bool eCheck = false;

	//Run through all players once to determine height system
	for (int ii = 0; ii < gteams[teamSel].num_on_team; ii++)
	{
		//Find each player on team
		for (int jj = 0; jj < gnum_players; jj++)
		{
			if (gplayers[jj].id == gteams[teamSel].players[ii])
			{
				player = gplayers[jj];
				break;
			}
		}
		//If any player is in the green height brackets, team is on Green height system and we can halt
		if (player.height >= heightGiant)
		{
			usingRed = false;
			break;
		}
	}

	//Now check each player for errors
	int errorTot = 0;
	for (int ii = 0; ii < gteams[teamSel].num_on_team; ii++)
	{
		//Find each player on team
		for (int jj = 0; jj < gnum_players; jj++)
		{
			if (gplayers[jj].id == gteams[teamSel].players[ii])
			{
				player = gplayers[jj];
				break;
			}
		}

		msgOut += _T("\x2022 Checking ");
		msgOut += player.name;
		msgOut += _T("\r\n");

		tstringstream errorMsg;		

		int cardCount = 0;
		int cardMod = 0;
		int cardLimit = 0;
		int heightMod = 0;
		int weakFootUse = 0;
		int weakFootAcc = 0;
		bool hasTrick = false;
		int targetRate = 0, targetRate2 = 0, targetRate3 = 0;
		int rating = player.drib;
		rating = max(player.gk, rating);
		rating = max(player.finish, rating);
		rating = max(player.lowpass, rating);
		rating = max(player.loftpass, rating);
		rating = max(player.header, rating);
		rating = max(player.swerve, rating);
		rating = max(player.catching, rating);
		if (pesVersion > 15)
		{
			rating = max(player.clearing, rating);
			rating = max(player.reflex, rating);
			rating = max(player.cover, rating);
		}
		rating = max(player.body_ctrl, rating);
		if (pesVersion > 16) rating = max(player.phys_cont, rating); //Not in 16
		rating = max(player.kick_pwr, rating);
		rating = max(player.exp_pwr, rating);
		rating = max(player.ball_ctrl, rating);
		rating = max(player.ball_win, rating);
		rating = max(player.jump, rating);
		rating = max(player.place_kick, rating);
		rating = max(player.stamina, rating);
		rating = max(player.speed, rating);
		if (pesVersion > 19) rating = max(player.aggres, rating);

		/*if(player.injury+1 > 3)
		{
			errorTot++;
			errorMsg << _T("Injury resist is ") << player.injury+1 << _T(", cannot exceed 3; ");
		}*/

		//Check if this player is the captain
		if (player.id == gteams[teamSel].players[gteams[teamSel].captain_ind]) hasCaptain = true;

		//Check if registered pos has playable set to A
		int requiredAPos = regPosToPlayPosMap[player.reg_pos];
		if (player.play_pos[requiredAPos] != 2)
		{
			errorTot++;
			errorMsg << _T("Doesn't have A in registered position; ");
		}

		//Count number of registered GKs
		if (player.reg_pos == 0) numGK++;

		//Count A positions
		int countA = 0;
		int countB = 0;
		for (int jj = 0; jj < 13; jj++)
		{
			if (player.play_pos[jj] == 2)
				countA++;
			else if (player.play_pos[jj] == 1)
				countB++;
		}

		//No B positions allowed:
		if (countB > 0)
		{
			errorTot++;
			errorMsg << _T("Has B position; ");
		}


		//If more than 1 A, 1 card less for each
		if (countA > 1)
		{
			if (player.play_pos[12] == 2) //Can't have GK as second A
			{
				errorTot++;
				errorMsg << _T("Has GK as second A position; ");
			}
			cardMod -= (countA - 1);
		}

		//Count cards
		int numTrick = 0;
		int numCom = 0;
		int numSkill;
		if (pesVersion == 19) numSkill = 39;
		else if (pesVersion > 19) numSkill = 41;
		else numSkill = 28;
		for (int jj = 0; jj < numSkill; jj++)
		{
			if (player.play_skill[jj])
			{
				cardCount++;
				//SPECIAL Winter/Spring 24: Malicia (21) is a free card
				if (jj == 21) cardMod++;
				//Captain gets free captaincy card
				if (jj == 25 && player.id == gteams[teamSel].players[gteams[teamSel].captain_ind])
					cardMod++;
				//Trick cards may be free, count number
				if (jj < 6 || jj == 16 || jj == 28 || jj == 29 || jj == 30 || jj == 34)
				{
					hasTrick = true;
					numTrick++;
				}
			}
		}
		for (int jj = 0; jj < 7; jj++)
		{
			if (player.com_style[jj])
			{
				cardCount++;
				numCom++;
			}
		}

		if (player.age < 15 || player.age>50)
		{
			errorTot++;
			errorMsg << _T("Age out of range (15,50); ");
		}

		if (player.weight<max(30, player.height - 129) || player.weight>(player.height - 81))
		{
			errorTot++;
			errorMsg << _T("Weight out of range (") << max(30, player.height - 129) << _T(",") << player.height - 81 << _T("); ");
		}

		//Check playing style and registered position are in valid range per PES version
		if (player.reg_pos > 12)
		{
			errorTot++;
			errorMsg << _T("Registered position out of range (0-12); ");
		}

		if (pesVersion <= 16)
		{
			if (player.play_style > 18 || player.play_style == 16)
			{
				errorTot++;
				errorMsg << _T("Playing style out of range (0-18, excluding 16); ");
			}
		}
		else if (pesVersion > 16 && pesVersion < 19)
		{
			if (player.play_style > 17)
			{
				errorTot++;
				errorMsg << _T("Playing style out of range (0-17); ");
			}
		}
		else
		{
			if (player.play_style > 21)
			{
				errorTot++;
				errorMsg << _T("Playing style out of range (0-21); ");
			}
		}

		/* REGULAR */
		if (rating < silverRate - silverGiantPen) //Regular player
		{
			numReg++;
			targetRate = regRate;
			targetRate2 = regRate;
			targetRate3 = regRate;
			if (player.reg_pos == 0) //GK target rate is 77
			{
				targetRate = gkRate;
				targetRate2 = gkRate;
				targetRate3 = gkRate;
			}

			weakFootUse = regWeakFootUse;
			weakFootAcc = regWeakFootAcc;

			/*if(countA > 3)
			{
				errorTot++;
				errorMsg << _T("Regular player with > 3 A positions; ");
			}*/

			/*//SPECIAL Summer 24: Malicia (21) is now mandatory on all non-medal players
			if (!player.play_skill[21])
			{
				errorTot++;
				errorMsg << _T("Malicia card is mandatory for all non-medal players; ");
			}*/

			if (player.form + 1 != regForm)
			{
				errorTot++;
				errorMsg << _T("Form is ") << player.form + 1 << _T(", should be ") << regForm << _T("; ");
			}

			if (player.reg_pos == 0) //GK gets 2 cards
			{
				cardMod += min(gkTrickCards, numTrick); //1 free tricks
				cardLimit = gkSkillCards + cardMod;

				if (player.height > heightMid && player.height < heightTallGK)
				{
					errorTot++;
					errorMsg << _T("GKs in this bracket must be ") << heightTallGK << _T("cm; ");
				}
				//SPECIAL Autumn 24 - GK and medals can'ts be in giant height bracket
				if (player.height >= heightGiant)
				{
					errorMsg << _T("GK heights cannot be ") << heightGiant << _T("cm; ");
				}
			}
			else
			{
				cardMod += min(regCOM, numCom); //0 free COM styles
				cardMod += min(regTrickCards, numTrick); //2 free tricks
				//cardMod += min(1, (countA - 1)); //1 free A-position
				cardLimit = regSkillCards + cardMod; //3 skill cards
			}

			if (player.injury + 1 > regIR)
			{
				errorTot++;
				errorMsg << _T("Injury resist is ") << player.injury + 1 << _T(", should be ") << regIR << _T("; ");
			}

			if (player.height <= heightManlet && usingRed)
			{
				targetRate += manletBonus;
				targetRate2 += manletBonus;
				targetRate3 += manletBonus;
			}

			/*//SPECIAL Spring 24: non-medals that are registered in a blue position (CB, LB, RB) get +5 defensive prowess
			if (player.reg_pos >= 1 && player.reg_pos <= 3)
			{
				targetRate2 += 5;
			}*/

			/*//SPECIAL Spring 24: red heights non-medals registered in a red position (CF/SS/LWF/RWF) receive a +5 boost to all stats and can stack with the boost from being 175cm however these players cannot be given a 2nd A position
			if (player.reg_pos >= 9 && player.reg_pos <= 12 && usingRed)
			{
				targetRate += 5;
				targetRate2 += 5;
				//SPECIAL Summer 24: Red Position non-medal manlet players (CF/SS/LWF/RWF) have stamina stat = 77
				if (player.height <= heightManlet && usingRed)
					targetRate3 = regRate;
				else
					targetRate3 += 5;
				if (countA > 1)
				{
					errorTot++;
					errorMsg << _T("Illegal 2nd A position on red heights non-medal forward; ");
				}
			}*/

			/*//SPECIAL Summer 24: green heights non-medals registered in a red position (CF/SS/LWF/RWF) get an additional 5cm of height except for 199cm players
			if (player.reg_pos >= 9 && player.reg_pos <= 12 && !usingRed && player.height <= heightGiant)
			{
				heightMod = 5;
			}*/

			if (eCheck)
			{
				if (player.reg_pos == 0)
				{
					if (numTrick < gkTrickCards) errorMsg << _T("WARN: Has ") << numTrick << _T(" trick cards, allowed ") << gkTrickCards << _T("; ");
					if (numCom < regCOM) errorMsg << _T("WARN: Has ") << numCom << _T(" COM cards, allowed ") << regCOM << _T("; ");
				}
				else
				{
					if (numTrick < regTrickCards) errorMsg << _T("WARN: Has ") << numTrick << _T(" trick cards, allowed ") << regTrickCards << _T("; ");
					if (numCom < regCOM) errorMsg << _T("WARN: Has ") << numCom << _T(" COM cards, allowed ") << regCOM << _T("; ");
				}
				if (player.injury + 1 < regIR) errorMsg << _T("WARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << regIR << _T("; ");
			}
		}
		/* SILVER */
		else if (rating < goldRate - goldGiantPen) //Silver player
		{
			numSilver++;
			targetRate = silverRate;
			targetRate2 = silverRate;
			targetRate3 = silverRate;

			weakFootUse = silverWeakFootUse;
			weakFootAcc = silverWeakFootAcc;

			if (numSilver > reqNumSilver)
			{
				errorTot++;
				errorMsg << _T("Too many Silver medals; ");
			}
			if (player.form + 1 != silverForm)
			{
				errorTot++;
				errorMsg << _T("Form is ") << player.form + 1 << _T(", should be ") << silverForm << _T("; ");
			}
			if (player.reg_pos == 0) //Medals can't be GK
			{
				errorTot++;
				errorMsg << _T("Medals cannot play as GK; ");
			}
			if (player.height >= heightGiant) //HA get penalty
			{
				targetRate -= silverGiantPen;
				targetRate2 -= silverGiantPen;
				targetRate3 -= silverGiantPen;
			}
			else if (player.height <= heightManlet && usingRed)
			{
				targetRate += silverManletBonus;
				targetRate2 += silverManletBonus;
				targetRate3 += silverManletBonus;
			}
			cardMod += min(silverTrickCards, numTrick); //3 free tricks
			cardMod += min(silverCOM, numCom); //1 free COM
			//cardMod += min(1, (countA - 1)); //1 free A-position
			cardLimit = silverSkillCards + cardMod; //4 skill cards

			if (player.injury + 1 > silverIR)
			{
				errorTot++;
				errorMsg << _T("Injury resist is ") << player.injury + 1 << _T(", should be ") << silverIR << _T("; ");
			}

			if (eCheck)
			{
				if (numTrick < silverTrickCards) errorMsg << _T("WARN: Has ") << numTrick << _T(" trick cards, allowed ") << silverTrickCards << _T("; ");
				if (numCom < silverCOM) errorMsg << _T("WARN: Has ") << numCom << _T(" COM cards, allowed ") << silverCOM << _T("; ");
				if (player.injury + 1 < silverIR) errorMsg << _T("WARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << silverIR << _T("; ");
			}

			//SPECIAL FAG13: Medals can trade a card for 4/4 footedness
			/*if (player.weak_use + 1 > weakFootUse)
			{
				weakFootUse = 4;
				cardLimit--;
			}*/
		}
		/* GOLD */
		else //rating == 99 //Gold player
		{
			numGold++;
			targetRate = goldRate;
			targetRate2 = goldRate;
			targetRate3 = goldRate;

			weakFootUse = goldWeakFootUse;
			weakFootAcc = goldWeakFootAcc;

			if (numGold > reqNumGold)
			{
				errorTot++;
				errorMsg << _T("Too many Gold medals; ");
			}
			if (player.form + 1 != goldForm)
			{
				errorTot++;
				errorMsg << _T("Form is ") << player.form + 1 << _T(", should be ") << goldForm << _T("; ");
			}
			if (player.reg_pos == 0) //Medals can't be GK
			{
				errorTot++;
				errorMsg << _T("Medals cannot play as GK; ");
			}
			if (player.height >= heightGiant) //Medal HA penalty
			{
				targetRate -= goldGiantPen;
				targetRate2 -= goldGiantPen;
				targetRate3 -= goldGiantPen;
			}
			else if (player.height <= heightManlet && usingRed)
			{
				targetRate += goldManletBonus;
				targetRate2 += goldManletBonus;
				targetRate3 += goldManletBonus;
			}

			if (player.height > heightGiant)
			{
				errorMsg << _T("Gold heights cannot exceed ") << heightGiant << _T("cm; ");
			}

			cardMod += min(goldTrickCards, numTrick); //4 free tricks
			cardMod += min(goldCOM, numCom); //2 free COMs
			cardLimit = goldSkillCards + cardMod; //5 skill cards

			if (player.injury + 1 > goldIR)
			{
				errorTot++;
				errorMsg << _T("Injury resist is ") << player.injury + 1 << _T(", should be ") << goldIR << _T("; ");
			}

			if (eCheck)
			{
				if (cardCount < 10)
				{
					if (numTrick < goldTrickCards) errorMsg << _T("WARN: Has ") << numTrick << _T(" trick cards, allowed ") << goldTrickCards << _T("; ");
					if (numCom < goldCOM) errorMsg << _T("WARN: Has ") << numCom << _T(" COM cards, allowed ") << goldCOM << _T("; ");
				}
				if (player.injury + 1 < goldIR) errorMsg << _T("WARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << goldIR << _T("; ");
			}

			//SPECIAL Spring 25 - GK and gold medals can't be in giant height bracket
			if (player.height >= heightGiant)
			{
				errorMsg << _T("Gold medal heights cannot be ") << heightGiant << _T("cm; ");
			}

			//SPECIAL FAG13: Medals can trade a card for 4/4 footedness
			/*if (player.weak_use + 1 > weakFootUse)
			{
				weakFootUse = 4;
				cardLimit--;
			}*/
		}

		//Check player height
		if (((player.height - heightMod) <= heightManlet))
		{
			numManlet++;
			cardLimit += manletCardBonus; //Manlets get a bonus card
			if (countA > 1) cardLimit += manletPosBonus; //Manlets get a bonus double A position
			weakFootUse = manletWeakFootUse; //Manlets get weak foot acc/use 4/4
			weakFootAcc = manletWeakFootAcc;
		}
		else if ((player.height - heightMod) <= heightMid)
		{
			numMid++;
		}
		else if ((player.height - heightMod) == heightTall)
			numTall++;
		else if ((player.height - heightMod) == heightTallGK && player.reg_pos == 0) //GK
			numTall++;
		else if ((player.height - heightMod) == heightGiant)
			numGiant++;
		else if ((player.height - heightMod) == heightGiga)
			numGiga++;
		else
		{
			errorTot++;
			errorMsg << _T("Illegal height (") << player.height << _T(" cm); ");
		}

		//Check weak foot ratings
		if (player.weak_use + 1 > weakFootUse)
		{
			errorTot++;
			errorMsg << _T("Weak foot usage > ") << weakFootUse << _T("; ");
		}
		if (player.weak_acc + 1 > weakFootAcc)
		{
			errorTot++;
			errorMsg << _T("Weak foot accuracy > ") << weakFootAcc << _T("; ");
		}

		//Check player card count
		if (cardCount > cardLimit)
		{
			errorTot++;
			errorMsg << _T("Has ") << cardCount << _T(" cards, only allowed ") << cardLimit << _T("; ");
		}

		//Check PES skill card limit of 10
		if (cardCount - numCom > 10)
		{
			errorTot++;
			errorMsg << _T("Has ") << cardCount - numCom << _T(" skill cards, PES limit is 10, please swap to COM cards or trade for additional A positions; ");
		}

		if (eCheck)
		{
			if (cardCount < min(cardLimit, 10)) errorMsg << _T("WARN: Has ") << cardCount << _T(" cards, allowed ") << cardLimit << _T("; ");
			if (player.weak_use + 1 < weakFootUse) errorMsg << _T("WARN: Has weak usage ") << player.weak_use + 1 << _T(", allowed ") << weakFootUse << _T("; ");
			if (player.weak_acc + 1 < weakFootAcc) errorMsg << _T("WARN: Has weak accuracy ") << player.weak_acc + 1 << _T(", allowed ") << weakFootAcc << _T("; ");
		}

		//Check player overall rating
		if (rating != targetRate)
		{
			errorTot++;
			errorMsg << _T("Illegal Ability scores; ");
		}

		//Check individual skill ratings
		//								c_skillRate			s_skillName			n_minPesVersion	n_targetRate
		skillCheck skillChecks[25] = { {player.drib,		_T("Dribbling"),			0,		targetRate	},
										{player.gk,			_T("Goalkeeping"),			0,		targetRate	},
										{player.finish,		_T("Finishing"),			0,		targetRate	},
										{player.lowpass,	_T("Low Pass"),				0,		targetRate	},
										{player.loftpass,	_T("Lofted Pass"),			0,		targetRate	},
										{player.header,		_T("Header"),				0,		targetRate	},
										{player.swerve,		_T("Swerve"),				0,		targetRate	},
										{player.catching,	_T("Catching"),				0,		targetRate	},
										{player.clearing,	_T("Clearing"),				16,		targetRate	},
										{player.reflex,		_T("Reflexes"),				16,		targetRate	},
										{player.body_ctrl,	_T("Body Control"),			0,		targetRate	},
										{player.phys_cont,	_T("Physical Contact"),		17,		targetRate	},
										{player.kick_pwr,	_T("Kicking Power"),		0,		targetRate	},
										{player.exp_pwr,	_T("Explosive Power"),		0,		targetRate	},
										{player.ball_ctrl,	_T("Ball Control"),			0,		targetRate	},
										{player.ball_win,	_T("Ball Winning"),			0,		targetRate	},
										{player.jump,		_T("Jump"),					0,		targetRate	},
										{player.cover,		_T("Coverage"),				16,		targetRate	},
										{player.place_kick, _T("Place Kicking"),		0,		targetRate	},
										{player.stamina,	_T("Stamina"),				0,		targetRate3	},
										{player.speed,		_T("Speed"),				0,		targetRate	},
										{player.atk,		_T("Attacking Prowess"),	0,		targetRate	},
										{player.def,		_T("Defensive Prowess"),	0,		targetRate2	},
										{player.tight_pos,	_T("Tight Possession"),		20,		targetRate	},
										{player.aggres,		_T("Aggression"),			20,		targetRate	} };

		for (int ii = 0; ii < 25; ii++)
		{
			//atk and def can be lower than the target rate
			if (skillChecks[ii].s_skillName == _T("Attacking Prowess") || skillChecks[ii].s_skillName == _T("Defensive Prowess"))
			{
				if (skillChecks[ii].n_minPesVersion <= pesVersion && skillChecks[ii].c_skillRate > skillChecks[ii].n_targetRate)
				{
					errorTot++;
					errorMsg << skillChecks[ii].s_skillName << _T(" is ") << skillChecks[ii].c_skillRate << _T(", should be <= ") << skillChecks[ii].n_targetRate << _T("; ");
				}
			}
			else if (skillChecks[ii].n_minPesVersion <= pesVersion &&
				skillChecks[ii].c_skillRate != skillChecks[ii].n_targetRate)
			{
				errorTot++;
				errorMsg << skillChecks[ii].s_skillName << _T(" is ") << skillChecks[ii].c_skillRate << _T(", should be ") << skillChecks[ii].n_targetRate << _T("; ");
			}
		}

		if (errorMsg.rdbuf()->in_avail())
		{
			errorMsg << _T("\r\n");
			msgOut += _T("\t");
			msgOut += errorMsg.str();
		}
	}
	//Team level errors
	int diff;
	tstringstream errorMsg;

	if (!hasCaptain)
	{
		errorTot++;
		errorMsg << _T("Team must have an assigned Captain; ");
	}

	//Must have at least 1 GK
	if (numGK < 1)
	{
		errorTot++;
		errorMsg << _T("Team must have a registered GK; ");
	}

	//Check heights
	if (!usingRed) //Using Green height system
	{
		msgOut += _T("Using Green height system\r\n");
		if (diff = greenGiga - numGiga)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("Has ") << numGiga << _T("/") << greenGiga << _T(" ") << heightGiga << _T("cm players; ");
		}
		if (diff = greenGiant - numGiant)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("Has ") << numGiant << _T("/") << greenGiant << _T(" ") << heightGiant << _T("cm players; ");
		}
		if (diff = greenTall - numTall)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("Has ") << numTall << _T("/") << greenTall << _T(" ") << heightTall << _T("/") << heightTallGK << _T("cm players; ");
		}
		if (diff = greenMid - numMid)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("Has ") << numMid << _T("/") << greenMid << _T(" ") << heightMid << _T("cm players; ");
		}
		if (diff = greenManlet - numManlet)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("Has ") << numManlet << _T("/") << greenManlet << _T(" ") << heightManlet << _T("cm players; ");
		}
	}
	else //Using Red height system
	{
		msgOut += _T("Using Red height system\r\n");
		if (diff = numGiga)
		{
			errorTot += diff;
			errorMsg << _T("Has ") << numGiga << _T("/") << redGiga << _T(" ") << heightGiga << _T("cm players; ");
		}
		if (diff = numGiant)
		{
			errorTot += diff;
			errorMsg << _T("Has ") << numGiant << _T("/") << redGiant << _T(" ") << heightGiant << _T("cm players; ");
		}
		if (diff = redTall - numTall)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("Has ") << numTall << _T("/") << redTall << _T(" ") << heightTall << _T("/") << heightTallGK << _T("cm players; ");
		}
		if (diff = redMid - numMid)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("Has ") << numMid << _T("/") << redMid << _T(" ") << heightMid << _T("cm players; ");
		}
		if (diff = redManlet - numManlet)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("Has ") << numManlet << _T("/") << redManlet << _T(" ") << heightManlet << _T("cm players; ");
		}
	}
	if (errorMsg.rdbuf()->in_avail())
	{
		errorMsg << _T("\r\n");
		msgOut += errorMsg.str();
		errorMsg.clear();
		errorMsg.str(tstring());
	}

	//Check ability stats
	if (numReg != (23 - reqNumSilver - reqNumGold))
	{
		errorTot++;
		errorMsg << _T("Number of Regular players is ") << numReg << _T(", should be ") << 23 - reqNumSilver - reqNumGold << _T("; ");
	}
	if (numSilver != reqNumSilver)
	{
		errorTot++;
		errorMsg << _T("Number of Silver medals is ") << numSilver << _T(", should be ") << reqNumSilver << _T("; ");
	}
	if (numGold != reqNumGold)
	{
		errorTot++;
		errorMsg << _T("Number of Gold medals is ") << numGold << _T(", should be ") << reqNumGold << _T("; ");
	}
	if (errorMsg.rdbuf()->in_avail())
		errorMsg << _T("\r\n");
	errorMsg << _T("\r\nErrors: ") << errorTot << _T("\r\n");
	msgOut += errorMsg.str();

	SetWindowText(GetDlgItem(hAatfbox, IDT_AATFOUT), msgOut.c_str());
	if (errorTot)
		SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM)_T("KWABxport melty in the 'tor"));
	else
		SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM)_T("Well, Seymour, this export made it... despite your directions"));
}

void aatf_single_vgl(HWND hAatfbox, int pesVersion, int teamSel, player_entry* gplayers, team_entry* gteams, int gnum_players, bool useSuggestions)
{
	player_entry player;
	tstring msgOut;
	msgOut += _T("Team: ");
	msgOut += gteams[teamSel].name;
	msgOut += _T("\r\n");

	//============================

	bool hasCaptain = false;
	bool captainHasCaptaincy = false;
	int numGK = 0;
	//Count of player ratings
	int numReg = 0;
	int numBronze = 0;
	int numSilver = 0;
	int numGold = 0;
	//Count of height brackets
	int numGiga = 0;
	int numGiant = 0;
	int numTall = 0;
	int numMid = 0;
	int numManlet = 0;

	int maxCardMod = 0;
	int maxAPosMod = 0;
	//bool usingRed = true;

	//Run through all players once to determine height system
	for (int ii = 0; ii < gteams[teamSel].num_on_team; ii++)
	{
		//Find each player on team
		for (int jj = 0; jj < gnum_players; jj++)
		{
			if (gplayers[jj].id == gteams[teamSel].players[ii])
			{
				player = gplayers[jj];
				break;
			}
		}
		//If any player is in the green height brackets, team is on Green height system and we can halt
		/*if (player.height >= heightGiant)
		{
			usingRed = false;
			break;
		}*/
	}

	//Now check each player for errors
	int errorTot = 0;
	for (int ii = 0; ii < gteams[teamSel].num_on_team; ii++)
	{
		//Find each player on team
		for (int jj = 0; jj < gnum_players; jj++)
		{
			if (gplayers[jj].id == gteams[teamSel].players[ii])
			{
				player = gplayers[jj];
				break;
			}
		}

		msgOut += _T("\x2022 Checking ");
		msgOut += player.name;
		msgOut += _T("\r\n");

		tstringstream errorMsg;

		int cardCount = 0;
		int cardMod = 0;
		int cardLimit = 0;
		int heightMod = 0;
		int weakFootUse = 0;
		int weakFootAcc = 0;
		bool hasTrick = false;
		// Individual Stat Comparison
		int targetRate = 0;
		int targetDrib = 0;
		int targetGk = 0;
		int targetFinish = 0;
		int targetLowPass = 0;
		int targetLoftPass = 0;
		int targetHeader = 0;
		int targetSwerve = 0;
		int targetCatching = 0;
		int targetClearing = 0;
		int targetReflex = 0;
		int targetBodyCtrl = 0;
		int targetPhysCont = 0;
		int targetKickPwr = 0;
		int targetExpPwr = 0;
		int targetBallCtrl = 0;
		int targetBallWin = 0;
		int targetJump = 0;
		int targetCover = 0;
		int targetPlcKick = 0;
		int targetStamina = 0;
		int targetSpeed = 0;
		int targetAtkProw = 0;
		int targetDefProw = 0;
		int targetTightPos = 0;
		int targetAggres = 0;
		// Height and A positions
		//int allowedHeight = 0;
		int allowedAPostions = 0;
		int allowedAPostionsMod = 0;
		// Rating Calculation
		int rating = player.drib;
		rating = max(player.gk, rating);
		//rating = max(player.finish, rating);
		rating = max(player.lowpass, rating);
		rating = max(player.loftpass, rating);
		//rating = max(player.header, rating);
		rating = max(player.swerve, rating);
		rating = max(player.catching, rating);
		if (pesVersion > 15)
		{
			rating = max(player.clearing, rating);
			rating = max(player.reflex, rating);
			rating = max(player.cover, rating);
		}
		rating = max(player.body_ctrl, rating);
		if (pesVersion > 16) rating = max(player.phys_cont, rating); //Not in 16
		rating = max(player.kick_pwr, rating);
		rating = max(player.exp_pwr, rating);
		rating = max(player.ball_ctrl, rating);
		rating = max(player.ball_win, rating);
		rating = max(player.jump, rating);
		rating = max(player.place_kick, rating);
		rating = max(player.stamina, rating);
		rating = max(player.speed, rating);
		//if (pesVersion > 19) rating = max(player.aggres, rating);

		/*if(player.injury+1 > 3)
		{
			errorTot++;
			errorMsg << _T("Injury resist is ") << player.injury+1 << _T(", cannot exceed 3; ");
		}*/

		//Check if this player is the captain
		if (player.id == gteams[teamSel].players[gteams[teamSel].captain_ind]) hasCaptain = true;

		//Check if registered pos has playable set to A
		int requiredAPos = regPosToPlayPosMap[player.reg_pos];
		if (player.play_pos[requiredAPos] != 2)
		{
			errorTot++;
			errorMsg << _T("\tDoesn't have A in registered position;\r\n");
		}

		//Count number of registered GKs
		if (player.reg_pos == 0) numGK++;

		//Count A positions
		int countA = 0;
		int countB = 0;
		for (int jj = 0; jj < 13; jj++)
		{
			if (player.play_pos[jj] == 2)
				countA++;
			else if (player.play_pos[jj] == 1)
				countB++;
		}

		//No B positions allowed:
		if (countB > 0)
		{
			errorTot++;
			errorMsg << _T("\tHas B position;\r\n");
		}

		if (player.reg_pos != 0 && player.play_pos[12] == 2) //Can't have GK as second A
		{
			errorTot++;
			errorMsg << _T("\tHas GK as second A position;\r\n");
		}

		//Count cards
		int numTrick = 0;
		int numCom = 0;
		int numSkill;
		if (pesVersion == 19) numSkill = 39;
		else if (pesVersion > 19) numSkill = 41;
		else numSkill = 28;
		for (int jj = 0; jj < numSkill; jj++)
		{
			if (player.play_skill[jj])
			{
				cardCount++;
				//Captain gets free captaincy card
				if (jj == 25 && player.id == gteams[teamSel].players[gteams[teamSel].captain_ind])
				{
					captainHasCaptaincy = true;
					cardMod++;
				}
				//Trick cards may be free, count number
				if (jj < 6 || jj == 16 || jj == 21 || jj == 28 || jj == 29 || jj == 30 || jj == 34)
				{
					hasTrick = true;
					numTrick++;
				}
			}
		}

		//Captain gets a free regular card
		if (player.id == gteams[teamSel].players[gteams[teamSel].captain_ind]) {
			cardMod++;
		}

		for (int jj = 0; jj < 7; jj++)
		{
			if (player.com_style[jj])
			{
				cardCount++;
				numCom++;
			}
		}

		if (player.age < 15 || player.age>50)
		{
			errorTot++;
			errorMsg << _T("\tAge out of range (15,50);\r\n");
		}

		if (player.weight<max(30, player.height - 129) || player.weight>(player.height - 81))
		{
			errorTot++;
			errorMsg << _T("\tWeight out of range (") << max(30, player.height - 129) << _T(",") << player.height - 81 << _T(");\r\n");
		}

		//Check playing style and registered position are in valid range per PES version
		if (player.reg_pos > 12)
		{
			errorTot++;
			errorMsg << _T("\tRegistered position out of range (0-12);\r\n");
		}

		if (pesVersion <= 16)
		{
			if (player.play_style > 18 || player.play_style == 16)
			{
				errorTot++;
				errorMsg << _T("\tPlaying style out of range (0-18, excluding 16);\r\n");
			}
		}
		else if (pesVersion > 16 && pesVersion < 19)
		{
			if (player.play_style > 17)
			{
				errorTot++;
				errorMsg << _T("\tPlaying style out of range (0-17);\r\n");
			}
		}
		else
		{
			if (player.play_style > 21)
			{
				errorTot++;
				errorMsg << _T("\tPlaying style out of range (0-21);\r\n");
			}
		}

		if (player.height == globalStats::heightColossal) {
			numGiga++;
			cardMod += globalStats::colossal_card_mod;
			allowedAPostionsMod += globalStats::colossal_apos_mod;
			weakFootAcc = globalStats::colossal_foot_acc == 0 ? weakFootAcc : globalStats::colossal_foot_acc;
			weakFootUse = globalStats::colossal_foot_usage == 0 ? weakFootUse : globalStats::colossal_foot_usage;
		}
		else if (player.height == globalStats::heightGiant) {
			numGiant++;
			cardMod += globalStats::giant_card_mod;
			allowedAPostionsMod += globalStats::giant_apos_mod;
			weakFootAcc = globalStats::giant_foot_acc == 0 ? weakFootAcc : globalStats::giant_foot_acc;
			weakFootUse = globalStats::giant_foot_usage == 0 ? weakFootUse : globalStats::giant_foot_usage;
		}
		else if (player.height == globalStats::heightTall || player.reg_pos == 0 && player.height == globalStats::gk_height_override) {
			numTall++;
			cardMod += globalStats::tall_card_mod;
			allowedAPostionsMod += globalStats::tall_apos_mod;
			weakFootAcc = globalStats::tall_foot_acc == 0 ? weakFootAcc : globalStats::tall_foot_acc;
			weakFootUse = globalStats::tall_foot_usage == 0 ? weakFootUse : globalStats::tall_foot_usage;
		}
		else if (player.height == globalStats::heightMid) {
			numMid++;
			cardMod += globalStats::mid_card_mod;
			allowedAPostionsMod += globalStats::mid_apos_mod;
			weakFootAcc = globalStats::mid_foot_acc == 0 ? weakFootAcc : globalStats::mid_foot_acc;
			weakFootUse = globalStats::mid_foot_usage == 0 ? weakFootUse : globalStats::mid_foot_usage;
		}
		else if (player.height == globalStats::heightManlet) {
			numManlet++;
			cardMod += globalStats::manlet_card_mod;
			allowedAPostionsMod += globalStats::manlet_apos_mod;
			weakFootAcc = globalStats::manlet_foot_acc == 0 ? weakFootAcc : globalStats::manlet_foot_acc;
			weakFootUse = globalStats::manlet_foot_usage == 0 ? weakFootUse : globalStats::manlet_foot_usage;
		}
		else {
			errorTot++;
			errorMsg << _T("Illegal height (") << player.height << _T(" cm); ");
		}

		/* GOALKEEPER */
		if (player.reg_pos == 0) //Goalkeeper player, counts towards regular
		{
			numReg++;
			using namespace goalkeeper;
			targetRate = base_stat;
			targetDrib = dribbling;
			targetGk = gk_awareness;
			targetFinish = finishing;
			targetLowPass = low_pass;
			targetLoftPass = lofted_pass;
			targetHeader = header;
			targetSwerve = curl;
			targetCatching = catching;
			targetClearing = clearing;
			targetReflex = reflexes;
			targetBodyCtrl = balance;
			targetPhysCont = physical_contact;
			targetKickPwr = kicking_power;
			targetExpPwr = acceleration;
			targetBallCtrl = ball_control;
			targetBallWin = ball_winning;
			targetJump = jump;
			targetCover = gk_reach;
			targetPlcKick = place_kicking;
			targetStamina = stamina;
			targetSpeed = speed;
			targetAtkProw = offensive_awareness;
			targetDefProw = defensive_awareness;
			targetTightPos = tight_possession;
			targetAggres = aggression;

			//allowedHeight = globalStats::gk_height_override == 0 ? globalStats::heightTall : globalStats::gk_height_override;
			allowedAPostions = a_pos;

			if (weakFootAcc == 0) weakFootAcc = weak_foot_accuracy;
			if (weakFootUse == 0) weakFootUse = weak_foot_usage;

			if (player.form + 1 != form)
			{
				errorTot++;
				errorMsg << _T("\tForm is ") << player.form + 1 << _T(", should be ") << form << _T(";\r\n");
			}

			cardMod += min(coms, numCom); //1 free COM styles
			cardMod += min(tricks, numTrick); //12 free tricks (aka all of them)
			cardLimit = skills + cardMod; //5 skill cards

			if (player.injury + 1 > injury_resistance)
			{
				errorTot++;
				errorMsg << _T("\tInjury resist is ") << player.injury + 1 << _T(", should be ") << injury_resistance << _T(";\r\n");
			}
			if (useSuggestions)
			{
				//if (numTrick < tricks) errorMsg << _T("\tWARN: Has ") << numTrick << _T(" trick cards, allowed ") << tricks << _T(";\r\n");
				if (numCom < coms) errorMsg << _T("\tWARN: Has ") << numCom << _T(" COM cards, allowed ") << coms << _T(";\r\n");
				if (player.injury + 1 < injury_resistance) errorMsg << _T("\tWARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << injury_resistance << _T(";\r\n");
			}
		}
		/* REGULAR */
		else if (rating < bronze::base_stat) //Regular player
		{
			numReg++;
			using namespace regular;
			targetRate = base_stat;
			targetDrib = dribbling;
			targetGk = gk_awareness;
			targetFinish = finishing;
			targetLowPass = low_pass;
			targetLoftPass = lofted_pass;
			targetHeader = header;
			targetSwerve = curl;
			targetCatching = catching;
			targetClearing = clearing;
			targetReflex = reflexes;
			targetBodyCtrl = balance;
			targetPhysCont = physical_contact;
			targetKickPwr = kicking_power;
			targetExpPwr = acceleration;
			targetBallCtrl = ball_control;
			targetBallWin = ball_winning;
			targetJump = jump;
			targetCover = gk_reach;
			targetPlcKick = place_kicking;
			targetStamina = stamina;
			targetSpeed = speed;
			targetAtkProw = offensive_awareness;
			targetDefProw = defensive_awareness;
			targetTightPos = tight_possession;
			targetAggres = aggression;

			//allowedHeight = height;
			allowedAPostions = a_pos;

			if (weakFootAcc == 0) weakFootAcc = weak_foot_accuracy;
			if (weakFootUse == 0) weakFootUse = weak_foot_usage;

			if (player.form + 1 != form)
			{
				errorTot++;
				errorMsg << _T("\tForm is ") << player.form + 1 << _T(", should be ") << form << _T(";\r\n");
			}

			cardMod += min(coms, numCom); //1 free COM styles
			cardMod += min(tricks, numTrick); //12 free tricks (aka all of them)
			cardLimit = skills + cardMod;

			if (player.injury + 1 > injury_resistance)
			{
				errorTot++;
				errorMsg << _T("\tInjury resist is ") << player.injury + 1 << _T(", should be ") << injury_resistance << _T(";\r\n");
			}
			if (useSuggestions)
			{	
				//if (numTrick < tricks) errorMsg << _T("\tWARN: Has ") << numTrick << _T(" trick cards, allowed ") << tricks << _T(";\r\n");
				if (numCom < coms) errorMsg << _T("\tWARN: Has ") << numCom << _T(" COM cards, allowed ") << coms << _T(";\r\n");
				if (player.injury + 1 < injury_resistance) errorMsg << _T("\tWARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << injury_resistance << _T(";\r\n");
			}
		}
		/* Bronze */
		else if (rating == bronze::base_stat) //Bronze player
		{
			numBronze++;
			using namespace bronze;
			targetRate = base_stat;
			targetDrib = dribbling;
			targetGk = gk_awareness;
			targetFinish = finishing;
			targetLowPass = low_pass;
			targetLoftPass = lofted_pass;
			targetHeader = header;
			targetSwerve = curl;
			targetCatching = catching;
			targetClearing = clearing;
			targetReflex = reflexes;
			targetBodyCtrl = balance;
			targetPhysCont = physical_contact;
			targetKickPwr = kicking_power;
			targetExpPwr = acceleration;
			targetBallCtrl = ball_control;
			targetBallWin = ball_winning;
			targetJump = jump;
			targetCover = gk_reach;
			targetPlcKick = place_kicking;
			targetStamina = stamina;
			targetSpeed = speed;
			targetAtkProw = offensive_awareness;
			targetDefProw = defensive_awareness;
			targetTightPos = tight_possession;
			targetAggres = aggression;

			//allowedHeight = height;
			allowedAPostions = a_pos;

			if (weakFootAcc == 0) weakFootAcc = weak_foot_accuracy;
			if (weakFootUse == 0) weakFootUse = weak_foot_usage;

			if (numBronze > globalStats::allowedBronze)
			{
				errorTot++;
				errorMsg << _T("\tToo many Bronze medals;");
			}
			if (player.form + 1 != form)
			{
				errorTot++;
				errorMsg << _T("\tForm is ") << player.form + 1 << _T(", should be ") << form << _T(";\r\n");
			}
			if (player.reg_pos == 0) //Medals can't be GK
			{
				errorTot++;
				errorMsg << _T("\tMedals cannot play as GK;\r\n");
			}

			cardMod += min(tricks, numTrick); //12 free tricks (aka all of them)
			cardMod += min(coms, numCom); //2 free COM
			cardLimit = skills + cardMod; //8 skill cards

			if (player.injury + 1 != injury_resistance)
			{
				errorTot++;
				errorMsg << _T("\tInjury resist is ") << player.injury + 1 << _T(", should be ") << injury_resistance << _T(";\r\n");
			}
			if (useSuggestions)
			{
				//if (numTrick < tricks) errorMsg << _T("WARN: Has ") << numTrick << _T(" trick cards, allowed ") << tricks << _T(";\r\n");
				if (numCom < coms) errorMsg << _T("\tWARN: Has ") << numCom << _T(" COM cards, allowed ") << coms << _T(";\r\n");
				if (player.injury + 1 < injury_resistance) errorMsg << _T("\tWARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << injury_resistance << _T(";\r\n");
			}
		}
		/* SILVER */
		else if (rating == silver::base_stat) //Silver player
		{
			numSilver++;
			using namespace silver;
			targetRate = base_stat;
			targetDrib = dribbling;
			targetGk = gk_awareness;
			targetFinish = finishing;
			targetLowPass = low_pass;
			targetLoftPass = lofted_pass;
			targetHeader = header;
			targetSwerve = curl;
			targetCatching = catching;
			targetClearing = clearing;
			targetReflex = reflexes;
			targetBodyCtrl = balance;
			targetPhysCont = physical_contact;
			targetKickPwr = kicking_power;
			targetExpPwr = acceleration;
			targetBallCtrl = ball_control;
			targetBallWin = ball_winning;
			targetJump = jump;
			targetCover = gk_reach;
			targetPlcKick = place_kicking;
			targetStamina = stamina;
			targetSpeed = speed;
			targetAtkProw = offensive_awareness;
			targetDefProw = defensive_awareness;
			targetTightPos = tight_possession;
			targetAggres = aggression;

			//allowedHeight = height;
			allowedAPostions = a_pos;

			if (weakFootAcc == 0) weakFootAcc = weak_foot_accuracy;
			if (weakFootUse == 0) weakFootUse = weak_foot_usage;

			if (numSilver > globalStats::allowedSilver)
			{
				errorTot++;
				errorMsg << _T("\tToo many Silver medals;\r\n");
			}
			if (player.form + 1 != form)
			{
				errorTot++;
				errorMsg << _T("\tForm is ") << player.form + 1 << _T(", should be ") << form << _T(";\r\n");
			}
			if (player.reg_pos == 0) //Medals can't be GK
			{
				errorTot++;
				errorMsg << _T("\tMedals cannot play as GK;\r\n");
			}

			cardMod += min(tricks, numTrick); //12 free tricks (aka all of them)
			cardMod += min(coms, numCom); //2 free COM
			cardLimit = skills + cardMod; //8 skill cards

			if (player.injury + 1 != injury_resistance)
			{
				errorTot++;
				errorMsg << _T("\tInjury resist is ") << player.injury + 1 << _T(", should be ") << injury_resistance << _T(";\r\n");
			}
			if (useSuggestions)
			{
				//if (numTrick < tricks) errorMsg << _T("\tWARN: Has ") << numTrick << _T(" trick cards, allowed ") << tricks << _T(";\r\n");
				if (numCom < coms) errorMsg << _T("\tWARN: Has ") << numCom << _T(" COM cards, allowed ") << coms << _T(";\r\n");
				if (player.injury + 1 < injury_resistance) errorMsg << _T("\tWARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << injury_resistance << _T(";\r\n");
			}
		}
		/* GOLD */
		else if (rating == gold::base_stat) //Gold player
		{
			numGold++;
			using namespace gold;
			targetRate = base_stat;
			targetDrib = dribbling;
			targetGk = gk_awareness;
			targetFinish = finishing;
			targetLowPass = low_pass;
			targetLoftPass = lofted_pass;
			targetHeader = header;
			targetSwerve = curl;
			targetCatching = catching;
			targetClearing = clearing;
			targetReflex = reflexes;
			targetBodyCtrl = balance;
			targetPhysCont = physical_contact;
			targetKickPwr = kicking_power;
			targetExpPwr = acceleration;
			targetBallCtrl = ball_control;
			targetBallWin = ball_winning;
			targetJump = jump;
			targetCover = gk_reach;
			targetPlcKick = place_kicking;
			targetStamina = stamina;
			targetSpeed = speed;
			targetAtkProw = offensive_awareness;
			targetDefProw = defensive_awareness;
			targetTightPos = tight_possession;
			targetAggres = aggression;

			//allowedHeight = height;
			allowedAPostions = a_pos;

			if (weakFootAcc == 0) weakFootAcc = weak_foot_accuracy;
			if (weakFootUse == 0) weakFootUse = weak_foot_usage;

			if (numGold > globalStats::allowedGold)
			{
				errorTot++;
				errorMsg << _T("\tToo many Gold medals;\r\n");
			}
			if (player.form + 1 != form)
			{
				errorTot++;
				errorMsg << _T("\tForm is ") << player.form + 1 << _T(", should be ") << form << _T(";\r\n");
			}
			if (player.reg_pos == 0) //Medals can't be GK
			{
				errorTot++;
				errorMsg << _T("\tMedals cannot play as GK;\r\n");
			}

			cardMod += min(tricks, numTrick); //12 free tricks (aka all of them)
			cardMod += min(coms, numCom); //2 free COMs
			cardLimit = skills + cardMod; //8 skill cards

			if (player.injury + 1 != injury_resistance)
			{
				errorTot++;
				errorMsg << _T("\tInjury resist is ") << player.injury + 1 << _T(", should be ") << injury_resistance << _T(";\r\n");
			}
			if (useSuggestions)
			{
				//if (numTrick < tricks) errorMsg << _T(\tWARN: Has ") << numTrick << _T(" trick cards, allowed ") << tricks << _T(";\r\n");
				if (numCom < coms) errorMsg << _T("\tWARN: Has ") << numCom << _T(" COM cards, allowed ") << coms << _T(";\r\n");
				if (player.injury + 1 < injury_resistance) errorMsg << _T("\tWARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << injury_resistance << _T(";\r\n");
			}
		}
		else
		{
			errorTot++;
			errorMsg << _T("\tIllegal Ability scores, this player's stats does not match any available player types;\r\n"); //mentions that stats is what is being checked
			//spit out whatever errors were already found, but target scores can't be set, so quit out of this player to avoid useless error outputs
			msgOut += errorMsg.str();
			continue;
		}

		//Check player height
		/*if (((player.height - heightMod) <= heightManlet))
		{
			numManlet++;
			cardLimit += manletCardBonus; //Manlets get a bonus card
			if (countA > 1) cardLimit += manletPosBonus; //Manlets get a bonus double A position
			weakFootUse = manletWeakFootUse; //Manlets get weak foot acc/use 4/4
			weakFootAcc = manletWeakFootAcc;
		}
		else if ((player.height - heightMod) <= heightMid)
		{
			numMid++;
		}
		else if ((player.height - heightMod) == heightTall)
			numTall++;
		else if ((player.height - heightMod) == heightTallGK && player.reg_pos == 0) //GK
			numTall++;
		else if ((player.height - heightMod) == heightGiant)
			numGiant++;
		else if ((player.height - heightMod) == heightGiga)
			numGiga++;
		else
		{
			errorTot++;
			errorMsg << _T("Illegal height (") << player.height << _T(" cm); ");
		}*/
		/*if (player.height != allowedHeight)
		{
			errorTot++;
			errorMsg << _T("\tWrong height, allowed height is: ") << allowedHeight << _T(";\r\n");
		}*/

		//If more than allowed A positions, 1 card less for each
		if (countA > allowedAPostions + allowedAPostionsMod) cardLimit -= (countA - (allowedAPostions + allowedAPostionsMod));

		//Check weak foot ratings
		if (player.weak_use + 1 > weakFootUse)
		{
			errorTot++;
			errorMsg << _T("\tWeak foot usage > ") << weakFootUse << _T(";\r\n");
		}
		if (player.weak_acc + 1 > weakFootAcc)
		{
			errorTot++;
			errorMsg << _T("\tWeak foot accuracy > ") << weakFootAcc << _T(";\r\n");
		}

		//Check player card count
		if (cardCount > cardLimit)
		{
			errorTot++;
			errorMsg << _T("\tHas ") << cardCount - numTrick << _T(" cards, only allowed ") << cardLimit - numTrick << _T(";\r\n");
		}

		if (useSuggestions)
		{
			if (cardCount < cardLimit) errorMsg << _T("\tWARN: Has ") << cardCount - numTrick << _T(" cards, allowed ") << cardLimit - numTrick << _T(";\r\n");
			if (player.weak_use + 1 < weakFootUse) errorMsg << _T("\tWARN: Has weak usage ") << player.weak_use + 1 << _T(", allowed ") << weakFootUse << _T(";\r\n");
			if (player.weak_acc + 1 < weakFootAcc) errorMsg << _T("\tWARN: Has weak accuracy ") << player.weak_acc + 1 << _T(", allowed ") << weakFootAcc << _T(";\r\n");
			if (countA < allowedAPostions + allowedAPostionsMod) errorMsg << _T("\tWARN; Has ") << countA << _T(" A positions, allowed ") << (allowedAPostions + allowedAPostionsMod) << _T(";\r\n");
		}

		//Run over every on-field player, if they aren't registered to one of their positions add an error
		bool isInPosition = false;
		for (int index = 0; index < 23; index++) {
			if (aatf_check_player_in_pos(gteams[teamSel], player, player.reg_pos, 0)) {//Make sure they are in their registered position at some point
				isInPosition = true;
				break;
			}
		}
		if (!isInPosition) {
			errorTot++;
			errorMsg << _T("\tPlayer is a registered ") << aatf_get_position_name_from_byte(player.reg_pos) << _T(" but is not in that position in any Game Plan\r\n");
		}

		//Check player overall rating
		if (rating != targetRate)
		{
			errorTot++;
			errorMsg << _T("\tIllegal Ability scores; ");
		}

		//Check individual skill ratings
		//								c_skillRate			s_skillName			n_minPesVersion	n_targetRate
		skillCheck skillChecks[25] = { {player.drib,		_T("Dribbling"),			0,		(targetDrib		== 0) ? targetRate : targetDrib		},
										{player.gk,			_T("Goalkeeping"),			0,		(targetGk		== 0) ? targetRate : targetGk		},
										{player.finish,		_T("Finishing"),			0,		(targetFinish	== 0) ? targetRate : targetFinish	},
										{player.lowpass,	_T("Low Pass"),				0,		(targetLowPass	== 0) ? targetRate : targetLowPass	},
										{player.loftpass,	_T("Lofted Pass"),			0,		(targetLoftPass	== 0) ? targetRate : targetLoftPass	},
										{player.header,		_T("Header"),				0,		(targetHeader	== 0) ? targetRate : targetHeader	},
										{player.swerve,		_T("Swerve"),				0,		(targetSwerve	== 0) ? targetRate : targetSwerve	},
										{player.catching,	_T("Catching"),				0,		(targetCatching	== 0) ? targetRate : targetCatching	},
										{player.clearing,	_T("Clearing"),				16,		(targetClearing	== 0) ? targetRate : targetClearing	},
										{player.reflex,		_T("Reflexes"),				16,		(targetReflex	== 0) ? targetRate : targetReflex	},
										{player.body_ctrl,	_T("Body Control"),			0,		(targetBodyCtrl	== 0) ? targetRate : targetBodyCtrl	},
										{player.phys_cont,	_T("Physical Contact"),		17,		(targetPhysCont	== 0) ? targetRate : targetPhysCont	},
										{player.kick_pwr,	_T("Kicking Power"),		0,		(targetKickPwr	== 0) ? targetRate : targetKickPwr	},
										{player.exp_pwr,	_T("Explosive Power"),		0,		(targetExpPwr	== 0) ? targetRate : targetExpPwr	},
										{player.ball_ctrl,	_T("Ball Control"),			0,		(targetBallCtrl	== 0) ? targetRate : targetBallCtrl	},
										{player.ball_win,	_T("Ball Winning"),			0,		(targetBallWin	== 0) ? targetRate : targetBallWin	},
										{player.jump,		_T("Jump"),					0,		(targetJump		== 0) ? targetRate : targetJump		},
										{player.cover,		_T("Coverage"),				16,		(targetCover	== 0) ? targetRate : targetCover	},
										{player.place_kick, _T("Place Kicking"),		0,		(targetPlcKick	== 0) ? targetRate : targetPlcKick	},
										{player.stamina,	_T("Stamina"),				0,		(targetStamina	== 0) ? targetRate : targetStamina	},
										{player.speed,		_T("Speed"),				0,		(targetSpeed	== 0) ? targetRate : targetSpeed	},
										{player.atk,		_T("Attacking Prowess"),	0,		(targetAtkProw	== 0) ? targetRate : targetAtkProw	},
										{player.def,		_T("Defensive Prowess"),	0,		(targetDefProw	== 0) ? targetRate : targetDefProw	},
										{player.tight_pos,	_T("Tight Possession"),		20,		(targetTightPos	== 0) ? targetRate : targetTightPos	},
										{player.aggres,		_T("Aggression"),			20,		(targetAggres	== 0) ? targetRate : targetAggres	} };

		for (int ii = 0; ii < 25; ii++)
		{
			//atk and def can be lower than the target rate
			if (skillChecks[ii].s_skillName == _T("Attacking Prowess") || skillChecks[ii].s_skillName == _T("Defensive Prowess"))
			{
				if (skillChecks[ii].n_minPesVersion <= pesVersion && skillChecks[ii].c_skillRate > skillChecks[ii].n_targetRate)
				{
					errorTot++;
					errorMsg << _T("\t") << skillChecks[ii].s_skillName << _T(" is ") << skillChecks[ii].c_skillRate << _T(", should be <= ") << skillChecks[ii].n_targetRate << _T(";\r\n");
				}
			}
			else if (skillChecks[ii].n_minPesVersion <= pesVersion &&
				skillChecks[ii].c_skillRate != skillChecks[ii].n_targetRate)
			{
				errorTot++;
				errorMsg << _T("\t") << skillChecks[ii].s_skillName << _T(" is ") << skillChecks[ii].c_skillRate << _T(", should be ") << skillChecks[ii].n_targetRate << _T(";\r\n");
			}
		}

		if (errorMsg.rdbuf()->in_avail()) msgOut += errorMsg.str();
	}
	//Team level errors
	int diff;
	tstringstream errorMsg;

	if (!hasCaptain)
	{
		errorTot++;
		errorMsg << _T("\tTeam must have an assigned Captain;\r\n");
	}

	if (!captainHasCaptaincy && useSuggestions) errorMsg << _T("\tWARN: Captain does not have the free Captaincy card;\r\n");

	//Must have at least 1 GK
	if (numGK < 1)
	{
		errorTot++;
		errorMsg << _T("\tTeam must have a registered GK;\r\n");
	}

	//Check heights
	/*if (!usingRed) //Using Green height system
	{
		msgOut += _T("Using Green height system\r\n");
		if (diff = greenGiga - numGiga)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("\tHas ") << numGiga << _T("/") << greenGiga << _T(" ") << heightGiga << _T("cm players;\r\n");
		}
		if (diff = greenGiant - numGiant)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("\tHas ") << numGiant << _T("/") << greenGiant << _T(" ") << heightGiant << _T("cm players;\r\n");
		}
		if (diff = greenTall - numTall)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("\tHas ") << numTall << _T("/") << greenTall << _T(" ") << heightTall << _T("/") << heightTallGK << _T("cm players;\r\n");
		}
		if (diff = greenMid - numMid)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("\tHas ") << numMid << _T("/") << greenMid << _T(" ") << heightMid << _T("cm players;\r\n");
		}
		if (diff = greenManlet - numManlet)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("\tHas ") << numManlet << _T("/") << greenManlet << _T(" ") << heightManlet << _T("cm players;\r\n");
		}
	}
	else //Using Red height system
	{
		msgOut += _T("Using Red height system\r\n");
		if (diff = numGiga)
		{
			errorTot += diff;
			errorMsg << _T("\tHas ") << numGiga << _T("/") << redGiga << _T(" ") << heightGiga << _T("cm players;\r\n");
		}
		if (diff = numGiant)
		{
			errorTot += diff;
			errorMsg << _T("\tHas ") << numGiant << _T("/") << redGiant << _T(" ") << heightGiant << _T("cm players;\r\n");
		}
		if (diff = redTall - numTall)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("\tHas ") << numTall << _T("/") << redTall << _T(" ") << heightTall << _T("/") << heightTallGK << _T("cm players;\r\n");
		}
		if (diff = redMid - numMid)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("\tHas ") << numMid << _T("/") << redMid << _T(" ") << heightMid << _T("cm players;\r\n");
		}
		if (diff = redManlet - numManlet)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			errorMsg << _T("\tHas ") << numManlet << _T("/") << redManlet << _T(" ") << heightManlet << _T("cm players;\r\n");
		}
	}*/
	if (errorMsg.rdbuf()->in_avail())
	{
		errorMsg << _T("\r\n");
		msgOut += errorMsg.str();
		errorMsg.clear();
		errorMsg.str(tstring());
	}

	//Check ability stats
	/*if (numReg != regular::count)
	{
		errorTot++;
		errorMsg << _T("\tNumber of Regular players is ") << numReg << _T(", should be ") << regular::count << _T(";\r\n");
	}*/
	/*if (numBuff != buffed::count)
	{
		errorTot++;
		errorMsg << _T("\tNumber of Buffed players is ") << numBuff << _T(", should be ") << buffed::count << _T(";\r\n");
	}*/
	if (numBronze != globalStats::allowedBronze)
	{
		errorTot++;
		errorMsg << _T("\tNumber of Bronze medals is ") << numBronze << _T(", should be ") << globalStats::allowedBronze << _T(";\r\n");
	}
	if (numSilver != globalStats::allowedSilver)
	{
		errorTot++;
		errorMsg << _T("\tNumber of Silver medals is ") << numSilver << _T(", should be ") << globalStats::allowedSilver << _T(";\r\n");
	}
	if (numGold != globalStats::allowedGold)
	{
		errorTot++;
		errorMsg << _T("\tNumber of Gold medals is ") << numGold << _T(", should be ") << globalStats::allowedGold << _T(";\r\n");
	}
	if (numGiga != globalStats::allowedColossal)
	{
		errorTot++;
		errorMsg << _T("\tNumber of ") << globalStats::heightColossal << _T(" players is ") << numTall << _T(". Should be ") << globalStats::allowedColossal << _T(";\r\n");
	}
	if (numGiant != globalStats::allowedGiant)
	{
		errorTot++;
		errorMsg << _T("\tNumber of ") << globalStats::heightGiant << _T(" players is ") << numTall << _T(". Should be ") << globalStats::allowedGiant << _T(";\r\n");
	}
	if (numTall != globalStats::allowedTall)
	{
		errorTot++;
		errorMsg << _T("\tNumber of ") << globalStats::heightTall << _T(" players is ") << numTall << _T(". Should be ") << globalStats::allowedTall << _T(";\r\n");
	}
	if (numMid != globalStats::allowedMid)
	{
		errorTot++;
		errorMsg << _T("\tNumber of ") << globalStats::heightMid << _T(" players is ") << numTall << _T(". Should be ") << globalStats::allowedMid << _T(";\r\n");
	}
	if (numManlet != globalStats::allowedManlet)
	{
		errorTot++;
		errorMsg << _T("\tNumber of ") << globalStats::heightManlet << _T(" players is ") << numTall << _T(". Should be ") << globalStats::allowedManlet << _T(";\r\n");
	}
	if (errorMsg.rdbuf()->in_avail()) errorMsg << _T("\r\n");
	errorMsg << _T("\r\nErrors: ") << errorTot << _T("\r\n");
	msgOut += errorMsg.str();

	SetWindowText(GetDlgItem(hAatfbox, IDT_AATFOUT), msgOut.c_str());
	if (errorTot)
		SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM)_T("It's all fucked."));
	else
		SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM)_T("Perfect, blaze."));
}


//Exclusive: Check if player is ONLY in that position across all presets and formations
bool aatf_check_player_in_pos(team_entry& team, player_entry& player, int position, bool exclusive)
{
	bool isInPos = true;
	bool isInStarting11 = false;
	int playerIndex = 0;

	for (int ii = 0; ii < 11; ii++)
	{
		if (player.id == (team.id * 100) + 1 + team.starting11[ii])
		{
			isInStarting11 = true;
			playerIndex = ii;
			break;
		}
	}

	if (isInStarting11)
	{
		for (int indexP = 0; indexP < 3; indexP++)
		{
			int formationCount = team.presets[indexP].fluid ? 3 : 1;
			for (int indexF = 0; indexF < formationCount; indexF++)
			{
				int playerPos = team.presets[indexP].formations[indexF].players[playerIndex].pos;

				//If not exclusive, return true if any player has that position in any preset or formation
				if (!exclusive && playerPos == position)
					return true;

				isInPos = isInPos && team.presets[indexP].formations[indexF].players[playerIndex].pos == position;
			}
		}
	}
	else
	{
		return player.reg_pos == position;
	}

	return isInPos;
}

wchar_t* aatf_get_position_name_from_byte(byte pos)
{
	switch (pos)
	{
	case 0x01:
		return L"CB";
		break;
	case 0x02:
		return L"LB";
		break;
	case 0x03:
		return L"RB";
		break;
	case 0x04:
		return L"DMF";
		break;
	case 0x05:
		return L"CMF";
		break;
	case 0x06:
		return L"LMF";
		break;
	case 0x07:
		return L"RMF";
		break;
	case 0x08:
		return L"AMF";
		break;
	case 0x09:
		return L"LWF";
		break;
	case 0x0A:
		return L"RWF";
		break;
	case 0x0B:
		return L"SS";
		break;
	case 0x0C:
		return L"CF";
		break;
	default:
		return L"GK";
		break;
	}
}