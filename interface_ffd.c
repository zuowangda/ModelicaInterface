#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
<<<<<<< HEAD
#include <windows.h>
#include <conio.h>
#include <tchar.h>

#include "interface_ffd.h"
=======

#include "resource.h"
//#include "interface_ffd.h"

FILE *file_log;

typedef struct {
  double number0;
  double number1;
  int command;
} ReceivedData;
>>>>>>> e8b242a3c3c5c87c63bf45c3b4c697834931659a

typedef struct {
  int feedback;
} SentCommand;

<<<<<<< HEAD
#define BUF_FFD_SIZE (sizeof(ffdSharedData))
#define BUF_MODELICA_SIZE (sizeof(ModelicaSharedData))

TCHAR ffdDataName[] = TEXT("FFDDataMappingObject");
TCHAR modelicaDataName[] = TEXT("ModelicaDataMappingObject");

typedef struct {
  float number[3];
  int command;
  char message[20];
}ffdSharedData;

typedef struct {
  float number;
  int command;
  float arr[3];
  char message[30];
}ModelicaSharedData;
=======
typedef struct {
  double number0;
  double number1;
  int command;
}SentData;

typedef struct {
  int feedback;
}ReceivedCommand;

//Global Variables:
ReceivedData data_received;
ReceivedCommand command_received;
SentData data_sent;


//const char fFDRecvDataCh1[] = "FFDRecvDataCh1";         //Ch1: Dymola to FFD   Need define it in caption
TCHAR command_window_ffd[] = TEXT("FFDRecvDataCh1");    
//const char fFDRecvFeedbackCh2[] = "FFDRecvFeedbackCh2"; //Ch2: FFD to Dymola   Need define it in caption
TCHAR data_window_ffd[] = TEXT("FFDRecvFeedbackCh2");

/******************************************************************************
| Write the log file
******************************************************************************/
void cosim_log(char *message)
{
  if((file_log=fopen("log_cosim.ffd","a+")) == NULL)
  {
    fprintf(stderr,"Error:can not open error file!\n");
    exit(1);
  }

  fprintf(file_log, "%s\n", message);

  fclose(file_log);
}; // End of cosim_log()

/******************************************************************************
| Receive the command from the other program
******************************************************************************/
BOOL CALLBACK receive_command(HWND hwndDlg, UINT message, WPARAM wParam, 
                              LPARAM lParam) 
{
  PCOPYDATASTRUCT pcds; 

  // Hide message window
  ShowWindow(hwndDlg, SW_HIDE);

  switch(message)
  {
    case WM_CLOSE:
      EndDialog(hwndDlg, 0);
      break;
    case WM_COPYDATA:
      pcds = (PCOPYDATASTRUCT)lParam;
      // If the size matches, copy the data
      if (pcds->cbData == sizeof(command_received))
      {
        memcpy_s(&command_received, sizeof(command_received), pcds->lpData, pcds->cbData);
        EndDialog(hwndDlg, 0);
      }
  }
  return FALSE;

} // End of receive_command()

/******************************************************************************
| Send commands to the other programs
******************************************************************************/
void send_command(SentCommand command_sent){
  HWND hSendWindow;
  HWND hRecvWindow;
  COPYDATASTRUCT data_package;

  // Get handel of two windows
  hSendWindow = GetConsoleWindow ();        
  if (hSendWindow == NULL)
    cosim_log("Error in cosimulation.c: self handel not found.\n");

  // Check if the other program is ready (created relative console window) 
  // for receiving the message
  hRecvWindow = FindWindow(NULL, command_window_ffd);

  // Continue to check the status if the other program is ready
  while(hRecvWindow == NULL){
    // Wait
    Sleep(500);
    // Check it again
    hRecvWindow = FindWindow(NULL, command_window_ffd);
  }

  // Set the command to a package
  data_package.cbData = sizeof(command_sent);
  data_package.lpData = &command_sent;

  // Send the command
  SendMessage(hRecvWindow, WM_COPYDATA, (WPARAM)hSendWindow, (LPARAM)&data_package);
}
>>>>>>> e8b242a3c3c5c87c63bf45c3b4c697834931659a

