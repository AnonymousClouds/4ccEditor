//----------------------------------------------------------------------
/*Settings window and saved configuration options
  The options are stored in 4cce_settings.cfg in the working directory*/

#include "resource.h"
#include "editor.h"
#include "window.h"

#include <string>
#include <vector>
#include <algorithm>

//----------------------------------------------------------------------
/*Global variables*/
HWND ghw_settings = NULL;	//Handle to the Settings window
bool gb_autoFixDb = false;	//Whether Database > Fix database runs automatically before saving
bool gb_autoColorNames = false;	//Whether Make ... applies the color tag on its own
bool gb_autoManlet = false;	//Whether entering the manlet height applies the manlet buff automatically
TCHAR g_tc_work_dir[MAX_PATH] = _T("");	//Directory the editor was started in
TCHAR g_tc_ruleset_file[MAX_PATH] = _T("");	//AATF ruleset .cfg selected in Settings (empty = none)

//Name of the file holding the saved configuration options
const TCHAR* gpc_settings_file = _T("4cce_settings.cfg");

//The settings that are currently applied; editing the window keeps the
//  pending values in the globals and only Apply (or Cancel) commits them
static bool gb_appliedAutoFix = false;
static bool gb_appliedAutoColor = false;
static bool gb_appliedAutoManlet = false;
static TCHAR gtc_appliedRuleset[MAX_PATH] = _T("");

//The controls making up each setting: checkbox, title label and description
const struct
{
	int n_check;				//Checkbox (or the combo box for the ruleset row)
	int n_title;				//Title label (hovering over it also shows the description)
	const TCHAR* pcs_text;		//Description, shown in the floating box while the setting is hovered
} gsc_settings[] =
{
	{ IDC_SET_AUTOFIX, IDC_SET_AUTOFIX_TITLE,
		_T("Runs Database > Fix database automatically before an edit file is saved.") },
	{ IDC_SET_AUTOCOLOR, IDC_SET_AUTOCOLOR_TITLE,
		_T("Automatically adds the medal color tag to the player name when Make Gold, Make Silver or Make Bronze is used, and hides the Add Color buttons.") },
	{ IDC_SET_MANLET, IDC_SET_MANLET_TITLE,
		_T("When a player's height is set to the manlet class height and the active height bracket allows the manlet buff, automatically adds the class's manlet stat bonus to every ability and raises Weak Foot Usage/Accuracy to the bracket's values.") },
	{ IDC_SET_RULESET, IDC_SET_RULESET_TITLE,
		_T("The ruleset .cfg file AATF checks teams against. Put the file next to the editor, then pick it here; the list is refreshed every time it is opened. AATF will not run without a ruleset.") },
	{ IDC_SET_NEWRULESET, IDC_SET_NEWRULESET,
		_T("Create a blank example AATF configuration file.") },
};

//----------------------------------------------------------------------
//Resolve a file name against the directory the editor was started in.
//  File dialogs can change the process's current directory, so settings and
//  rulesets must not use the current directory directly.
void make_work_path(TCHAR* pc_dest, size_t n_dest, const TCHAR* pc_leaf)
{
	if(pc_leaf[0] && PathIsRelative(pc_leaf) && g_tc_work_dir[0])
		_sntprintf_s(pc_dest, n_dest, _TRUNCATE, _T("%s\\%s"), g_tc_work_dir, pc_leaf);
	else
		_tcsncpy_s(pc_dest, n_dest, pc_leaf, _TRUNCATE);
}

//----------------------------------------------------------------------
//Copy a setting's value from a line of the settings file, trimming spaces
//  and the line ending
static void copy_setting_value(TCHAR* pc_dest, const TCHAR* pc_src)
{
	while(*pc_src == _T(' ') || *pc_src == _T('\t')) pc_src++;
	int len = (int)_tcslen(pc_src);
	while(len > 0 && (pc_src[len - 1] == _T('\r') || pc_src[len - 1] == _T('\n') ||
		pc_src[len - 1] == _T(' ') || pc_src[len - 1] == _T('\t'))) len--;
	if(len >= MAX_PATH) len = MAX_PATH - 1;
	_tcsncpy_s(pc_dest, MAX_PATH, pc_src, len);
	pc_dest[len] = 0;
}

