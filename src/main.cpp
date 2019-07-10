// Sorting Algorithms for AUI
// by Amine Rehioui aka foo
// http://www.aui.ma/~A.Rehioui

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include "aui_id.h"
#include "time.h" /* time() */

// WNDCLASS FUNCS
ATOM MainWindowClassInit();

// WINDOW FUNCS
HWND MainWindowInit();

// WINDOW PROCS
LRESULT CALLBACK MainWindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MainDialogProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SimDialogProc(HWND, UINT, WPARAM, LPARAM);

// GLOBALS
HINSTANCE g_hInst;
HWND MainWindow=0; 
HWND MainDialog=0;
char *AppName = "Sorting Algorithms for AUI";
char *MainWindowClassName = "foo_class";
bool Active=false;


// SPECIFIC 
char **items=NULL; // List of strings representing data
int num_items; // Size of list

// Generate items (students) list 
//void GenerateItems(int);
void GenerateItems(char ***arr, int size);

// Sorting
void Sort(char **, int, char *, char *);

void SelectionSort(char **, int, char*(char*));

void InsertionSort(char **, int, char*(char*));

// Merge sort funcs //////////////////////////////////////////
void Merge(char **arr, int left_half_start, int left_half_size, 
		   int right_half_start, int right_half_size, char* KeyExtractor(char*));

// Actual recursive func
void MergeSort(char **arr, int left, int right, char* KeyExtractor(char*));
// end of Merge sort funcs //////////////////////

void QuickSort(char **arr, int left, int right, char* KeyExtractor(char*));

void ShellSort(char **arr, int size, char* KeyExtractor(char*));

void BubbleSort(char **arr, int size, char* KeyExtractor(char*));

// Get selected item
char *GetListSelection(HWND);
char *GetLastName(char *item);
char *GetFirstName(char *item);
char *GetID(char *item);

// Thread code
DWORD WINAPI ThreadFunc(void*);
struct ThreadData
{
  char **arr;
  int size;
  char *key;
  char *alg;
  bool *running;
};

// Simulation code
bool ThreadRunning;
HANDLE ThreadHandle;

// WINMAIN
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{	
    MSG msg;
	BOOL msg_waiting;

	g_hInst = hInstance;
	
	// creates main window
	MainWindowClassInit();	
	if (!(MainWindow=MainWindowInit())) 
	{
        MessageBox(NULL, "Can't create main window!\nAre you runing windows?!! :))", "Fatal error", MB_OK);
		return 0;
	}
	ShowWindow(MainWindow,nCmdShow);	
    
	GenerateItems(&items, 3000);
	
	DialogBox(g_hInst, MAKEINTRESOURCE(MAIN_DIALOG), MainWindow, (DLGPROC)MainDialogProc);			

	goto no_error;	

error:
	DestroyWindow(MainWindow);

no_error:

    msg.message = WM_NULL;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( msg.message != WM_QUIT )
    {
        // Use PeekMessage() if the app is active, so idle time is used to
        // render the scene. Else, use GetMessage() to avoid eating CPU time.
        if( Active )
            msg_waiting = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
        else
            msg_waiting = GetMessage( &msg, NULL, 0U, 0U );

        if( msg_waiting )
        {            
                TranslateMessage( &msg );
                DispatchMessage( &msg );            
        }
        else
        {
            if( Active )
            {                
				// render
				
            }
        }
    } // end while( msg != WM_QUIT )
 
  if(items)
  {
    for(int i=0; i<num_items; i++) delete[] items[i];
	delete[] items;
  }

  return 0;
}

// RegisterMainWindowClass
ATOM MainWindowClassInit()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)MainWindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInst;
	wcex.hIcon			= LoadIcon(NULL, (LPCTSTR)IDI_APPLICATION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(RGB(0,0,100));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= MainWindowClassName;
	wcex.hIconSm		= LoadIcon(NULL, (LPCTSTR)IDI_APPLICATION);

	return RegisterClassEx(&wcex);
}

