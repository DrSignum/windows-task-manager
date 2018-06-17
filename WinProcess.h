#pragma once
#include <malloc.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#define NumOfPoints (25)

typedef struct NODEPROCESS
{
	struct NODEPROCESS *Next;
	struct NODEPROCESS *Prev;
	DWORD ID;
	TCHAR ExeName[MAX_PATH];
	BOOL Choose;
} NodeProcess;

typedef struct PROCESSLIST
{
	NodeProcess *Head;
	NodeProcess *Tail;
	NodeProcess *Chosen;
	DWORD ChID;
} ProcessList;

int FreeProcessList(ProcessList *List, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}
	NodeProcess *Process = List->Head, *NextProcess = List->Head;
	for (Process = List->Head; NextProcess != NULL;)
	{
		NextProcess = Process->Next;
		free((void*)Process);
		Process = NextProcess;
	}
	List->Head = NULL;
	List->Tail = NULL;
	return 0;
}

int FindByName(ProcessList *List, TCHAR *Name, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}
	if (Name == NULL)
	{
		return 2;
	}
	NodeProcess *Process;
	size_t i, Size;
	BOOL FindFlg = FALSE;
	Size = lstrlen(Name) + 1;
	for (Process = List->Head; Process != NULL; Process = Process->Next)
	{
		for (i = 0; i < Size; i++)
		{
			if (Process->ExeName[i] == Name[i])
			{
				FindFlg = TRUE;
			}
			else
			{
				FindFlg = FALSE;
				break;
			}
		}
		if (FindFlg == TRUE)
		{
			List->Chosen = Process;
			List->ChID = Process->ID;
			List->Chosen->Choose = TRUE;
			return 0;
		}
	}
	List->Chosen = List->Head;
	List->ChID = List->Head->ID;
	List->Chosen->Choose = TRUE;
	return 0;
}

int FindByID(ProcessList *List, DWORD ID, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}
	NodeProcess *Process;
	for (Process = List->Head; Process != NULL; Process = Process->Next)
	{
		if (Process->ID == ID)
		{
			List->Chosen = Process;
			List->ChID = Process->ID;
			List->Chosen->Choose = TRUE;
			return 0;
		}
	}
	List->Chosen = List->Head;
	List->ChID = List->Head->ID;
	List->Chosen->Choose = TRUE;
	return 0;
}

int WriteLog(HANDLE CONST Log, TCHAR CONST *String, int Error)
{
	if (String == NULL)
	{
		return 1;
	}
	int i;
	BOOL Flg = FALSE;
	for (i = 0; i < 1023; i++)
	{
		if (String[i] == '%')
		{
			if ((String[i + 1] == 'd') && (Flg = FALSE))
			{
				Flg = TRUE;
			}
			else
			{
				return 2;
			}
		}
	}
	if (Flg == FALSE)
	{
		return 3;
	}
	TCHAR Buffer[1024];
	DWORD Count;
	wsprintf(Buffer, String, Error);
	if (WriteFile(Log, Buffer, lstrlen(Buffer), &Count, NULL) == FALSE)
	{
		return 4;
	}
	FlushFileBuffers(Log);
	return 0;
}

int CopyString(TCHAR *Source, TCHAR *Target, HANDLE CONST Log)
{
	if (Source == NULL)
	{
		return 1;
	}
	if (Target == NULL)
	{
		return 2;
	}
	size_t Size = lstrlen(Source) + 1;
	for (size_t i = 0; i < Size; i++)
	{
		Target[i] = Source[i];
	}
	return 0;
}

int AppEnd(ProcessList *List, DWORD ID, TCHAR *ExeName, BOOL Choose, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}
	int Ctrl;
	NodeProcess *NewNode;
	NewNode = (NodeProcess*)malloc(sizeof(NodeProcess));
	if (NewNode == NULL)
	{
		return 2;
	}
	Ctrl = CopyString(ExeName, NewNode->ExeName, Log);
	if (Ctrl != 0)
	{
		free((void*)NewNode);
		WriteLog(Log, "CopyString return: %d\n", Ctrl);
		return 3;
	}
	NewNode->Choose = Choose;
	NewNode->ID = ID;
	if (List->Head == NULL)
	{
		if (List->Tail == NULL)
		{
			List->Head = NewNode;
			List->Tail = NewNode;
			NewNode->Next = NULL;
			NewNode->Prev = NULL;
		}
		else
		{
			free((void*)NewNode);
			return 4;
		}
	}
	else
	{
		NewNode->Prev = List->Tail;
		List->Tail->Next = NewNode;
		List->Tail = NewNode;
		NewNode->Next = NULL;
	}
	return 0;
}