//----------------------------------------------------------------------
//Load the saved configuration options from 4cce_settings.cfg
//Returns true if the file was found (and the options loaded), false otherwise
bool load_settings()
{
	//Capture the directory the editor was started in; everything relative
	//  (settings file, ruleset list) is resolved against it from now on
	if(!GetCurrentDirectory(MAX_PATH, g_tc_work_dir))
		g_tc_work_dir[0] = 0;

	TCHAR cs_path[MAX_PATH];
	make_work_path(cs_path, MAX_PATH, gpc_settings_file);

	FILE* pFile = _tfopen(cs_path, _T("r"));
	if(!pFile) return false;

	TCHAR cs_line[256];
	while(_fgetts(cs_line, 256, pFile))
	{
		if(_tcsncmp(cs_line, _T("auto_fix_db="), 12) == 0)
			gb_autoFixDb = (_ttoi(&cs_line[12]) != 0);
		else if(_tcsncmp(cs_line, _T("auto_color_names="), 17) == 0)
			gb_autoColorNames = (_ttoi(&cs_line[17]) != 0);
		else if(_tcsncmp(cs_line, _T("auto_manlet="), 12) == 0)
			gb_autoManlet = (_ttoi(&cs_line[12]) != 0);
		else if(_tcsncmp(cs_line, _T("ruleset_cfg="), 12) == 0)
			copy_setting_value(g_tc_ruleset_file, &cs_line[12]);
	}
	fclose(pFile);
	return true;
}

//----------------------------------------------------------------------
//Write the current configuration options to 4cce_settings.cfg
void save_settings()
{
	TCHAR cs_path[MAX_PATH];
	make_work_path(cs_path, MAX_PATH, gpc_settings_file);

	FILE* pFile = _tfopen(cs_path, _T("w"));
	if(!pFile) return;

	_ftprintf(pFile, _T("auto_fix_db=%d\r\n"), gb_autoFixDb ? 1 : 0);
	_ftprintf(pFile, _T("auto_color_names=%d\r\n"), gb_autoColorNames ? 1 : 0);
	_ftprintf(pFile, _T("auto_manlet=%d\r\n"), gb_autoManlet ? 1 : 0);
	_ftprintf(pFile, _T("ruleset_cfg=%s\r\n"), g_tc_ruleset_file);
	fclose(pFile);
}

//----------------------------------------------------------------------
/*The AATF ruleset dropdown lists the .cfg files in the editor's folder
  (except 4cce_settings.cfg). The list is rebuilt every time the dropdown
  is opened, so a file created while the window is open shows up at once.*/

typedef std::basic_string<TCHAR> SString;

static bool ruleset_name_less(const SString& a, const SString& b)
{
	return _tcsicmp(a.c_str(), b.c_str()) < 0;
}