// InitMainWindow
HWND MainWindowInit()
{
   HWND hWnd;   
   int w = GetSystemMetrics(SM_CXSCREEN),
	   h = GetSystemMetrics(SM_CYSCREEN),
	   ww = 800, wh = 600;
   
   hWnd = CreateWindow(MainWindowClassName, AppName, 
	      WS_POPUP,
		  (w-ww)/2,(h-wh)/2,1,1, NULL, 
		  /*(HMENU)*/NULL, 
		  g_hInst, NULL);  
  
   return hWnd;
}

// MainWindowProc
LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	switch (msg) 
	{
		case WM_ACTIVATE:

	    if(!(BOOL)HIWORD(wParam)/*Maximized*/&& 
		(LOWORD(wParam)==WA_ACTIVE||LOWORD(wParam)==WA_CLICKACTIVE)) Active = true;
		else Active = false;

		break;
		
		case WM_DESTROY:
			PostQuitMessage(0);
			break;	

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam); 
	}	  

	return 0; 
}

// MainDialogProc
LRESULT CALLBACK MainDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static char buf[100];
	static int time_start;
	static ThreadData *_ThreadData;
	static boolean refresh = false;
	
	switch (msg) 
	{
	case WM_INITDIALOG:

		for(int i=0; i<num_items; i++)
		SendMessage(GetDlgItem(hDlg, LIST), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)items[i]);				
		
		sprintf(buf, "Number of students: %d", num_items);
		SetDlgItemText(hDlg,DESC_NUM_STUD, (LPCSTR)buf);
		
		SendMessage(GetDlgItem(hDlg, LIST_SORT), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)"Semester (ID)");
		SendMessage(GetDlgItem(hDlg, LIST_SORT), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)"Last Name");
		SendMessage(GetDlgItem(hDlg, LIST_SORT), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)"First Name");
	    SendMessage(GetDlgItem(hDlg, LIST_SORT), LB_SETCURSEL, 0, 0);

		
		SendMessage(GetDlgItem(hDlg, LIST_ALG), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)"Merge sort");
		SendMessage(GetDlgItem(hDlg, LIST_ALG), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)"Shell sort");		
		SendMessage(GetDlgItem(hDlg, LIST_ALG), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)"Bubble sort");
		SendMessage(GetDlgItem(hDlg, LIST_ALG), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)"Selection sort");
		SendMessage(GetDlgItem(hDlg, LIST_ALG), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)"Quick sort");		
		SendMessage(GetDlgItem(hDlg, LIST_ALG), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)"Insertion sort");
		SendMessage(GetDlgItem(hDlg, LIST_ALG), LB_SETCURSEL, 0, 0);

		
		EnableWindow(GetDlgItem(hDlg, BUTTON_STOP), FALSE);

		return 1;
	break;
	case WM_COMMAND:			
			switch (LOWORD(wParam))
			{			
			    case BUTTON_SORT:
					    
							EnableWindow(GetDlgItem(hDlg, BUTTON_STOP), TRUE);

							SetDlgItemText(hDlg, BUTTON_SORT, "Sorting..");
							EnableWindow(GetDlgItem(hDlg, BUTTON_SORT), FALSE);
							EnableWindow(GetDlgItem(hDlg, BUTTON_RAND), FALSE);
							EnableWindow(GetDlgItem(hDlg, BUTTON_SIM), FALSE);

							EnableWindow(GetDlgItem(hDlg, LIST_SORT), FALSE);
							EnableWindow(GetDlgItem(hDlg, LIST_ALG), FALSE);

							ThreadRunning = true;
					
							_ThreadData = new ThreadData;
							_ThreadData->arr = items;
							_ThreadData->size = num_items;
							_ThreadData->key = GetListSelection(GetDlgItem(hDlg,LIST_SORT));
							_ThreadData->alg = GetListSelection(GetDlgItem(hDlg,LIST_ALG)); 	
							_ThreadData->running = &ThreadRunning;

							time_start = GetTickCount();	
							SetTimer(hDlg, 0, 1, 0);		
							ThreadHandle = CreateThread(0,100,&ThreadFunc,(void*)_ThreadData,0,0);
											
			    break;

				case BUTTON_RAND:
									
						GenerateItems(&items, 0);

						SendMessage(GetDlgItem(hDlg, LIST), LB_RESETCONTENT, 0, 0);	
						for(int i=0; i<num_items; i++)
						SendMessage(GetDlgItem(hDlg, LIST), LB_INSERTSTRING, (WPARAM)i, (LPARAM)(LPCSTR)items[i]);				
						
				break;

				case BUTTON_SIM:

					DialogBox(g_hInst, MAKEINTRESOURCE(SIM_DIALOG), hDlg, (DLGPROC)SimDialogProc);			

				break;

				case BUTTON_STOP:

					TerminateThread(ThreadHandle,0);
					KillTimer(hDlg,0);

					EnableWindow(GetDlgItem(hDlg, BUTTON_STOP), FALSE);

					for(int i=0; i<num_items; i++)
					SendMessage(GetDlgItem(hDlg, LIST), LB_INSERTSTRING, (WPARAM)i, (LPARAM)(LPCSTR)items[i]);				
	
					SetDlgItemText(hDlg, BUTTON_SORT, "Sort!");
					EnableWindow(GetDlgItem(hDlg, BUTTON_SORT), TRUE);
					EnableWindow(GetDlgItem(hDlg, BUTTON_RAND), TRUE);
					EnableWindow(GetDlgItem(hDlg, BUTTON_SIM), TRUE);

					EnableWindow(GetDlgItem(hDlg, LIST_SORT), TRUE);
					EnableWindow(GetDlgItem(hDlg, LIST_ALG), TRUE);

	
				break;

				case BUTTON_TECH:
				
MessageBox(hDlg,
"This program performs sorting tasks on a list of 3000 students,\nusing various soting keys (By semester, last name, and first name)\nand various sort algorithms.\nThe list is randomly generated using fake ids and names\n(see algorithms in aui_id.h). This program has also a simulation\nmode that allows to show the full range of results from all the possible\nkey/algorithm combinations.\nThis could be used to evaluate what best algorithm to use\nin sorting AUI students considering different keys.\nThat's it!\nby Amine Rehioui, http://www.aui.ma/~A.Rehioui","Tech. info",MB_OK);

				break;					
					
				case LIST_REFRESH:
					refresh = !refresh;
				break;

				case IDCANCEL:
				SendMessage(hDlg,WM_CLOSE,0,0);
				break; 
			}		
	break;

	case WM_CLOSE:
	if(MessageBox(hDlg,"Really quit ?\n","Quit?",MB_YESNO)==IDYES)	
	{
	  if(ThreadRunning) 
	  {
		TerminateThread(ThreadHandle,0);
		KillTimer(hDlg,0);
	  }
	  EndDialog(hDlg,0);
	  DestroyWindow(MainWindow);		
	}
	return 1;
	break;	

	case WM_TIMER:
		if(ThreadRunning)
		{
			if(refresh)
			{
				for(int i=0; i<num_items; i++)
				SendMessage(GetDlgItem(hDlg, LIST), LB_INSERTSTRING, (WPARAM)i, (LPARAM)(LPCSTR)items[i]);							
			}
			sprintf(buf, "sorting .. %d ms elapsed.",GetTickCount()-time_start);
			SetDlgItemText(hDlg,RESULTS,buf);
		}
		else
		{
		  EnableWindow(GetDlgItem(hDlg, BUTTON_STOP), FALSE);

		  SendMessage(GetDlgItem(hDlg, LIST), LB_RESETCONTENT, 0, 0);
		  for(int i=0; i<num_items; i++)
		  SendMessage(GetDlgItem(hDlg, LIST), LB_INSERTSTRING, (WPARAM)i, (LPARAM)(LPCSTR)items[i]);				
	
		  KillTimer(hDlg,0);
		  SetDlgItemText(hDlg, BUTTON_SORT, "Sort!");
		  EnableWindow(GetDlgItem(hDlg, BUTTON_SORT), TRUE);
		  EnableWindow(GetDlgItem(hDlg, BUTTON_RAND), TRUE);
		  EnableWindow(GetDlgItem(hDlg, BUTTON_SIM), TRUE);

		  EnableWindow(GetDlgItem(hDlg, LIST_SORT), TRUE);
		  EnableWindow(GetDlgItem(hDlg, LIST_ALG), TRUE);

		  char *_1 = GetListSelection(GetDlgItem(hDlg,LIST_SORT));
		  char *_2 = GetListSelection(GetDlgItem(hDlg,LIST_ALG));		  
		  sprintf(buf, "Sorting %s with %s made %d ms.",
			  _1,
			  _2,
			  GetTickCount()-time_start);
		  SetDlgItemText(hDlg,RESULTS,buf);
		  MessageBox(hDlg,buf,"results",MB_OK);
		  delete[] _1;
		  delete[] _2;
		}		
	break;
  }

  return 0;
}