int GetProcessList(ProcessList **List, DWORD Access, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}

	PROCESSENTRY32 PendingProcess;
	BOOL Test;
	HANDLE TestHandle;
	HANDLE CONST ListOfProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	int Ctrl;
	if (ListOfProcess == INVALID_HANDLE_VALUE)
	{
		return 2;
	}
	if (*List == NULL)
	{
		*List = (ProcessList*)malloc(sizeof(ProcessList));
		if (*List == NULL)
		{
			return 3;
		}
		(*List)->Head = NULL;
		(*List)->Tail = NULL;
	}

	PendingProcess.dwSize = sizeof(PROCESSENTRY32);

	for (Test = Process32First(ListOfProcess, &PendingProcess);
		Test != FALSE;
		Test = Process32Next(ListOfProcess, &PendingProcess))
	{
		TestHandle = OpenProcess(Access, FALSE, PendingProcess.th32ProcessID);
		if ((TestHandle != NULL) || (Access == 0))
		{
			Ctrl = AppEnd(*List, PendingProcess.th32ProcessID, PendingProcess.szExeFile, FALSE, Log);
			if (Ctrl != 0)
			{
				FreeProcessList(*List, Log);
				free((void*)(*List));
				WriteLog(Log, "AppEnd return:", Ctrl);
				return 4;
			}
		}
	}
	FindByID(*List, (*List)->ChID, Log);
	if (CloseHandle(ListOfProcess) == 0)
	{
		return 5;
	}
	return 0;
}

int PrintProcessPath(HANDLE CONST CoutHandle, NodeProcess *Process, HANDLE CONST Log)
{
	TCHAR Buffer[1024];
	DWORD BufLen;
	HANDLE ProcessHandle;
	ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, Process->ID);
	if (ProcessHandle == NULL)
	{
		return 0;
	}
	BufLen = GetModuleFileNameEx(ProcessHandle, NULL, Buffer, 1024);
	if (BufLen == 0)
	{
		return 2;
	}
	Buffer[BufLen] = '\0';
	if (WriteConsole(CoutHandle, Buffer, lstrlen(Buffer), NULL, NULL) == 0)
	{
		return 3;
	}
	return 0;
}

int PrintProcess(HANDLE CONST CoutHandle, ProcessList *List, int SortMode, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}
	system("cls");
	TCHAR Buffer[MAX_PATH * 2] = { '\0' };
	NodeProcess *Process;
	size_t i, k;
	BOOL Flg = FALSE;

	switch (SortMode)
	{
	case 0:
		if (WriteConsole(CoutHandle, "whithout sorting:\n", lstrlen("whithout sorting:\n"), NULL, NULL) == 0)
		{
			return 4;
		}
		break;
	case 1:
		if (WriteConsole(CoutHandle, "sorting by id:\n", lstrlen("sorting by id:\n"), NULL, NULL) == 0)
		{
			return 4;
		}
		break;
	case 2:
		if (WriteConsole(CoutHandle, "sorting by name:\n", lstrlen("sorting by name:\n"), NULL, NULL) == 0)
		{
			return 4;
		}
		break;
	default:
		if (WriteConsole(CoutHandle, "whithout sorting:\n", lstrlen("whithout sorting:\n"), NULL, NULL) == 0)
		{
			return 4;
		}
		break;
	}

	Process = List->Chosen;
	for (i = 0; i < (NumOfPoints / 2) + 1; i++)
	{
		if (Process->Next == NULL)
		{
			break;
		}
		Process = Process->Next;
	}

	Process = List->Chosen;
	for (k = 0; k < NumOfPoints - i; k++)
	{
		if (Process->Prev == NULL)
		{
			break;
		}
		Process = Process->Prev;
	}


	for (i = 0; i < NumOfPoints; i++)
	{
		if (Process->Choose == FALSE)
		{
			if (wsprintf(Buffer, "   %05X %s\n", Process->ID, Process->ExeName) != (lstrlen(Buffer)))
			{
				return 2;
			}
		}
		else
		{
			if (wsprintf(Buffer, ">> %05X %s\n", Process->ID, Process->ExeName) != (lstrlen(Buffer)))
			{
				return 3;
			}
		}
		if (WriteConsole(CoutHandle, Buffer, lstrlen(Buffer), NULL, NULL) == 0)
		{
			return 4;
		}
		Process = Process->Next;
		if (Process == NULL)
		{
			return 5;
		}
	}
	return 0;
}

int KillProcess(NodeProcess Target, char Mode, HANDLE CONST Log)
{
	HANDLE TargetHandle = OpenProcess(PROCESS_TERMINATE, FALSE, Target.ID);
	if (TargetHandle == NULL)
	{
		return 1;
	}
	TCHAR Buffer[1024];

	switch (Mode)
	{
	case 1:
		if (TerminateProcess(TargetHandle, 0) == 0)
		{
			return 2;
		}
		break;
	case 2:
		wsprintf(Buffer, "taskkill /PID %d\0", Target.ID);
		system(Buffer);
	default:
		break;
	}
	if (CloseHandle(TargetHandle) == 0)
	{
		return 3;
	}
	return 0;
}