//Rebuild the dropdown and select the current/saved ruleset if it exists.
//  prefer_current keeps whatever the dropdown already shows (used while it is
//  being opened); otherwise the pending ruleset is selected.
static void refresh_ruleset_list(HWND hwnd, bool prefer_current)
{
	HWND hw_combo = GetDlgItem(hwnd, IDC_SET_RULESET);
	if(!hw_combo) return;

	//Remember the current selection unless the entry is "(none)"
	TCHAR tc_current[MAX_PATH] = _T("");
	int n_current = (int)SendMessage(hw_combo, CB_GETCURSEL, 0, 0);
	if(n_current > 0)
		SendMessage(hw_combo, CB_GETLBTEXT, n_current, (LPARAM)tc_current);

	SendMessage(hw_combo, CB_RESETCONTENT, 0, 0);
	SendMessage(hw_combo, CB_ADDSTRING, 0, (LPARAM)_T("(none)"));

	//Collect the .cfg files next to the editor, excluding its settings file
	std::vector<SString> files;
	WIN32_FIND_DATA s_find;
	TCHAR cs_glob[MAX_PATH];
	make_work_path(cs_glob, MAX_PATH, _T("*.cfg"));
	HANDLE h_find = FindFirstFile(cs_glob, &s_find);
	if(h_find != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(s_find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
			if(_tcsicmp(s_find.cFileName, gpc_settings_file) == 0) continue;
			files.push_back(SString(s_find.cFileName));
		}
		while(FindNextFile(h_find, &s_find));
		FindClose(h_find);
	}
	std::sort(files.begin(), files.end(), ruleset_name_less);
	for(size_t ii = 0; ii < files.size(); ii++)
		SendMessage(hw_combo, CB_ADDSTRING, 0, (LPARAM)files[ii].c_str());

	//Select the current item, else the pending ruleset, else "(none)"
	const TCHAR* pc_select = (prefer_current && tc_current[0]) ? tc_current : g_tc_ruleset_file;
	int n_select = 0;
	if(pc_select[0])
	{
		n_select = (int)SendMessage(hw_combo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pc_select);
		if(n_select < 0) n_select = 0;
	}
	SendMessage(hw_combo, CB_SETCURSEL, n_select, 0);

	//If the selected file no longer exists, clear the pending ruleset
	TCHAR tc_selected[MAX_PATH] = _T("");
	if(n_select > 0) SendMessage(hw_combo, CB_GETLBTEXT, n_select, (LPARAM)tc_selected);
	if(_tcsicmp(tc_selected, g_tc_ruleset_file) != 0)
		_tcsncpy_s(g_tc_ruleset_file, MAX_PATH, tc_selected, _TRUNCATE);
}

//----------------------------------------------------------------------
/*The New button next to the ruleset dropdown writes the blank example
  ruleset (embedded in the exe from blank_config.cfg) to a new .cfg file in
  the editor's folder and selects it, so it can be used as a starting point*/

static void create_blank_ruleset(HWND hwnd)
{
	HRSRC hrsrc = FindResource(ghinst, MAKEINTRESOURCE(IDR_BLANK_CFG), RT_RCDATA);
	if(!hrsrc) return;
	DWORD dw_size = SizeofResource(ghinst, hrsrc);
	HGLOBAL hglob = LoadResource(ghinst, hrsrc);
	if(!hglob || dw_size == 0) return;
	const void* pdata = LockResource(hglob);
	if(!pdata) return;

	//Pick a free file name: new_ruleset.cfg, then new_ruleset_2.cfg, ...
	TCHAR cs_name[MAX_PATH];
	TCHAR cs_path[MAX_PATH];
	_tcsncpy_s(cs_name, MAX_PATH, _T("new_ruleset.cfg"), _TRUNCATE);
	make_work_path(cs_path, MAX_PATH, cs_name);
	for(int n = 2; PathFileExists(cs_path); n++)
	{
		_sntprintf_s(cs_name, MAX_PATH, _TRUNCATE, _T("new_ruleset_%d.cfg"), n);
		make_work_path(cs_path, MAX_PATH, cs_name);
	}

	FILE* pFile = _tfopen(cs_path, _T("wb"));
	if(!pFile)
	{
		MessageBox(hwnd, _T("Could not create the new ruleset file."), _T("Settings"), MB_ICONERROR);
		return;
	}
	fwrite(pdata, 1, dw_size, pFile);
	fclose(pFile);

	//Make the new file the pending ruleset and select it in the dropdown
	_tcsncpy_s(g_tc_ruleset_file, MAX_PATH, cs_name, _TRUNCATE);
	refresh_ruleset_list(hwnd, false);
}

//----------------------------------------------------------------------
/*The Settings window edits pending values; Apply writes them to
  4cce_settings.cfg and closes the window, Cancel drops them*/

//Remember the currently applied settings
static void snapshot_settings()
{
	gb_appliedAutoFix = gb_autoFixDb;
	gb_appliedAutoColor = gb_autoColorNames;
	gb_appliedAutoManlet = gb_autoManlet;
	_tcsncpy_s(gtc_appliedRuleset, MAX_PATH, g_tc_ruleset_file, _TRUNCATE);
}