// MainDialogProc
LRESULT CALLBACK SimDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ThreadData *_ThreadData;
	static int time_start;
	static int sim_timers[6][3];
	static char **sim_items;
	static int sim_type;
	static char sort_types[3][20] = { "Semester (ID)", "Last Name", "First Name" };
	static char sort_algs[6][20] = { "Shell sort", "Quick sort", "Merge sort",
									 "Bubble sort", "Selection sort", "Insertion sort"};
	static int current_alg;
	static char buf[200];
	
	switch (msg) 
	{
	case WM_INITDIALOG:				

		sprintf(buf, "%-20s%-25s%-25s%-25s",
					 " ",sort_types[0],sort_types[1],sort_types[2]);
		SetDlgItemText(hDlg,SIM_DESC_HEADER,buf);

		time_start = GetTickCount();
		
        _ThreadData = new ThreadData;
		sim_items = NULL;
		GenerateItems(&sim_items,num_items);
		_ThreadData->arr = sim_items;
		_ThreadData->size = num_items;							
		_ThreadData->key = new char[20]; strcpy(_ThreadData->key,sort_types[0]);
		_ThreadData->alg = new char[20]; strcpy(_ThreadData->alg,sort_algs[0]);
		_ThreadData->running = &ThreadRunning; 
		current_alg = 0;
		sim_type = 0;
		sim_timers[0][0] = GetTickCount();
		ThreadRunning = true;
		ThreadHandle = CreateThread(0,0,&ThreadFunc,(void*)_ThreadData,0,0);
		
		SetTimer(hDlg,0,1,0);
		
		return 1;
	break;
	case WM_COMMAND:			
		switch (LOWORD(wParam))
		{
		   case SIM_STOP:
			 SendMessage(hDlg,WM_CLOSE,0,0);
		  break;
		}

	break;	

	case WM_TIMER:

		sprintf(buf, "Simulation started.. %d ms elapsed", GetTickCount()-time_start);
		SetDlgItemText(hDlg,SIM_DESC_TIME,buf);		

		if(ThreadRunning)
		{					
			 sprintf(buf,"%-20s%-25d%-25d%-25d",
				 sort_algs[current_alg],
				 sim_type==0?GetTickCount()-sim_timers[current_alg][0]:sim_timers[current_alg][0],
				 sim_type<1?0:(sim_type==1?GetTickCount()-sim_timers[current_alg][1]:sim_timers[current_alg][1]),
				 sim_type<2?0:(sim_type==2?GetTickCount()-sim_timers[current_alg][2]:sim_timers[current_alg][2]));
			 SetDlgItemText(hDlg,SIM_DESC1+current_alg, buf);
		}
		else
		{
			if(sim_type < 2)
			{
				sim_timers[current_alg][sim_type] = GetTickCount()-sim_timers[current_alg][sim_type];
				sim_type++;
				sim_timers[current_alg][sim_type] = GetTickCount();

				_ThreadData = new ThreadData;
				for(int k=0; k<num_items; k++) delete[] sim_items[k];
				delete[] sim_items;
				sim_items = NULL;
				GenerateItems(&sim_items,num_items);
				_ThreadData->arr = sim_items;
				_ThreadData->size = num_items;	
				_ThreadData->key = new char[20]; strcpy(_ThreadData->key,sort_types[sim_type]);
				_ThreadData->alg = new char[20]; strcpy(_ThreadData->alg,sort_algs[current_alg]);
				_ThreadData->running = &ThreadRunning; 
				ThreadRunning = true;
				ThreadHandle = CreateThread(0,0,&ThreadFunc,(void*)_ThreadData,0,0);		
			}		
			else
			if(current_alg < 6)
			{
				sim_timers[current_alg][sim_type] = GetTickCount()-sim_timers[current_alg][sim_type];
			
				sim_type = 0;
				current_alg++;

				 _ThreadData = new ThreadData;
				for(int k=0; k<num_items; k++) delete[] sim_items[k];
				delete[] sim_items;			
				sim_items = NULL;
				GenerateItems(&sim_items,num_items);
				_ThreadData->arr = sim_items;
				_ThreadData->size = num_items;							
				_ThreadData->key = new char[20]; strcpy(_ThreadData->key,sort_types[sim_type]);
				_ThreadData->alg = new char[20]; strcpy(_ThreadData->alg,sort_algs[current_alg]);
				_ThreadData->running = &ThreadRunning; 
				sim_timers[current_alg][0] = GetTickCount();
				ThreadRunning = true;
				ThreadHandle = CreateThread(0,0,&ThreadFunc,(void*)_ThreadData,0,0);
			}
			else
			{
				for(int j=0; j<num_items; j++) delete[] sim_items[j];
					delete[] sim_items;				
				
				KillTimer(hDlg,0);
				MessageBox(hDlg,"Simulation finished!","Sim. end",MB_OK);	
			}
		}

	break;
	case WM_CLOSE:
	if(MessageBox(hDlg,"End simulation?\n(IT IS NOT RECOMMENDED TO CLOSE WHILE SORTING)",
		"Stop sim.",MB_YESNO)==IDYES)	
	{
	  if(ThreadRunning) 
	  {
		 TerminateThread(ThreadHandle,0);
		 KillTimer(hDlg,0);

		 for(int j=0; j<num_items; j++) delete[] sim_items[j];
			delete[] sim_items;		
	  }
	  EndDialog(hDlg,0);
	}	
	break;
	}

  return 0;
}

