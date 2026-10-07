#define OEMRESOURCE //Necessary in order for OBM_CHECK to be defined when
					// loading system bitmaps
					
#define SIZE_CHECK 13 //Typical pixel size of system checkmark icon

#include <Commdlg.h>
#include <windowsx.h>	//Provides Win32 macros (e.g. Button_GetCheck())
#include <stdio.h>		//C standard I/O functions (fopen, sprintf, etc.)
#include <Shlobj.h>		//Provides shell objects 
						//  (SHGetFolderPath(), SHBrowseForFolder(), 
						//  SHGetPathFromIDList(), BROWSEINFO struct)
#include <time.h> 
#include <math.h> 
//#include <errno.h>
//#include <Uxtheme.h>

//Set to use "Modern" visual style for windows controls:
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//Dialog procedures:
LRESULT CALLBACK wnd_proc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK aatf_sing_dlg_proc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK aatf_mult_dlg_proc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK aatf_mult_dlg_proc_sug(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK aatf_sel_dlg_proc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK aatf_sel_dlg_proc_sug(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK aatf_comp_dlg_proc(HWND, UINT, WPARAM, LPARAM);

//Control procedures:
LRESULT CALLBACK lv_cntl_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK cb_cntl_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK cb2_cntl_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK scale_cntl_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK scale_static_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK onto_tab_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK from_tab_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK tab_two_dlg_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK tab_three_dlg_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK tab_four_dlg_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
BOOL CALLBACK statDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK bumpDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK copyDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK swapDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK bogloDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK settings_dlg_proc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK scale_children(HWND,LPARAM);
BOOL CALLBACK draw_children(HWND,LPARAM);

//Helper functions
void setup_control(HWND,HFONT,SUBCLASSPROC);
void setup_combo(HWND,HFONT,SUBCLASSPROC);

//Settings (settings.cpp)
bool load_settings();	//Reads 4cce_settings.cfg, returns true if the file was found
void save_settings();	//Writes the current options to 4cce_settings.cfg
void show_settings(HWND);	//Displays the Settings window
void apply_autocolor_layout(bool);	//Shows/hides the Add Color buttons and resizes the Make ... buttons
void update_make_buttons_enabled();	//Greys out Make ... for classes the selected ruleset does not allow
void update_ruleset_menu();	//Bolds the "Load PES## Edit file" item for the ruleset's PES_VERSION
void update_logo_bitmap();	//Picks the VGL or 4CC logo for the AATF ruleset's LEAGUE_TYPE
void update_skill_card_labels();	//Marks the cards the selected ruleset makes free with a trailing '*'
void make_work_path(TCHAR*, size_t, const TCHAR*);	//Resolves a file name against the startup directory

//Global variables
extern HINSTANCE ghinst;	//Main window instance
extern HWND ghw_main;	//Main window
extern HWND ghw_tabcon, ghw_tab1, ghw_tab2, ghw_tab3, ghw_tab4;
extern HWND ghw_settings;	//Settings window
extern bool gb_autoFixDb;	//Auto Fix Database option
extern bool gb_autoColorNames;	//Auto-Color Names option
extern bool gb_autoManlet;	//Auto-Manlet option
extern TCHAR g_tc_work_dir[MAX_PATH];	//Directory the editor was started in; settings and rulesets live here
extern TCHAR g_tc_ruleset_file[MAX_PATH];	//AATF ruleset .cfg selected in Settings
extern HFONT ghFont;

//Main window
void setup_main(HWND);
void setup_tab1(HWND);
void setup_tab2(HWND);
void setup_tab3(HWND);
void setup_tab4(HWND);