/******************************************************************************
| Receive data from Windows message
******************************************************************************/
BOOL CALLBACK receive_data_dialog(HWND hwndDlg, UINT message, 
                                  WPARAM wParam, LPARAM lParam)
{
  PCOPYDATASTRUCT pcds;

  // Hide the dialog window
  ShowWindow(hwndDlg, SW_HIDE);

<<<<<<< HEAD
/******************************************************************************
| Start the memory management and FFD
******************************************************************************/
int instantiate(){
  int status;

  // Launch the memory management tool
  status = system("start ..\\..\\MemoryManagement\\Debug\\MemoryManagement.exe"); 
  printf("Launch MemoryManagement with status: %d\n", status);
  // Use start to creat a new window for FFD; like fork the process
  status = system("start ..\\..\\Fast-Fluid-Dynamics\\Debug\\FFD_SCI.exe"); 
  printf("Launch FFD with status: %d\n", status);

  return status;
} // End of instantiate()


/******************************************************************************
| Exchange data between Modelica and Shared Memory
******************************************************************************/
void exchangeData(double *x1, int x2, char *x3, double *y1)
{
  ModelicaSharedData modelicaData;
  HANDLE hMapFile;
  LPCTSTR pBuf;
  ModelicaSharedData *modelicaDataBuf;
  ffdSharedData *ffdData;
  int y2;
  char y3[20];

  modelicaData.arr[0] = (float) x1[0];
  modelicaData.arr[1] = (float) x1[1];
  modelicaData.arr[2] = (float) x1[2];
  modelicaData.number = (float) x1[3];
  modelicaData.command = x2;
  strcpy(modelicaData.message, x3);

  /*--------------------------------------------------------------------------
  | Write data to FFD
  --------------------------------------------------------------------------*/
  hMapFile = OpenFileMapping(
                  FILE_MAP_ALL_ACCESS,   // read/write access
                  FALSE,                 // do not inherit the name
                  modelicaDataName);               // name of mapping object

  while (hMapFile == NULL)
  {
    hMapFile = OpenFileMapping(
                  FILE_MAP_ALL_ACCESS,   // read/write access
                  FALSE,                 // do not inherit the name
                  modelicaDataName);               // name of mapping object
  }

  modelicaDataBuf = (ModelicaSharedData *) MapViewOfFile(hMapFile, // handle to map object
              FILE_MAP_ALL_ACCESS,  // read/write permission
              0,
              0,
              BUF_MODELICA_SIZE);

  while(modelicaDataBuf == NULL)
  {
    Sleep(100);
    printf("Wait for shared memory to be created\n");
    modelicaDataBuf = (ModelicaSharedData *) MapViewOfFile(hMapFile, // handle to map object
              FILE_MAP_ALL_ACCESS,  // read/write permission
              0,
              0,
              BUF_MODELICA_SIZE);
  }

  // Copy a block of memory from modelicaData to modelicaDataBuf
  CopyMemory((PVOID)modelicaDataBuf, &modelicaData, sizeof(ModelicaSharedData));
  UnmapViewOfFile(modelicaDataBuf);
  CloseHandle(hMapFile);

  /*--------------------------------------------------------------------------
  | Read FFD data from shared memory
  --------------------------------------------------------------------------*/
  hMapFile = OpenFileMapping(
                  FILE_MAP_ALL_ACCESS,   // read/write access
                  FALSE,                 // do not inherit the name
                  ffdDataName);               // name of mapping object

  while (hMapFile == NULL)
  {
    hMapFile = OpenFileMapping(
                  FILE_MAP_ALL_ACCESS,   // read/write access
                  FALSE,                 // do not inherit the name
                  ffdDataName);               // name of mapping object
  }

  ffdData = (ffdSharedData *) MapViewOfFile(hMapFile, // handle to map object
              FILE_MAP_ALL_ACCESS,  // read/write permission
              0,
              0,
              BUF_FFD_SIZE);

  while(ffdData == NULL || ffdData->command == -1)
  {
    Sleep(100);
    ffdData = (ffdSharedData *) MapViewOfFile(hMapFile, // handle to map object
              FILE_MAP_ALL_ACCESS,  // read/write permission
              0,
              0,
              BUF_FFD_SIZE);
  }

  // Copy a block of memory from ffdData
  y1[0] = ffdData->number[0];
  y1[1] = ffdData->number[1];
  y1[2] = ffdData->number[2];
  y2 = ffdData->command;
  strcpy(y3, ffdData->message);

  printf("Data got from FFD\n");
  printf("y1[0] = %f, y1[1] = %f, y1[2] = %f \n", y1[0], y1[1], y1[2]);
  printf("y2 = %d\n", y2);
  printf("y3 = %s\n", y3);
  printf("ffdData->message=%s\n", ffdData->message);
  UnmapViewOfFile(ffdData);
  CloseHandle(hMapFile);
=======
  switch (message)
  {
    case WM_CLOSE:
      EndDialog(hwndDlg, 0);
      break;
    case WM_COPYDATA:
      pcds = (PCOPYDATASTRUCT)lParam;
      // If the size matches, copy the data
      if (pcds->cbData == sizeof(data_received)){
        memcpy_s(&data_received, sizeof(data_received), pcds->lpData, pcds->cbData);
        EndDialog(hwndDlg, 0);
      }
  }

  return FALSE;
} //End of receive_data_dialog()

/******************************************************************************
| Receive data from the other program
******************************************************************************/
void receive_data()
{
  SentCommand command_sent;
  char msg[500];

  // Receive the data through the message
  DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DATA), NULL, receive_data_dialog);
  
  sprintf(msg, "interface_ffd.c: Received data: %d, %f, %f ", 
          data_received.command, data_received.number0, data_received.number1);
  cosim_log(msg);

  // Send feedback for successfully receive
  command_sent.feedback = 1;
  send_command(command_sent);

} // End of receive_data()




