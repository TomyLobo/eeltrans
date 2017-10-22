#include "../AVSTrans_cpp/AVSTrans_cpp.h" // for AVSTrans support, obviously ;)
#include <windows.h>
#include <stdio.h>						// for logging
#include "resource.h"
#include "avs_ape.h"

#define MOD_NAME "Misc / AVSTrans Automation"
#define UNIQUEIDSTRING "Misc: AVSTrans Automation"

// extended APE api support
APEinfo *g_extinfo;
extern "C"
{
  void __declspec(dllexport) _AVS_APE_SetExtInfo(HINSTANCE hDllInstance, APEinfo *ptr)
  {
    g_extinfo = ptr;
  }
}

class C_THISCLASS : public C_RBASE
{
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual char *get_desc();
		
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
		
		bool itsme;				// determines whether the current component is the first created component
};

struct mylist {
	int value;
	mylist *next;
};

mylist *root=NULL;
int listcount=0;

int addtolist(int newentry) {
	mylist *newroot = new mylist();
	newroot->value=newentry;
	newroot->next=root;
	root=newroot;
	return ++listcount;
}

int isinlist(int val) {
	mylist *tmp = root;
	int num = listcount;
	while(tmp) {
		if(val == tmp->value) {
			return num;
		}
		tmp = tmp->next;
		num--;
	}
	return 0;
}

void clearelem(mylist *bla) {
	if(bla) {
		clearelem(bla->next);
		delete[] bla;
	}
}

void clearlist() {
	clearelem(root);
	root=NULL;
	listcount=0;
}

class Chwnd {
	protected:	

	public:
		HWND hwnd;
		int width, height, left, top;
		Chwnd(HWND hw) {
			hwnd = hw;
		}
		void setpos() {
			if (hwnd) {
				WINDOWPLACEMENT wp;
				wp.length					= sizeof(wp);
				GetWindowPlacement(hwnd,&wp);
				if ((GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE)==0) {
					//not visible
					wp.showCmd				= SW_HIDE;
				} else {
					//visible
					wp.showCmd				= SW_SHOW;
				}
				wp.rcNormalPosition.left	= left;
				wp.rcNormalPosition.top		= top;
				wp.rcNormalPosition.right	= left+width;
				wp.rcNormalPosition.bottom	= top +height;
				SetWindowPlacement(hwnd,&wp);
			}
		}
		void getpos() {
			if (hwnd) {
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				GetWindowPlacement(hwnd,&wp);
				left	= wp.rcNormalPosition.left;
				top		= wp.rcNormalPosition.top;
				width	= wp.rcNormalPosition.right -wp.rcNormalPosition.left;
				height	= wp.rcNormalPosition.bottom-wp.rcNormalPosition.top;
			}
		}
		void refresh() {
			HDC hDC = GetWindowDC(hwnd);
			SendMessage(hwnd, WM_PAINT, (WPARAM)hDC, 0);
			ReleaseDC(hwnd, hDC);
		}

		HWND GetDlgItemNum(int DlgItemNum) {
			ECP_tmphwnd		= 0;
			ECP_cnt			= 0;
			ECP_DlgItemNum	= DlgItemNum;
			EnumChildWindows(hwnd, &ECP_GetDlgItemNum_priv, (LPARAM)this);
			return ECP_tmphwnd;
		}
		char *GetWindowCaption() {
			int r;
			if ((hwnd) && (r = GetWindowTextLength(hwnd))) {
				LPTSTR buffer = new char[r+1];
				r = GetWindowText(hwnd, buffer, r+1);
				return buffer;
			} else {
				LPTSTR buffer = new char[1];
				*buffer = 0;
				return buffer;
			}
		}
		BOOL ECP_GetDlgItemNum(HWND childhwnd) {
			ECP_cnt++;
			if (ECP_DlgItemNum == ECP_cnt) {
				ECP_tmphwnd = childhwnd;
			}
			return true;
		}