// Sorting algorithms
void Sort(char **arr, int size, char *key, char *alg)
{
  if(strcmp(key,"Semester (ID)")==0)
  {
    if(strcmp(alg,"Bubble sort")==0)
	  BubbleSort(arr,size,&GetID);  
    else if(strcmp(alg,"Selection sort")==0)
	  SelectionSort(arr,size,&GetID);  
	else if(strcmp(alg,"Insertion sort")==0)		
	  InsertionSort(arr,size,&GetID);	
	else if(strcmp(alg,"Merge sort")==0)		
	  MergeSort(arr,0,size-1,&GetID);	 
	else if(strcmp(alg,"Quick sort")==0)		
	  QuickSort(arr,0,size-1,&GetID);
	else if(strcmp(alg,"Shell sort")==0)		
	  ShellSort(arr,size,&GetID);
  }  
  else if(strcmp(key,"Last Name")==0)
  {
    if(strcmp(alg,"Bubble sort")==0)
	  BubbleSort(arr,size,&GetLastName); 
	else if(strcmp(alg,"Selection sort")==0)		
	  SelectionSort(arr,size,&GetLastName);		
	else if(strcmp(alg,"Insertion sort")==0)		
	  InsertionSort(arr,size,&GetLastName);	
	else if(strcmp(alg,"Merge sort")==0)		
	  MergeSort(arr,0,size-1,&GetLastName); 
	else if(strcmp(alg,"Quick sort")==0)		
	  QuickSort(arr,0,size-1,&GetLastName);
	else if(strcmp(alg,"Shell sort")==0)		
	  ShellSort(arr,size,&GetLastName);
  }
  else if(strcmp(key,"First Name")==0)
  {
    if(strcmp(alg,"Bubble sort")==0)
	  BubbleSort(arr,size,&GetFirstName); 
	else if(strcmp(alg,"Selection sort")==0)	
	  SelectionSort(arr,size,&GetFirstName);	
	else if(strcmp(alg,"Insertion sort")==0)		
	  InsertionSort(arr,size,&GetFirstName);		
	else if(strcmp(alg,"Merge sort")==0)		
	  MergeSort(arr,0,size-1,&GetFirstName); 
	else if(strcmp(alg,"Quick sort")==0)		
	  QuickSort(arr,0,size-1,&GetFirstName);
	else if(strcmp(alg,"Shell sort")==0)		
	  ShellSort(arr,size,&GetFirstName);
  } // else if..
}

