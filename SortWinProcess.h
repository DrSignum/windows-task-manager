#pragma once
#include "WinProcess.h"

typedef struct TREENODE
{
	struct TREENODE *Left;
	struct TREENODE *Right;
	DWORD ID;
	TCHAR ExeName[MAX_PATH];
	BOOL Choose;
} BinTreeNode;

typedef enum COMPARE
{
	LESS,
	EQUAL,
	MORE
} Compare;

void FreeBinTree(BinTreeNode *Root)
{
	if (Root != NULL)
	{
		FreeBinTree(Root->Left);
		FreeBinTree(Root->Right);
		free((void*)Root);
	}
}

int AddIDBinTreeNode(BinTreeNode **Root, DWORD ID, TCHAR *Name, BOOL Choose, HANDLE CONST Log)
{
	int Ctrl;
	if (Root == NULL)
	{
		return 1;
	}
	if (*Root == NULL)
	{
		*Root = (BinTreeNode*)malloc(sizeof(BinTreeNode));
		if (*Root == NULL)
		{
			FreeBinTree(*Root);
			return 2;
		}
		(*Root)->ID = ID;
		Ctrl = CopyString(Name, (*Root)->ExeName, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "CopeString return: %d\n", Ctrl);
			FreeBinTree(*Root);
			return 3;
		}
		(*Root)->Choose = Choose;
		(*Root)->Left = NULL;
		(*Root)->Right = NULL;
	}
	else
	{
		if (ID < (*Root)->ID)
		{
			Ctrl = AddIDBinTreeNode(&((*Root)->Left), ID, Name, Choose, Log);
			if (Ctrl != 0)
			{
				WriteLog(Log, "AddIdBinTreeNode return: %d\n", Ctrl);
				FreeBinTree(*Root);
				return 3;
			}
		}
		else
		{
			Ctrl = AddIDBinTreeNode(&((*Root)->Right), ID, Name, Choose, Log);
			if (Ctrl != 0)
			{
				WriteLog(Log, "AddIdBinTreeNode return: %d\n", Ctrl);
				FreeBinTree(*Root);
				return 4;
			}
		}
	}
	return 0;
}

int TreeToList(BinTreeNode *Root, ProcessList *List, HANDLE CONST Log)
{
	if (Root != NULL)
	{
		TreeToList(Root->Left, List, Log);
		AppEnd(List, Root->ID, Root->ExeName, Root->Choose, Log);
		TreeToList(Root->Right, List, Log);
	}
	return 0;
}

int GetSortIDList(ProcessList **List, DWORD Access, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}

	BinTreeNode *Root = NULL;
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
			Ctrl = AddIDBinTreeNode(&Root, PendingProcess.th32ProcessID, PendingProcess.szExeFile, FALSE, Log);
			if (Ctrl != NULL)
			{
				FreeBinTree(Root);
				WriteLog(Log, "AddIdBinTreeNode return: %d\n", Ctrl);
				return 5;
			}
		}
	}
	TreeToList(Root, *List, Log);
	FindByID(*List, (*List)->ChID, Log);
	if (CloseHandle(ListOfProcess) == 0)
	{
		return 6;
	}
	return 0;
}

int CmpString(TCHAR *First, TCHAR *Second, Compare *Cmp, HANDLE CONST Log)
{
	if (First == NULL)
	{
		return 1;
	}
	if (Second == NULL)
	{
		return 2;
	}
	if (Cmp == NULL)
	{
		return 3;
	}

	size_t MinSize, i;
	size_t FirstSize = lstrlen(First);
	size_t SecondSize = lstrlen(Second);
	BOOL FlgEqual = FALSE;

	if (FirstSize < SecondSize)
	{
		MinSize = FirstSize;
	}
	else
	{
		MinSize = SecondSize;
	}
	for (i = 0; i < MinSize; i++)
	{
		if ((First[i] == '\0') || (Second[i] == '\0'))
		{
			return 4;
		}
		if (First[i] < Second[i])
		{
			*Cmp = LESS;
			return 0;
		}
		else if (First[i] == Second[i])
		{
			FlgEqual = TRUE;
		}
		else if (First[i] > Second[i])
		{
			*Cmp = MORE;
			return 0;
		}
	}
	if (FlgEqual == TRUE)
	{
		if (FirstSize < SecondSize)
		{
			*Cmp = LESS;
			return 0;
		}
		else if (FirstSize == SecondSize)
		{
			*Cmp = EQUAL;
			return 0;
		}
		else if (FirstSize > SecondSize)
		{
			*Cmp = MORE;
			return 0;
		}
	}
	return 0;
}

int AddNameBinTreeNode(BinTreeNode **Root, DWORD ID, TCHAR *Name, BOOL Choose, HANDLE CONST Log)
{
	int Ctrl = LESS;
	Compare Cmp;
	if (Root == NULL)
	{
		return 1;
	}
	if (*Root == NULL)
	{
		*Root = (BinTreeNode*)malloc(sizeof(BinTreeNode));
		if (*Root == NULL)
		{
			FreeBinTree(*Root);
			return 2;
		}
		(*Root)->ID = ID;
		Ctrl = CopyString(Name, (*Root)->ExeName, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "CopeString return: %d\n", Ctrl);
			FreeBinTree(*Root);
			return 3;
		}
		(*Root)->Choose = Choose;
		(*Root)->Left = NULL;
		(*Root)->Right = NULL;
	}
	else
	{
		Ctrl = CmpString(Name, (*Root)->ExeName, &Cmp, Log);
		if (Ctrl != 0)
		{
			WriteLog(Log, "CmpString return: %d\n", Ctrl);
			FreeBinTree(*Root);
			return 4;
		}
		if (Cmp == LESS)
		{
			Ctrl = AddIDBinTreeNode(&((*Root)->Left), ID, Name, Choose, Log);
			if (Ctrl != 0)
			{
				WriteLog(Log, "AddIdBinTreeNode return: %d\n", Ctrl);
				FreeBinTree(*Root);
				return 5;
			}
		}
		else
		{
			Ctrl = AddIDBinTreeNode(&((*Root)->Right), ID, Name, Choose, Log);
			if (Ctrl != 0)
			{
				WriteLog(Log, "AddIdBinTreeNode return: %d\n", Ctrl);
				FreeBinTree(*Root);
				return 6;
			}
		}
	}
	return 0;
}

int GetSortByNameList(ProcessList **List, DWORD Access, HANDLE CONST Log)
{
	if (List == NULL)
	{
		return 1;
	}

	BinTreeNode *Root = NULL;
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
			Ctrl = AddNameBinTreeNode(&Root, PendingProcess.th32ProcessID, PendingProcess.szExeFile, FALSE, Log);
			if (Ctrl != NULL)
			{
				FreeBinTree(Root);
				WriteLog(Log, "AddIdBinTreeNode return: %d\n", Ctrl);
				return 5;
			}
		}
	}
	TreeToList(Root, *List, Log);
	FindByID(*List, (*List)->ChID, Log);
	if (CloseHandle(ListOfProcess) == 0)
	{
		return 6;
	}
	return 0;
}

