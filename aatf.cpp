#include "editor.h"

//Exclusive: Check if player is ONLY in that position in the first preset
bool aatf_check_player_in_pos_first_preset(team_entry& team, player_entry& player, int position, bool exclusive)
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
		int playerPos = team.presets[0].formations[0].players[playerIndex].pos;

		//If not exclusive, return true if any player has that position in any preset or formation
		if (!exclusive && playerPos == position)
			return true;

		isInPos = isInPos && team.presets[0].formations[0].players[playerIndex].pos == position;
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