char* GetListSelection(HWND list)
{
  int index = (int)SendMessage(list,LB_GETCURSEL,0,0);
  char *str = new char[50];
  SendMessage(list,LB_GETTEXT,(WPARAM)index,(LPARAM)(LPCSTR)str);
  return str;
}

char *GetLastName(char *item)
{
  char copy[50];
  strcpy(copy, item);
  strtok(copy, " ");
  char *n = new char[50];
  strcpy(n,strtok(NULL," "));
  return n;
}

char *GetFirstName(char *item)
{
  char copy[50];
  strcpy(copy, item);
  strtok(copy, " ");
  strtok(NULL, " ");
  char *n = new char[50];
  strcpy(n,strtok(NULL," "));
  return n;
}

char *GetID(char *item)
{
  char copy[50];
  strcpy(copy, item);
  char *n = new char[50];
  strcpy(n, strtok(copy, " "));
  return n;
}

DWORD WINAPI ThreadFunc(void *param)
{
  ThreadData *_ThreadData = (ThreadData*)param;
  
  //wr(*_ThreadData);			

  Sort(_ThreadData->arr, _ThreadData->size, _ThreadData->key, _ThreadData->alg);

  delete[] _ThreadData->key;
  delete[] _ThreadData->alg;
  *_ThreadData->running = false;
  delete _ThreadData; 
  return 0;
}

