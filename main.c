/******************************************************************************
* MUI_CustomClass2 - CustomClass made from existing MUI-Classes
* (C)2022 M.Volkel (mario.volkel@outlook.com)
*******************************************************************************/

// Comment templates

/******************************************************************************
*
*******************************************************************************/

/*-----------------------------------------------------------------------------
-
------------------------------------------------------------------------------*/

/******************************************************************************
* Header-Files
*******************************************************************************/
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#include <exec/memory.h>
#include <libraries/mui.h>
#include <proto/exec.h>

#include <pragma/muimaster_lib.h>
#include <pragma/utility_lib.h>

/******************************************************************************
* Macros
*******************************************************************************/
#define HOOKPROTONH(name, ret, obj, param) ret name(register __a2 obj, register __a1 param)
#define MakeHook(hookName, hookFunc) struct Hook hookName = {{NULL, NULL}, (HOOKFUNC)hookFunc, NULL, NULL}
#define DISPATCHER(name) LONG name(register __a0 Class *cl, register __a2 Object *obj, register __a1 Msg msg)

/******************************************************************************
* Prototypes
*******************************************************************************/
HOOKPROTONH(ButtonFunc, ULONG, Object *obj, APTR *msg);
DISPATCHER(SampleDispatcher);

void init(void);
void end(void);
struct ObjApp * CreateApp(void);
void DisposeApp(struct ObjApp * ObjectApp);

/******************************************************************************
* Definitions
*******************************************************************************/
#define MAKE_ID(a, b, c, d) ((ULONG)(a) << 24 | (ULONG)(b) << 16 | (ULONG)(c) << 8 | (ULONG)(d))

struct ObjApp
{
	APTR	App;
	APTR	WI_label_0;
	APTR	BT_label_0;
};

/******************************************************************************
* Global Variables
*******************************************************************************/
struct IntuitionBase *IntuitionBase;
struct Library *MUIMasterBase;
struct Library *UtilityBase;

MakeHook(hook_button, ButtonFunc);

char buffer[40];
struct ObjApp *App = NULL;

struct MUI_CustomClass *mcc;

/******************************************************************************
* Hooks
*******************************************************************************/
/*-----------------------------------------------------------------------------
- ButtonFunc()
- Hook for Button press
------------------------------------------------------------------------------*/
HOOKPROTONH(ButtonFunc, ULONG, Object *obj, APTR *msg)
{
    Object *bt, *str;
        
    bt = (Object *) *msg++;
    str = (Object *) *msg;
                
    set(str, MUIA_String_Contents, "Button clicked...");
    set(bt, MUIA_Text_Contents, "Clicked!");
    set(bt, MUIA_Disabled, TRUE);

	return 0;
}

/******************************************************************************
* MUI-Custom-Class
*******************************************************************************/
/*-----------------------------------------------------------------------------
- Definitions/Variables
------------------------------------------------------------------------------*/
#define MUI_CLASS_TUTORIAL (TAG_USER | 0x80420000)
#define MUIA_MUIClassTutorial_TextStr MUI_CLASS_TUTORIAL + 1
#define MUIA_MUIClassTutorial_LabelBut MUI_CLASS_TUTORIAL + 2

struct MyData
{
	Object *button, *str;
	STRPTR labelButton, labelStr;
};

/*-----------------------------------------------------------------------------
- OM_NEW
------------------------------------------------------------------------------*/
ULONG DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...)
{
	return (DoSuperMethod(cl, obj, OM_NEW, &tag1, NULL));
}

ULONG myNew(struct IClass *cl, Object *obj, Msg msg)
{
	STRPTR labelStr, labelButton;
	Object *str, *button;
	struct MyData *data;
    struct TagItem *tags;

    tags = ((struct opSet *)msg)->ops_AttrList;
    labelStr = (STRPTR)GetTagData(MUIA_MUIClassTutorial_TextStr, (ULONG)" ", tags);
    labelButton = (STRPTR)GetTagData(MUIA_MUIClassTutorial_LabelBut, (ULONG)" ", tags);

	str = MUI_NewObject(MUIC_String,
						MUIA_Frame, MUIV_Frame_String,
						MUIA_String_Contents, labelStr,
						TAG_DONE);

	button = MUI_MakeObject(MUIO_Button, labelButton, TAG_DONE);

    obj = (Object *) DoSuperNew(cl, obj,
                                    MUIA_Group_Child, str,
                                    MUIA_Group_Child, button,
                                	TAG_MORE, ((struct opSet *)msg)->ops_AttrList);

	if (obj == NULL)
		return 0;

	data = (struct MyData *)INST_DATA(cl, obj);
	data->str = str;
	data->button = button;
	data->labelStr = labelStr;
	data->labelButton = labelButton;

	DoMethod(data->button, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Self, 4, MUIM_CallHook, &hook_button, data->button, data->str);

	return((ULONG)obj);
}

