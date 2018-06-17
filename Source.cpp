#include "WinProcess.h"
#include "SortWinProcess.h"
#include <conio.h>

typedef enum SORTMODE
{
	NO_SORT, BY_ID, BY_NAME
} SortMode;

int PushUp(ProcessList *List, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}
	if (List->Chosen->Prev != NULL)
	{
		List->Chosen->Choose = 0;
		List->Chosen = List->Chosen->Prev;
		List->Chosen->Choose = 1;
		List->ChID = List->Chosen->ID;
	}
	return 0;
}

int PushDown(ProcessList *List, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}
	if (List->Chosen->Next != NULL)
	{
		List->Chosen->Choose = 0;
		List->Chosen = List->Chosen->Next;
		List->Chosen->Choose = 1;
		List->ChID = List->Chosen->ID;
	}
	return 0;
}

int PushF(ProcessList *List, HANDLE CONST CinHandle, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}
	TCHAR UserInput[MAX_PATH];
	DWORD NumInput;
	int Ctrl;

	if (ReadConsole(CinHandle, UserInput, MAX_PATH - 1, &NumInput, NULL) == 0)
	{
		return 2;
	}
	UserInput[NumInput - 2] = ('\0');
	
	Ctrl = FindByName(List, UserInput, Log);
	if (Ctrl != 0)
	{
		WriteLog(Log, "FindByName return: %d\n", Ctrl);
		return 3;
	}
	
	List->ChID = List->Chosen->ID;
	List->Chosen->Choose = 1;
	return 0;
}

int PushI(ProcessList *List, BOOL *FlgInf, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}
	if (FlgInf == NULL)
	{
		return 2;
	}

	if (*FlgInf == FALSE)
	{
		*FlgInf = TRUE;
	}
	else
	{
		*FlgInf = FALSE;
	}
	return 0;
}

int PushS(SortMode *SMode, HANDLE CONST Log)
{
	if (SMode == NULL)
	{
		return 1;
	}
	switch (*SMode)
	{
	case NO_SORT:
		*SMode = BY_ID;
		break;
	case BY_ID:
		*SMode = BY_NAME;
		break;
	case BY_NAME:
		*SMode = NO_SORT;
		break;
	default:
		*SMode = NO_SORT;
		break;
	}
	return 0;
}

int GetPush(ProcessList *List, BOOL *FlgBreak, BOOL *FlgContinue, BOOL *FlgInf,
			SortMode *SMode, HANDLE CONST CinHandle, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}
	if ((FlgBreak == NULL) || (FlgContinue == NULL) || (FlgInf == NULL))
	{
		return 2;
	}
	if (SMode == NULL)
	{
		return 3;
	}

	int Ctrl;
	switch (_getch())
	{
	case 72:        //UP
		Ctrl = PushUp(List, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "PushUp return: %d\n", Ctrl);
			return 4;
		}
		break;
	case 80:        //DOWN
		Ctrl = PushDown(List, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "PushUp return: %d\n", Ctrl);
			return 5;
		}
		break;
	case 27:        //Escape
		*FlgBreak = TRUE;
		break;
	case '1':
		Ctrl = KillProcess(*(List->Chosen), 1, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "KillProcess return: %d\n", Ctrl);
			return 6;
		}
		break;
	case '2':
		Ctrl = KillProcess(*(List->Chosen), 2, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "KillProcess return: %d\n", Ctrl);
			return 7;
		}
		break;
	case 'f':
	case 'F':
		Ctrl = PushF(List, CinHandle, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "PuchF return: %d\n", Ctrl);
			return 8;
		}
		break;
	case 'i':
	case 'I':
		Ctrl = PushI(List, FlgInf, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "PushI return: %d\n", Ctrl);
			return 9;
		}
		break;
	case 's':
	case 'S':
		Ctrl = PushS(SMode, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "PushS return: %d\n", Ctrl);
			return 10;
		}
		break;
	default:
		*FlgContinue = TRUE;
		break;
	}
	return 0;
}

