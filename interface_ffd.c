#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <tchar.h>
#include "Debug/interface_ffd.h"

#define BUF_SIZE 256
#define BUF_FFD_SIZE (sizeof(ffdSharedData))
#define BUF_MODELICA_SIZE (sizeof(ModelicaSharedData))

#pragma comment(lib, "user32.lib")

//TCHAR ffdDataName[] = TEXT("FFDDataMappingObject");
//TCHAR modelicaDataName[] = TEXT("ModelicaDataMappingObject");

typedef struct {
  float t;
  int status;
  float number[3];
  char message[20];
}ffdSharedData;

typedef struct {
  float t;
  int status;
  float arr[3];
  char message[30];
}ModelicaSharedData;

HANDLE ffdDataMapFile;
HANDLE modelicaDataMapFile;
ffdSharedData *ffdDataBuf;
ModelicaSharedData *modelicaDataBuf;

/******************************************************************************
| Start the memory management and FFD
******************************************************************************/
int instantiate(char *ffdDatNam, char *modDatNam)
{
  char msg[100];

  sprintf(ffdDatNam, "ffd%d", rand());
  //ffdDatNam = (char *) malloc(sizeof(msg));
  //strcpy(ffdDatNam, msg);

  sprintf(modDatNam, "mod%d", rand());
  //modDatNam = (char *) malloc(sizeof(msg));
  //strcpy(modDatNam, msg);

  /*---------------------------------------------------------------------------
  | Create named file mapping objects for specified files
  ---------------------------------------------------------------------------*/
  ffdDataMapFile = CreateFileMapping(
                INVALID_HANDLE_VALUE,    // use paging file
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                0,                       // maximum object size (high-order DWORD)
                BUF_FFD_SIZE,                // maximum object size (low-order DWORD)
                ffdDatNam);                 // name of mapping object
  modelicaDataMapFile = CreateFileMapping(
                INVALID_HANDLE_VALUE,    // use paging file
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                0,                       // maximum object size (high-order DWORD)
                BUF_MODELICA_SIZE,                // maximum object size (low-order DWORD)
                modDatNam);                 // name of mapping object

  // Send warning if can not create shared memory
  if(ffdDataMapFile==NULL)
  {
    printf("Could not create file mapping object (%d).\n", 
            GetLastError());
    return 1;
  }
  if(modelicaDataMapFile==NULL)
  {
    printf("Could not create file mapping object (%d).\n", 
            GetLastError());
    return 1;
  }

  /*---------------------------------------------------------------------------
  | Mps a view of a file mapping into the address space of a calling process
  ---------------------------------------------------------------------------*/
  ffdDataBuf = (ffdSharedData *) MapViewOfFile(ffdDataMapFile,   // handle to map object
                      FILE_MAP_ALL_ACCESS, // read/write permission
                      0,
                      0,
                      BUF_FFD_SIZE);
  modelicaDataBuf = (ModelicaSharedData *) MapViewOfFile(modelicaDataMapFile,   // handle to map object
                      FILE_MAP_ALL_ACCESS, // read/write permission
                      0,
                      0,
                      BUF_MODELICA_SIZE);

  if(ffdDataBuf == NULL)
  {
    printf("Could not map view of file (%d).\n",
            GetLastError());
    CloseHandle(ffdDataMapFile);
    return 1;
  }

  if(modelicaDataBuf == NULL) 
  {
    printf("Could not map view of file (%d).\n",
            GetLastError());
    CloseHandle(modelicaDataMapFile);
    return 1;
  }

  // Set the status to -1
  ffdDataBuf->status = -1;
  modelicaDataBuf->status = -1;

  printf("Created shared memory \"%s\" and \"%s\".\n", ffdDatNam, modDatNam);
  getchar();
  return 0;
} // End of instantiate()


/******************************************************************************
| Exchange data between Modelica and Shared Memory
******************************************************************************/
void exchangeData(double *x1, float t, char *x3, double *y1)
{
  int imax = 10000;
  int y2;
  char y3[20];

  /*--------------------------------------------------------------------------
  | Write data to FFD
  | Command: 
  | -1: feak data
  |  0: data has been read by the other program
  |  1: data waiting for the other program to read
  --------------------------------------------------------------------------*/
  // If previous data hasn't been read, wait
  while(modelicaDataBuf->status==1)
    Sleep(1000);

  // Copy a block of memory from modelicaData to modelicaDataBuf
  modelicaDataBuf->arr[1] = (float) x1[1];
  modelicaDataBuf->arr[2] = (float) x1[2];
  modelicaDataBuf->t = (float) t;
  modelicaDataBuf->status = 1;
  strcpy(modelicaDataBuf->message, x3);

  // If the data is not ready or not updated, check again
  while(ffdDataBuf->status!=1)
    Sleep(1000);

  // Copy a block of memory from ffdData
  y1[0] = ffdDataBuf->number[0];
  y1[1] = ffdDataBuf->number[1];
  y1[2] = ffdDataBuf->number[2];
  y2 = ffdDataBuf->status;
  strcpy(y3, ffdDataBuf->message);

  printf("\n FFD: time=%f, status=%d\n", ffdDataBuf->t,ffdDataBuf->status);
  printf("y1[0] = %f, y1[1] = %f, y1[2] = %f \n", y1[0], y1[1], y1[2]);
  printf("Modelica: time=%f, status=%d\n", modelicaDataBuf->t,modelicaDataBuf->status);
  printf("arr[0] = %f, arr[1] = %f, arr[2] = %f \n", modelicaDataBuf->arr[0], modelicaDataBuf->arr[1], modelicaDataBuf->arr[2]);

  // Update the data status
  ffdDataBuf->status = 0;

} // End of exchangeData()

/******************************************************************************
| Terminate the memory management program
******************************************************************************/
void terminate_cosimulation( )
{
  if(!UnmapViewOfFile(ffdDataBuf))
    printf("Error in closing map view %d\n", GetLastError());
  else
    printf("Successfully closed data buffer for FFD.\n");
  
  if(!UnmapViewOfFile(modelicaDataBuf))
    printf("Error in closing map view %d\n", GetLastError());
  else
    printf("Successfully closed data buffer for Modelica.\n");
  
  if(!CloseHandle(ffdDataMapFile))
    printf("Error in closing handle %d\n", GetLastError());
  else
    printf("Successfully closed handle for FFD.\n");

  if(!CloseHandle(modelicaDataMapFile))
    printf("Error in closing handle %d\n", GetLastError());
  else
    printf("Successfully closed handle for Modelica.\n");

  getchar();
} // End of terminate()