//Read the controls and commit them
static void apply_pending_settings(HWND hwnd)
{
	gb_autoFixDb = (Button_GetCheck(GetDlgItem(hwnd, IDC_SET_AUTOFIX)) == BST_CHECKED);
	gb_autoColorNames = (Button_GetCheck(GetDlgItem(hwnd, IDC_SET_AUTOCOLOR)) == BST_CHECKED);
	gb_autoManlet = (Button_GetCheck(GetDlgItem(hwnd, IDC_SET_MANLET)) == BST_CHECKED);

	HWND hw_combo = GetDlgItem(hwnd, IDC_SET_RULESET);
	int n_sel = (int)SendMessage(hw_combo, CB_GETCURSEL, 0, 0);
	if(n_sel <= 0) g_tc_ruleset_file[0] = 0;
	else SendMessage(hw_combo, CB_GETLBTEXT, n_sel, (LPARAM)g_tc_ruleset_file);

	save_settings();
	snapshot_settings();
	apply_autocolor_layout(gb_autoColorNames);
	update_make_buttons_enabled();
	update_ruleset_menu();
	update_logo_bitmap();
	update_skill_card_labels();
}

//Restore the last applied settings in case the user cancels
static void discard_pending_settings(HWND hwnd)
{
	gb_autoFixDb = gb_appliedAutoFix;
	gb_autoColorNames = gb_appliedAutoColor;
	gb_autoManlet = gb_appliedAutoManlet;
	_tcsncpy_s(g_tc_ruleset_file, MAX_PATH, gtc_appliedRuleset, _TRUNCATE);

	Button_SetCheck(GetDlgItem(hwnd, IDC_SET_AUTOFIX), gb_autoFixDb ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hwnd, IDC_SET_AUTOCOLOR), gb_autoColorNames ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hwnd, IDC_SET_MANLET), gb_autoManlet ? BST_CHECKED : BST_UNCHECKED);
	refresh_ruleset_list(hwnd, false);
	apply_autocolor_layout(gb_autoColorNames);
	update_make_buttons_enabled();
	update_ruleset_menu();
}

//----------------------------------------------------------------------
/*Each setting has a description, shown in a small floating box next to the
  mouse pointer while the checkbox or title label is hovered over*/

//The floating description box and the setting it currently describes (-1: hidden)
static HWND ghw_tip = NULL;
static int gn_tip_setting = -1;

//Create the floating description box, if it doesn't exist yet
static void create_tip(HWND hwnd)
{
	if(ghw_tip) return;

	//Small popup window, owned by the Settings window so it is always shown on
	//  top of it and is destroyed together with it
	ghw_tip = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, _T("STATIC"), NULL,
		WS_POPUP | WS_BORDER | SS_LEFT,
		0, 0, 0, 0, hwnd, NULL, ghinst, NULL);
	if(!ghw_tip) return;

	//Match the text style of the dialog
	SendMessage(ghw_tip, WM_SETFONT,
		SendMessage(GetDlgItem(hwnd, IDC_SET_AUTOFIX), WM_GETFONT, 0, 0), FALSE);
}