int GetList(ProcessList *List, SortMode SMode, DWORD Access, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}

	int Ctrl;
	switch (SMode)
	{
	case NO_SORT:
		Ctrl = GetProcessList(&List, Access, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "GetProcessList return: %d\n", Ctrl);
			return 2;
		}
		break;
	case BY_ID:
		Ctrl = GetSortIDList(&List, Access, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "GetSortByIdList return: %d\n", Ctrl);
			return 3;
		}
		break;
	case BY_NAME:
		Ctrl = GetSortByNameList(&List, Access, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "GetSortByNameList return: %d\n", Ctrl);
			return 4;
		}
	default:
		break;
	}
	return 0;
}

int RunList(HANDLE CONST CoutHandle, HANDLE CONST CinHandle, DWORD Access, HANDLE CONST Log)
{
	ProcessList *List = NULL;
	NodeProcess *NewChProcess;
	BOOL FlgInf = FALSE;
	BOOL FlgBreak = FALSE;
	BOOL FlgContinue = FALSE;
	SortMode SMode = NO_SORT;
	int Ctrl;

	Ctrl = GetProcessList(&List, Access, Log);
	if (Ctrl != 0)
	{
		WriteLog(Log, "PrintProcess return: %d\n", Ctrl);
		return 1;
	}
	Ctrl = PrintProcess(CoutHandle, List, SMode, Log);
	if (Ctrl != 0)
	{
		WriteLog(Log, "PrintProcess return: %d\n", Ctrl);
		return 2;
	}

	while (FlgBreak == 0)
	{
		Ctrl = GetPush(List, &FlgBreak, &FlgContinue, &FlgInf, &SMode, CinHandle, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "GetPush return: %d\n", Ctrl);
			return 3;
		}

		if (FlgBreak == TRUE)
		{
			break;
		}

		if (FlgContinue == TRUE)
		{
			FlgContinue = 0;
			continue;
		}

		Ctrl = FreeProcessList(List, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "FreeProcessList return: %d\n", Ctrl);
			return 8;
		}

		Ctrl = GetList(List, SMode, Access, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "GetList return: %d\n", Ctrl);
		}
		
		Ctrl = PrintProcess(CoutHandle, List, SMode, Log);
		if (Ctrl != NULL)
		{
			WriteLog(Log, "PrintProcess return: %d\n", Ctrl);
			return 12;
		}

		if (FlgInf == TRUE)
		{
			Ctrl = PrintProcessPath(CoutHandle, List->Chosen, Log);
			if (Ctrl != 0)
			{
				WriteLog(Log, "PrintProcessPath return: %d\n", Ctrl);
				return 13;
			}
		}
	}
	return 0;
}

int Menu(HANDLE CONST CoutHandle, DWORD *Access)
{
	if (Access == NULL)
	{
		return 1;
	}
	TCHAR CONST *Buffer = "To see list off processes, tap\n"
		"0 - to see all process\n"
		"1 - list of readable processe\n"
		"2 - list of breakable processe\n"
		"Or pres Esc to exit\n\n"
		"When you see the list, you can tap\n"
		"f - to find processe by it's name\n"
		"i - to see the path of chosen process\n"
		"1 - to break the process\n"
		"2 - to break the process by alternative way";
	system("cls");
	WriteConsole(CoutHandle, Buffer, lstrlen(Buffer), NULL, NULL);
	char FlgBreak = 0;
	while (FlgBreak == 0)
	{
		switch (_getch())
		{
		case '0':
			*Access = 0;
			FlgBreak = 1;
			break;
		case '1':
			*Access = PROCESS_QUERY_INFORMATION;
			FlgBreak = 1;
			break;
		case '2':
			*Access = PROCESS_TERMINATE;
			FlgBreak = 1;
			break;
		case 27:		//Escape
			return 2;
			break;
		default:
			break;
		}
	}
	return 0;
}

int main()
{
	HANDLE CONST CoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE CONST CinHandle = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE CONST Log = CreateFile("LogFile.txt", GENERIC_WRITE, 0, NULL,
								CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD Access;
	int Ctrl;
	WriteFile(Log, "Start\n", lstrlen("Start\n"), NULL, NULL);
	FlushFileBuffers(Log);
	while (Menu(CoutHandle, &Access) == 0)
	{
		Ctrl = RunList(CoutHandle, CinHandle, Access, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "RunList return %d\n", Ctrl);
			return Ctrl;
		}
	}
	WriteFile(Log, "Finish\n", lstrlen("Finish\n"), NULL, NULL);
	FlushFileBuffers(Log);
	CloseHandle(Log);
	return 0;
}