	private:
		static BOOL CALLBACK ECP_GetDlgItemNum_priv(HWND hwnd, LPARAM lParam) {
			return ((Chwnd *)lParam)->ECP_GetDlgItemNum(hwnd);
		}
		int ECP_DlgItemNum;
		HWND ECP_tmphwnd;
		int ECP_cnt;
};

// global configuration dialog pointer
static C_THISCLASS *g_ConfigThis;
// global DLL instance pointer (not needed in this example, but could be useful)
extern HINSTANCE g_hDllInstance;

int enabled_avstrans	= 0;
int enabled_log			= 0;
char *logpath = NULL;


#define defMode			  mExec
#define defFilterComments 1
bool defTransFirst		= false;
int ReadCommentCodes	= 1;

int hacked=0;
bool notify=true;

void *tmp_eip_inner;
void *tmp_eip_outer;
void *tmp_ctx;
void *patchloc;
char *tmp_expression;
char *newbuf;
DWORD oldaccess;

/*
unsigned short *acp2unicode(char *InputString) {
	int oldlen = strlen(InputString)+1;
	int newlen = MultiByteToWideChar(CP_THREAD_ACP,0,InputString,oldlen,NULL,0);
	unsigned short *buffer = new unsigned short[newlen];
	MultiByteToWideChar(CP_THREAD_ACP,0,InputString,oldlen,buffer,newlen);
	//delete[] InputString;
	return buffer;
}

char *unicode2acp(unsigned short *InputString) {
	int oldlen = wcslen(InputString)+1;
	int newlen = WideCharToMultiByte(CP_THREAD_ACP,0,InputString,oldlen,NULL,0,NULL,NULL);
	char *buffer = new char[newlen];
	WideCharToMultiByte(CP_THREAD_ACP,0,InputString,oldlen,buffer,newlen,NULL,NULL);
	//delete[] InputString;
	return buffer;
}
*/

int compnum(int ctx) {
	int pos = isinlist(ctx);
	if (pos) {
		return pos;
	} else {
		return addtolist(ctx);
	}
}

char *event_compile_start(int ctx, char *_expression) {
	static int lastctx=0;
	static int num=0;
	if (_expression) {
		if (enabled_log) {
			if (ctx==lastctx) {
				num = (num%4)+1;
			} else {
				num=1;
				lastctx=ctx;
			}
			char filename[200];
			sprintf(filename,"%s\\comp%02dpart%d.log",logpath,compnum(ctx),num);
			FILE *logfile = fopen(filename, "w");
			if (logfile) {
				fprintf(logfile, "%s", _expression);
				fclose(logfile);
			} else {
				if (!CreateDirectory(logpath, NULL)) {
					MessageBox(NULL,"Could not create log directory, deactivating Code Logger.","Code Logger Error",0);
					enabled_log = 0;
				}
			}
		}
		if (enabled_avstrans && (strncmp(_expression, "//$notrans", 10)!=0)) {
			std::string tmp = translate(_expression, defMode, defTransFirst);
			newbuf = new char[tmp.size()+1];
			strcpy(newbuf,tmp.c_str());
			return newbuf;
		}
	}
	return _expression;
}

void event_compile_done() {
	if (enabled_avstrans) {
		delete[] newbuf;
	}
}

__declspec(naked) void hook_after() {
	__asm {
		pushad
		call event_compile_done
		popad
		jmp tmp_eip_outer
	}
}

__declspec(naked) void hook_before() {
	__asm {
		pop tmp_eip_inner
		pop tmp_eip_outer
		pop tmp_ctx
		pop tmp_expression
		
		push tmp_expression
		push tmp_ctx
		call event_compile_start
		add esp,8
		push eax

		push tmp_ctx
		mov eax,offset hook_after
		push eax
		
		//stuff from the original proc
/*01CEF330 83 EC 18   */ sub         esp,18h
/*01CEF333 53         */ push        ebx
/*01CEF334 8B 5C 24 20*/ mov         ebx,dword ptr [esp+20h]
		
		jmp tmp_eip_inner
	}
}