/*-----------------------------------------------------------------------------
- Dispatcher
------------------------------------------------------------------------------*/
DISPATCHER(SampleDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW:
			return myNew(cl, obj, (APTR)msg);
	}

	return DoSuperMethodA(cl, obj, msg);
}

/******************************************************************************
* Main-Program
*******************************************************************************/

/*-----------------------------------------------------------------------------
- main()
------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	BOOL running = TRUE;
	ULONG signal;

	init();

	if (mcc = (struct MUI_CustomClass *)MUI_CreateCustomClass(NULL, MUIC_Group, NULL, sizeof(struct MyData), SampleDispatcher))
	{
		if (!(App = CreateApp()))
		{
			printf("Can't Create App\n");
			end();
		}

		while (running)
		{
			switch (DoMethod(App->App, MUIM_Application_NewInput, &signal))
			{
				// Window close
				case MUIV_Application_ReturnID_Quit:
					if ((MUI_RequestA(App->App, 0, 0, "Quit?", "_Yes|_No", "\33cAre you sure?", 0)) == 1)
						running = FALSE;
				break;

				default:
					break;
			}

			if (running && signal)
				Wait(signal);
		}

		DisposeApp(App);
	}
	MUI_DeleteCustomClass(mcc);
	end();
}

/*-----------------------------------------------------------------------------
- init()
------------------------------------------------------------------------------*/
void init(void)
{
	if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 37)))
	{
		printf("Can't Open Intuition Library\n");
		exit(20);
	}

	if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN)))
	{
		printf("Can't Open MUIMaster Library\n");
		CloseLibrary((struct Library *)IntuitionBase);
		exit(20);
	}

	if (!(UtilityBase = OpenLibrary("utility.library", 39)))
	{
		printf("Can't Open Utility Library\n");
		CloseLibrary((struct Library *)MUIMasterBase);
		CloseLibrary((struct Library *)IntuitionBase);
		exit(20);
	}
}

/*-----------------------------------------------------------------------------
- end()
------------------------------------------------------------------------------*/
void end(void)
{
	CloseLibrary((struct Library *)UtilityBase);
	CloseLibrary((struct Library *)MUIMasterBase);
	CloseLibrary((struct Library *)IntuitionBase);
	exit(0);
}

/*-----------------------------------------------------------------------------
- CreateApp()
------------------------------------------------------------------------------*/
struct ObjApp * CreateApp(void)
{
	struct ObjApp * ObjectApp;

	APTR GROUP_ROOT_0, mcc1;

	if (!(ObjectApp = AllocVec(sizeof(struct ObjApp), MEMF_CLEAR)))
		return(NULL);

	mcc1 = NewObject(mcc->mcc_Class, NULL,
					MUIA_MUIClassTutorial_TextStr, (ULONG)"Click the button...",
					MUIA_MUIClassTutorial_LabelBut, (ULONG)"Click me!",
					TAG_DONE);

	ObjectApp->BT_label_0 = SimpleButton("Exit");

	GROUP_ROOT_0 = GroupObject,
		MUIA_Group_Columns,		1,
		Child,					mcc1,
		Child,					ObjectApp->BT_label_0,
	End;

	ObjectApp->WI_label_0 = WindowObject,
		MUIA_Window_Title,		"MUI_CClass2",
		MUIA_Window_ID,			MAKE_ID('0', 'W', 'I', 'N'),
		WindowContents,			GROUP_ROOT_0,
	End;

	ObjectApp->App = ApplicationObject,
		MUIA_Application_Author,		"M.Volkel",
		MUIA_Application_Base,			"MUICCLASS",
		MUIA_Application_Title,			"MUI_CustomClass2",
		MUIA_Application_Version,		"$VER: MUI_CustomClass2 V0.1",
		MUIA_Application_Copyright,		"(C)2022 M.Volkel",
		MUIA_Application_Description,	"MUI-CustomClass2",
		SubWindow,						ObjectApp->WI_label_0,
	End;


	if (!ObjectApp->App)
	{
		FreeVec(ObjectApp);
		return(NULL);
	}

	// Window-Close-Method
	DoMethod(ObjectApp->WI_label_0, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, ObjectApp->App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	// Exit-Button
	DoMethod(ObjectApp->BT_label_0, MUIM_Notify, MUIA_Pressed, MUIV_EveryTime, ObjectApp->App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
	
	// Open Window
	set(ObjectApp->WI_label_0, MUIA_Window_Open, TRUE);

	return(ObjectApp);
}

/*-----------------------------------------------------------------------------
- DisposeApp()
------------------------------------------------------------------------------*/
void DisposeApp(struct ObjApp * ObjectApp)
{
	MUI_DisposeObject(ObjectApp->App);
	FreeVec(ObjectApp);
}