void GenerateItems(char ***arr, int size)
{
  if(*arr == NULL) 
  {
     num_items = size;
	 *arr = new char*[size*sizeof(char*)];
	 for(int i=0; i<size; i++)
		 (*arr)[i] = new char[50];
  }  

  char *_1;
  srand(time(0));

  for(int i=0; i<num_items; i++)
  {
    _1 = GetRandomIdentity();
    strcpy((*arr)[i], _1);
	delete[] _1;
  }
}

void SelectionSort(char **arr, int size, char *KeyExtractor(char*))
{
	int min;
	char temp[50];
	char *_1, *_2;
	for(int i=0; i<size-1; i++)
	{
		min = i;
		for(int j=i+1; j<size; j++)
		{
			_1 = KeyExtractor(arr[j]);
			_2 = KeyExtractor(arr[min]);
			if(strcmp(_1, _2)<0) min = j;
			delete[] _1;
			delete[] _2;
			//Sleep(1);
		}
		if(min != i)
		{
			strcpy(temp, arr[i]);
			strcpy(arr[i], arr[min]);
			strcpy(arr[min], temp);
		}
	}
}

void InsertionSort(char **arr, int size, char *KeyExtractor(char*))
{  
  char temp[50];
  char *_1, *_2;
	
  for (int i=1,j; i < size; i++)
  {
    j = i;
    strcpy(temp, arr[j]);

    while (j>0)
    {
	  _1 = KeyExtractor(arr[j-1]);
	  _2 = KeyExtractor(temp);
      if(strcmp(_2, _1)<0)
      {
        strcpy(arr[j], arr[j-1]);
        j--;
		delete[] _1;
		delete[] _2;
	  }
      else  
	  {
		delete[] _1;
		delete[] _2;
		break;
	  }
    }
    strcpy(arr[j], temp);
  }
}


// Merge sort funcs //////////////////////////////////////////
void Merge(char **arr, int left_half_start, int left_half_size, 
		   int right_half_start, int right_half_size, char* KeyExtractor(char*))
{
	char **temp = new char*[left_half_size+right_half_size]; // temp array needed
	int dest_index=0;
	int left_index = left_half_start;
	int right_index = right_half_start;
	char *_1, *_2;

	while(1)
	{
		if(left_index >= left_half_start+left_half_size)
		{
			for(int i=right_index; i<right_half_start+right_half_size; i++)
			{
				temp[dest_index] = new char[strlen(arr[i])+1];
				strcpy(temp[dest_index++],arr[i]); 
			}
			
			break;
		} // reached end of left part, break
		if(right_index >= right_half_start+right_half_size)
		{
			for(int i=left_index; i<left_half_start+left_half_size; i++) 
			{
				temp[dest_index] = new char[strlen(arr[i])+1]; 
				strcpy(temp[dest_index++],arr[i]); 
			}
			break;
		} // reached end of right part, break

		_1 = KeyExtractor(arr[left_index]);
		_2 = KeyExtractor(arr[right_index]);

		if(strcmp(_1,_2)<0)
		{
		  temp[dest_index] = new char[strlen(arr[left_index])+1];	
		  strcpy(temp[dest_index++],arr[left_index++]);
		}
		else
		{
		  temp[dest_index] = new char[strlen(arr[right_index])+1];	
		  strcpy(temp[dest_index++],arr[right_index++]);
		}

		delete[] _1;
		delete[] _2;

	} // end while

	// copy temp to original array
	for(int i=left_half_start; i<left_half_start+left_half_size+right_half_size; i++)
	{  				 
		strcpy(arr[i], temp[i-left_half_start]);
	}

	for(i=0; i<left_half_size+right_half_size; i++)	delete[] temp[i];
	delete[] temp;
}