__declspec(naked) void h00kit() {
	__asm {
		// get address
		mov eax,dword ptr [g_extinfo]
		mov eax,dword ptr [eax+1Ch]
		
		mov patchloc,eax
		pushad
	}
	
	VirtualProtect(patchloc, 8, PAGE_EXECUTE_READWRITE, &oldaccess);
	
	__asm {
		popad
		
		// modify stuff
		mov byte ptr[eax],0xB8		// MOV eax,i32
		lea ebx,hook_before
		mov dword ptr[eax+1],ebx	// i32=hook_before
		mov byte ptr[eax+5],0xFF	// CALL r32
		mov byte ptr[eax+6],0xD0	// r32=eax
		mov byte ptr[eax+7],0x90	// NOP
		pushad
	}
	
	VirtualProtect(patchloc, 8, oldaccess, &oldaccess);
	
	__asm {
		popad
		
		ret
	}
}

__declspec(naked) void unh00kit() {
	__asm pushad
	
	VirtualProtect(patchloc, 8, PAGE_EXECUTE_READWRITE, &oldaccess);
	
	__asm {
		popad
		mov eax,patchloc
		mov byte ptr[eax],0x83		// \ 
		mov byte ptr[eax+1],0xEC	//  >-- sub esp,18h
		mov byte ptr[eax+2],0x18	// /
		
		mov byte ptr[eax+3],0x53	// push ebx
		
		mov byte ptr[eax+4],0x8B	// \ 
		mov byte ptr[eax+5],0x5C	//  \__ mov ebx,dword ptr [esp+20h]
		mov byte ptr[eax+6],0x24	//  /
		mov byte ptr[eax+7],0x20	// /
		pushad
	}
	
	VirtualProtect(patchloc, 8, oldaccess, &oldaccess);
	
	__asm {
		popad
		ret
	}
}

// set up default configuration
C_THISCLASS::C_THISCLASS() {
	//get module file name into string apepath
	char *c_apepath = new char[MAX_PATH];
	GetModuleFileName(g_hDllInstance,c_apepath,MAX_PATH);
	apepath = c_apepath;
	delete[] c_apepath;
	
	//remove the file name from the path
	apepath.erase(apepath.rfind('\\'));

	if(notify) {
		notify=false;
		enabled_avstrans	= 0;
		enabled_log			= 0;
		
		defTransFirst		= 0;
		ReadCommentCodes	= 1;
		autoprefix[this] = "";
		
		itsme=true;
		
		string filename = apepath + "\\eeltrans.ini";
		FILE *ini = fopen(filename.c_str(), "r");
		if (logpath) delete[] logpath;
		if (ini) {
			char *tmp = new char[2048];
			fscanf(ini, "%s", tmp);
			logpath = new char[strlen(tmp) + 1];
			strcpy(logpath, tmp);
			fclose(ini);
		} else {
			logpath = new char[10];
			strcpy(logpath, "C:\\avslog");
		}
		
		h00kit();
	} else {
		itsme=false;
	}
	hacked++;
}

// virtual destructor
C_THISCLASS::~C_THISCLASS()
{
	hacked--;
	if (itsme) {
		notify = true;
		string filename = apepath + "\\eeltrans.ini";
		FILE *ini = fopen(filename.c_str(), "w");
		if (ini) {
			fprintf(ini, "%s", logpath);
			fclose(ini);
		}
	}
	if (hacked == 0) {
		autoprefix[this] = "";
		unh00kit();
		clearlist();
	}
}