/******************************************************************************
| Send data to the other program
******************************************************************************/
int send_data(){
  HWND hSendWindow;
  HWND hRecvWindow ;
  COPYDATASTRUCT data_package;

  //get handel of two windows
  hSendWindow = GetConsoleWindow ();        
  if (hSendWindow == NULL) {
    cosim_log("Error in interface_ffd.c: Self handel not found");
    return 1;
  }

  // Get the window handle of the other program
  hRecvWindow = FindWindow(NULL, data_window_ffd);
  while(hRecvWindow == NULL){
    // Wait
    Sleep(500);
    // Check again
    hRecvWindow = FindWindow(NULL, data_window_ffd);
  }

  // Set data to a package
  data_package.cbData = sizeof(data_sent);
  data_package.lpData = &data_sent;

  // Send the package
  SendMessage(hRecvWindow, WM_COPYDATA, (WPARAM)hSendWindow, (LPARAM)&data_package);

  // Check if the data package have been received
  DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_FEEDBACK), NULL, receive_command);

  if(command_received.feedback==1)
    return 0;
  else
    return 1;
} // End of send_data()

int initiateFFD(int *command, double *data_to_ffd, double *data_from_ffd)
{
  // Send data to FFD
  data_sent.command = data_to_ffd[0];
  data_sent.number0 = data_to_ffd[1];
  data_sent.number1 = data_to_ffd[2];
  command[0] = send_data();


  // Receive data from FFD
  command[1] = receive_data();
  data_from_ffd[0] = data_received.command;
  data_from_ffd[1] = data_received.number0;
  data_from_ffd[2] = data_received.number2;
}

/******************************************************************************
| Start the FFD simulation
******************************************************************************/
int initiateFFD(){
  int status;

  // Use start to creat a new window for FFD; like fork the process
  status = system("start ..\\..\\Fast-Fluid-Dynamics\\Debug\\FFD_SCI.exe"); 
  cosim_log("inerface_ffd.c: Successully launched FFD\n");

  return status;
>>>>>>> e8b242a3c3c5c87c63bf45c3b4c697834931659a
}
//int _tmain(int argc, _TCHAR* argv[])
//{
//
//  double step = 1.0;
//  int command = 1;
//  int check;
//
//  while(command < 100 )
//  {
//
//    SentData dymolaSend1;                             //prepare data to send
//    dymolaSend1.number0 = step;
//    dymolaSend1.number1 = 10 * step;
//    dymolaSend1.command = command;
//
//    check = send_data(dymolaSend1);          //send
//
//    if (check==0){
//      printf("%s\n", "No feedback from FFD");
//      system("pause");
//    }
//    printf("%s\n", "get feedback");
//    //print...........
//    printf("%s\n", "Ch1");
//    printf("%f\n",dymolaSend1.number0);
//    printf("%f\n",dymolaSend1.number1);
//    printf("%d\n",dymolaSend1.command);
//
//    printf("%s\n", "Ch2");
//    receive_data();                             //receive
//    printf("%f\n",data_received.number0);
//    printf("%f\n",data_received.number1);
//    printf("%d\n",data_received.command);
//    
//    step = step + 1.0;
//    command = command + 1.0;
//  }
//}