//Resize the floating description box to fit the given text
static void size_tip(const TCHAR* pcs_text)
{
	HDC hdc = GetDC(ghw_tip);
	HFONT hf_old = (HFONT)SelectObject(hdc, (HFONT)SendMessage(ghw_tip, WM_GETFONT, 0, 0));

	RECT rc_text = { 0, 0, 320, 0 };	//Wrap after 320 pixels
	DrawText(hdc, pcs_text, -1, &rc_text, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);

	SelectObject(hdc, hf_old);
	ReleaseDC(ghw_tip, hdc);

	SetWindowPos(ghw_tip, NULL, 0, 0,
		rc_text.right - rc_text.left + 12, rc_text.bottom - rc_text.top + 8,	//Padding
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

//Hide the floating description box
static void hide_setting_description(HWND hwnd)
{
	if(!ghw_tip || gn_tip_setting < 0) return;

	ShowWindow(ghw_tip, SW_HIDE);
	gn_tip_setting = -1;
}

//Show the description of the setting the mouse pointer is over, and only that
//  one; the box follows the pointer while it stays over the setting
static void show_hovered_setting_description(HWND hwnd)
{
	if(!ghw_tip) return;

	POINT s_pt;
	if(!GetCursorPos(&s_pt)) return;

	int n_hovered = -1;
	for(int ii = 0; ii < _countof(gsc_settings) && n_hovered < 0; ii++)
	{
		//Both the checkbox and the title label count as hovering over the setting
		for(int jj = 0; jj < 2; jj++)
		{
			HWND hw_cntl = GetDlgItem(hwnd, jj == 0 ? gsc_settings[ii].n_check : gsc_settings[ii].n_title);
			RECT rc_cntl;
			if(!hw_cntl || !GetWindowRect(hw_cntl, &rc_cntl)) continue;

			//A drop-down combo's window includes its hidden list, so only the
			//  visible selection field should count as hovering the setting
			int n_type = GetWindowLong(hw_cntl, GWL_STYLE) & 0xF;
			if(n_type == CBS_DROPDOWN || n_type == CBS_DROPDOWNLIST)
			{
				int n_field = (int)SendMessage(hw_cntl, CB_GETITEMHEIGHT, (WPARAM)-1, 0);
				rc_cntl.bottom = rc_cntl.top + n_field + 8;
			}

			if(PtInRect(&rc_cntl, s_pt))
			{
				n_hovered = ii;
				break;
			}
		}
	}

	if(n_hovered < 0)	//Pointer not over any setting: hide the box
	{
		hide_setting_description(hwnd);
		return;
	}

	if(n_hovered != gn_tip_setting)	//Switched settings: show the new description
	{
		SetWindowText(ghw_tip, gsc_settings[n_hovered].pcs_text);
		size_tip(gsc_settings[n_hovered].pcs_text);
		gn_tip_setting = n_hovered;
		ShowWindow(ghw_tip, SW_SHOWNOACTIVATE);
	}
	//Keep the box just below and to the right of the pointer
	SetWindowPos(ghw_tip, HWND_TOPMOST, s_pt.x + 16, s_pt.y + 20, 0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE);
}

//----------------------------------------------------------------------
//Display the Settings window, creating it first if it doesn't exist yet
void show_settings(HWND hwnd_owner)
{
	if(!ghw_settings)
		ghw_settings = CreateDialog(ghinst, MAKEINTRESOURCE(IDD_SETTINGS), hwnd_owner, settings_dlg_proc);
	if(!ghw_settings) return;

	//Already open: just bring it to the front and keep the pending edits
	if(IsWindowVisible(ghw_settings))
	{
		SetForegroundWindow(ghw_settings);
		return;
	}

	//Reopening after Apply or Cancel: start from the applied options
	snapshot_settings();
	Button_SetCheck(GetDlgItem(ghw_settings, IDC_SET_AUTOFIX), gb_autoFixDb ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(ghw_settings, IDC_SET_AUTOCOLOR), gb_autoColorNames ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(ghw_settings, IDC_SET_MANLET), gb_autoManlet ? BST_CHECKED : BST_UNCHECKED);
	refresh_ruleset_list(ghw_settings, false);

	//Centre the window over its owner
	RECT rcDlg, rcOwner;
	GetWindowRect(ghw_settings, &rcDlg);
	GetWindowRect(hwnd_owner, &rcOwner);
	SetWindowPos(ghw_settings, HWND_TOP,
		(rcOwner.left + ((rcOwner.right - rcOwner.left) - (rcDlg.right - rcDlg.left)) / 2),
		(rcOwner.top + ((rcOwner.bottom - rcOwner.top) - (rcDlg.bottom - rcDlg.top)) / 2),
		0, 0, SWP_NOSIZE);

	ShowWindow(ghw_settings, SW_SHOW);
	SetForegroundWindow(ghw_settings);
	SetFocus(GetDlgItem(ghw_settings, IDC_SET_AUTOFIX));
}

//----------------------------------------------------------------------
//Message handler for the Settings window
BOOL CALLBACK settings_dlg_proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
		case WM_INITDIALOG:
			snapshot_settings();
			Button_SetCheck(GetDlgItem(hwnd, IDC_SET_AUTOFIX), gb_autoFixDb ? BST_CHECKED : BST_UNCHECKED);
			Button_SetCheck(GetDlgItem(hwnd, IDC_SET_AUTOCOLOR), gb_autoColorNames ? BST_CHECKED : BST_UNCHECKED);
			Button_SetCheck(GetDlgItem(hwnd, IDC_SET_MANLET), gb_autoManlet ? BST_CHECKED : BST_UNCHECKED);
			refresh_ruleset_list(hwnd, false);
			create_tip(hwnd);	//Descriptions are shown in a floating box
			SetTimer(hwnd, IDC_SET_HOVERTIMER, 100, NULL);
			return TRUE;
		case WM_SHOWWINDOW:
			if(wParam)	//Only poll the mouse pointer while the window is displayed
				SetTimer(hwnd, IDC_SET_HOVERTIMER, 100, NULL);
			else
			{
				KillTimer(hwnd, IDC_SET_HOVERTIMER);
				hide_setting_description(hwnd);
			}
		break;
		case WM_TIMER:
			if(wParam == IDC_SET_HOVERTIMER)
			{
				show_hovered_setting_description(hwnd);
				return TRUE;
			}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_SET_AUTOFIX:
					if(HIWORD(wParam) == BN_CLICKED) //Pending change; Apply commits it
						gb_autoFixDb = (Button_GetCheck(GetDlgItem(hwnd, IDC_SET_AUTOFIX)) == BST_CHECKED);
				break;
				case IDC_SET_AUTOCOLOR:
					if(HIWORD(wParam) == BN_CLICKED)
					{
						gb_autoColorNames = (Button_GetCheck(GetDlgItem(hwnd, IDC_SET_AUTOCOLOR)) == BST_CHECKED);
						apply_autocolor_layout(gb_autoColorNames);	//Show the effect right away
					}
				break;
				case IDC_SET_MANLET:
					if(HIWORD(wParam) == BN_CLICKED) //Pending change; Apply commits it
						gb_autoManlet = (Button_GetCheck(GetDlgItem(hwnd, IDC_SET_MANLET)) == BST_CHECKED);
				break;
				case IDC_SET_RULESET:
					if(HIWORD(wParam) == CBN_SELCHANGE)
					{
						//Pending change: remember the picked ruleset, "(none)" clears it
						int n_sel = (int)SendMessage(GetDlgItem(hwnd, IDC_SET_RULESET), CB_GETCURSEL, 0, 0);
						if(n_sel <= 0) g_tc_ruleset_file[0] = 0;
						else SendMessage(GetDlgItem(hwnd, IDC_SET_RULESET), CB_GETLBTEXT, n_sel, (LPARAM)g_tc_ruleset_file);
					}
					else if(HIWORD(wParam) == CBN_DROPDOWN)
					{
						//Rescan for .cfg files in case one was created while
						//  the settings window was open
						refresh_ruleset_list(hwnd, true);
					}
				break;
				case IDC_SET_NEWRULESET:
					if(HIWORD(wParam) == BN_CLICKED)
						create_blank_ruleset(hwnd);
				break;
				case IDC_SET_APPLY:
					if(HIWORD(wParam) == BN_CLICKED)
					{
						apply_pending_settings(hwnd);
						ShowWindow(hwnd, SW_HIDE);
					}
				break;
				case IDC_SET_AUTOFIX_TITLE:	//Clicking the title toggles its checkbox
				case IDC_SET_AUTOCOLOR_TITLE:
					if(HIWORD(wParam) == STN_CLICKED)
						SendMessage(GetDlgItem(hwnd, LOWORD(wParam) == IDC_SET_AUTOFIX_TITLE ?
							IDC_SET_AUTOFIX : IDC_SET_AUTOCOLOR), BM_CLICK, 0, 0);
				break;
				case IDC_CANCEL:
				case IDCANCEL:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
				break;
			}
		break;
		case WM_CLOSE:
			//Cancel: drop any pending changes, then hide rather than destroy so
			//  the window can be reopened from the File menu.
			//Must return TRUE: returning FALSE hands WM_CLOSE to the default dialog
			//processing, which sends an endless stream of further WM_CLOSE messages, so the
			//window gets hidden again as soon as show_settings() displays it
			discard_pending_settings(hwnd);
			ShowWindow(hwnd, SW_HIDE);
			return TRUE;
		case WM_DESTROY:
			KillTimer(hwnd, IDC_SET_HOVERTIMER);
			hide_setting_description(hwnd);
		break;
	}
	return FALSE;
}