// RENDER FUNCTION:
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout. this is
// used when you want to do something that you'd otherwise need to make a copy of the framebuffer.
// w and h are the-*/ width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
#ifdef _DEBUGgy
	static bool firstrun=false;
	if(!firstrun) {
		firstrun=true;
		VM_CONTEXT		vmc = g_extinfo->allocVM();
		compileContext	*ctx = (compileContext *)vmc;
		VM_CODEHANDLE	cod = g_extinfo->compileVMcode(vmc, "gmegabuf(0)");
		codeHandleType  *cht = (codeHandleType *)cod;
		char *output = new char[1024];
		memset(output,0,1024);
		
		void *foo = *((void **)(((unsigned char *)cht->code)+25));
		static double * (NSEEL_CGEN_CALL *__megabuf)(double ***,double *);
		__asm {
			mov eax,foo
			mov __megabuf,eax
		}
		void (*blub)(void) = (void (*)(void))(cht->code);
		blub();
		sprintf(output,"%X",__megabuf);
		MessageBox(NULL, output, "debug output", 0);
		delete[] output;
		g_extinfo->freeCode(cod);
		g_extinfo->freeVM(vmc);
	}
#endif
	if (notify) {
		if (!itsme) {
			notify=false;
			itsme=true;
		}
	}
	return 0;
}

// this is where we deal with the configuration screen
static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static WORD lastclwidth;
	static WORD lastclheight;
	static Chwnd *Txt_LogPath		= NULL;
	static Chwnd *Txt_AutoPrefix	= NULL;
	
	switch (uMsg) {
		case WM_INITDIALOG:
			HWND tmphwnd;
			tmphwnd = GetDlgItem(hwndDlg, IDC_EDIT_LOGPATH);
			Txt_LogPath = new Chwnd(tmphwnd);
			SetWindowText(tmphwnd, logpath);
			tmphwnd = GetDlgItem(hwndDlg, IDC_EDIT_AUTOPREFIX);
			Txt_AutoPrefix = new Chwnd(tmphwnd);
			SetWindowText(tmphwnd, autoprefix[g_ConfigThis].c_str());
			
			if (enabled_log		 ) { CheckDlgButton(hwndDlg,IDC_ENABLED_LOG		, BST_CHECKED); }
			if (enabled_avstrans ) { CheckDlgButton(hwndDlg,IDC_ENABLED_AVSTRANS, BST_CHECKED); }
			if (defTransFirst	 ) { CheckDlgButton(hwndDlg,IDC_TRANSFIRST		, BST_CHECKED); }
			if (ReadCommentCodes ) { CheckDlgButton(hwndDlg,IDC_READCOMMENTCODES, BST_CHECKED); }
			return 0;
		
		case WM_DESTROY:
			delete Txt_LogPath;
			Txt_LogPath = NULL;
			delete Txt_AutoPrefix;
			Txt_AutoPrefix = NULL;
			return 0;
		
		case WM_SIZE:
			lastclwidth		= LOWORD(lParam); // width of client area
			lastclheight	= HIWORD(lParam); // height of client area
			
			if (Txt_LogPath != NULL) {
				Txt_LogPath->getpos();
				Txt_LogPath->width	= lastclwidth - 143;
				Txt_LogPath->setpos();
			}
			
			if (Txt_AutoPrefix != NULL) {
				Txt_AutoPrefix->getpos();
				Txt_AutoPrefix->width	= lastclwidth	- 9;
				Txt_AutoPrefix->height	= lastclheight	- 145;
				Txt_AutoPrefix->setpos();
			}
			
			return 0;
		
		case WM_COMMAND:
			if (HIWORD(wParam) == EN_CHANGE) {
				if ((LOWORD(wParam) == IDC_EDIT_LOGPATH)&&(Txt_LogPath)) {
					/*char *tmp = Txt_LogPath->GetWindowCaption();
					logpath = new char[strlen(tmp)+1];
					strcpy(logpath, tmp);*/
					logpath = Txt_LogPath->GetWindowCaption();
				}
				
				if ((LOWORD(wParam) == IDC_EDIT_AUTOPREFIX)&&(Txt_AutoPrefix)) {
					autoprefix[g_ConfigThis] = Txt_AutoPrefix->GetWindowCaption();
				}
			}
			
			// see if the checkboxes are checked
			if (LOWORD(wParam) == IDC_ENABLED_LOG	  ) { enabled_log		= (IsDlgButtonChecked(hwndDlg,IDC_ENABLED_LOG	  )?1:0); }
			if (LOWORD(wParam) == IDC_ENABLED_AVSTRANS) { enabled_avstrans	= (IsDlgButtonChecked(hwndDlg,IDC_ENABLED_AVSTRANS)?1:0); }
			if (LOWORD(wParam) == IDC_TRANSFIRST	  ) { defTransFirst		= (IsDlgButtonChecked(hwndDlg,IDC_TRANSFIRST	  )?1:0); }
			if (LOWORD(wParam) == IDC_READCOMMENTCODES) { ReadCommentCodes	= (IsDlgButtonChecked(hwndDlg,IDC_READCOMMENTCODES)?1:0); }
			
			return 0;
	}
	return 0;
}

HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_ConfigThis = this;
	HWND cnf;
	if (itsme) {
		cnf = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_CONFIG),hwndParent,g_DlgProc);
	} else {
		cnf = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_CONF_ERR),hwndParent,g_DlgProc);
	}
	SetDlgItemText(cnf,IDC_VERSION,AVSTransVer);
	return cnf;
}

char *C_THISCLASS::get_desc(void)
{
	return MOD_NAME;
}

// load_/save_config are called when saving and loading presets (.avs files)
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	//if (itsme) {
		int pos=0;
		
		// always ensure there is data to be loaded
		if (len-pos >= 4) {
			enabled_avstrans = GET_INT();
			pos				+= 4;
		}
		if (len-pos >= 4) {
			enabled_log		 = GET_INT();
			pos				+= 4;
		}
		if (len-pos >= 4) {
			defTransFirst	 = (GET_INT()!=0);
			pos				+= 4;
		}
		if (len-pos >= 4) {
			ReadCommentCodes = GET_INT();
			pos				+= 4;
		}
		if (len>pos) {
			autoprefix[g_ConfigThis]=(char *)(data+pos);
		} else {
			autoprefix[g_ConfigThis]="";
		}
	/*} else {
		MessageBox(NULL, "Possible instance conflict, check your global code.", "eeltrans.ape", 0);
	}*/
}

// write configuration to data, return length. config data should not exceed 64k.
#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
	
	PUT_INT(enabled_avstrans);
	pos+=4;
	
	PUT_INT(enabled_log);
	pos+=4;
	
	PUT_INT(((int)defTransFirst));
	pos+=4;
	
	PUT_INT(ReadCommentCodes);
	pos+=4;
	
	strcpy((char *)(data+pos), autoprefix[this].c_str());
	pos+=autoprefix.size()+1;

	return pos;
}
/*
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	static bool doinit=true;
	switch(ul_reason_for_call) {
		case DLL_THREAD_ATTACH:
			if (doinit) {
				doinit=false;
				//COM_open();
			}
			break;

		case DLL_PROCESS_DETACH:
			//COM_close();
			break;
	}
    return TRUE;
}
*/
// export stuff
C_RBASE *R_RetrFunc(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) {
		strcpy(desc,MOD_NAME);
		return NULL;
	}
	//if (hacked == 0) {
		return (C_RBASE *) new C_THISCLASS();
	/*} else {
		return NULL;
	}*/
}

extern "C"
{
	__declspec (dllexport) int _AVS_APE_RetrFunc(HINSTANCE hDllInstance, char **info, int *create) // return 0 on failure
	{
		g_hDllInstance=hDllInstance;
		*info=UNIQUEIDSTRING;
		*create=(int)(void*)R_RetrFunc;
		return 1;
	}
}