// Actual recursive func
void MergeSort(char **arr, int left, int right, char* KeyExtractor(char*))
{
	if(right == left) return; // stop if arr has 1 elem

	int mid = (left + right) / 2;
	MergeSort(arr, left, mid, KeyExtractor); // sort left part
	MergeSort(arr, mid+1, right, KeyExtractor); // sort right part
	Merge(arr, left, mid-left+1, mid+1, right-mid, KeyExtractor); // merge them
}
// end of Merge sort funcs //////////////////////

// inspired from Qsort from James Gosling
// at www.javasoft.com
void QuickSort(char **arr, int left, int right, char* KeyExtractor(char*))
{
	int _left = left;
	int _right = right;
	char mid[50];
	char temp[50];
	char *_1, *_2;

	if ( right > left)
	{
		// take pivot from middle
		strcpy(mid, arr[(left+right)/2]);

		while( _left <= _right )
		{
			_1 = KeyExtractor(arr[_left]);
			_2 = KeyExtractor(mid);
			while( (_left<right) && strcmp(_1,_2)<0 )
			{				
				_left++;
				delete[] _1;
				_1 = KeyExtractor(arr[_left]);
			}

			delete[] _1;
			_1 = KeyExtractor(arr[_right]);
			while( ( _right>left ) && strcmp(_2,_1)<0 )
			{
				_right--;
				delete[] _1;
				_1 = KeyExtractor(arr[_right]);			
			}

			delete[] _1;
			delete[] _2;

			if( _left <= _right )
			{
				strcpy(temp, arr[_left]);
				strcpy(arr[_left], arr[_right]);
				strcpy(arr[_right], temp);
				_left++;
				_right--;
			}
		}

		if( left < _right )
			QuickSort( arr, left, _right, KeyExtractor);

		if( _left < right )
			QuickSort( arr, _left, right, KeyExtractor);
	}
}

// inspired from shell sort on 
// http://www.auto.tuwien.ac.at/~blieb/woop/shell.html 
void ShellSort(char **arr, int size, char* KeyExtractor(char*)) 
{
	int h = 1;
	char B[50];
	char *_1, *_2=NULL;
	
	while ((h * 3 + 1) < size) {  h = 3 * h + 1; }

	while( h > 0 ) 
	{
		for (int i = h - 1; i < size; i++) {
			
			strcpy(B, arr[i]);
			int j = i;	
			 
			_1 = KeyExtractor(B);
			for( j = i; j >= h; j-=h) 
			{			
				_2 = KeyExtractor(arr[j-h]);

				if(strcmp(_2,_1)<0) break;
				strcpy(arr[j], arr[j-h]);
				
				delete[]_2; 
				_2 = NULL;
			}

			delete[] _1;
			if(_2) { delete[] _2; _2 = NULL; }
			
			strcpy(arr[j], B);
		}		
		h = h / 3;
	}
}

void BubbleSort(char **arr, int size, char* KeyExtractor(char*))
{
	char *_1, *_2;
	char temp[50];  
	bool swap;

	for(int i=size-1; i > 0; i--)
	{
		swap = false;
		for(int j=0; j<i; j++)
		{
			_1 = KeyExtractor(arr[j+1]);
			_2 = KeyExtractor(arr[j]);
			if(strcmp(_1,_2)<0)
			{
		      strcpy(temp, arr[j+1]);
			  strcpy(arr[j+1], arr[j]);
			  strcpy(arr[j], temp);
			  swap = true;
			}    
			delete[] _1;
			delete[] _2;
		}
		if(!swap) break;
	}
}



