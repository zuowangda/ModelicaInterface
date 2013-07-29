#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <tchar.h>

#include "Debug/interface_ffd.h"

#pragma comment(lib, "user32.lib")

#define BUF_FFD_SIZE (sizeof(ffdSharedData))
#define BUF_MODELICA_SIZE (sizeof(ModelicaSharedData))

TCHAR ffdDataName[] = TEXT("FFDDataMappingObject");
TCHAR modelicaDataName[] = TEXT("ModelicaDataMappingObject");

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


/******************************************************************************
| Start the memory management and FFD
******************************************************************************/
int instantiate()
{
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
void exchangeData(double *x1, float t, char *x3, double *y1)
{
  ModelicaSharedData modelicaData;
  HANDLE hMapFile;
  ModelicaSharedData *modelicaDataBuf;
  ffdSharedData *ffdData;
  int i, imax = 10000;
  int y2;
  char y3[20];

  modelicaData.arr[0] = (float) x1[0];
  modelicaData.arr[1] = (float) x1[1];
  modelicaData.arr[2] = (float) x1[2];
  modelicaData.t = (float) t;
  modelicaData.status = 1;
  strcpy(modelicaData.message, x3);

  /*--------------------------------------------------------------------------
  | Write data to FFD
  | Command: 
  | -1: feak data
  |  0: data has been read by the other program
  |  1: data waiting for the other program to read
  --------------------------------------------------------------------------*/
  hMapFile = OpenFileMapping(
                  FILE_MAP_ALL_ACCESS,   // read/write access
                  FALSE,                 // do not inherit the name
                  modelicaDataName);               // name of mapping object

  i = 0;
  // Check again if map is not open and iteration does not reach the limit
  while(hMapFile==NULL && i<imax)
  {
    Sleep(100);
    hMapFile = OpenFileMapping(
                  FILE_MAP_ALL_ACCESS,   // read/write access
                  FALSE,                 // do not inherit the name
                  modelicaDataName);               // name of mapping object
  }
  if(hMapFile==NULL && i>=imax)
  {
    printf("interface.c: Cosimulation failed due to error in mapping the shared memory for modelica data after %d times\n."
          , i);
    exit(1);
  }

  // Map the file
  modelicaDataBuf = (ModelicaSharedData *) MapViewOfFile(hMapFile, // handle to map object
              FILE_MAP_ALL_ACCESS,  // read/write permission
              0,
              0,
              BUF_MODELICA_SIZE);

  // Check again if map view is not ready and iteration doesnot reach the limit
  i = 0;
  while(modelicaDataBuf==NULL && i<imax)
  {
    Sleep(100);
    modelicaDataBuf = (ModelicaSharedData *) MapViewOfFile(hMapFile, // handle to map object
              FILE_MAP_ALL_ACCESS,  // read/write permission
              0,
              0,
              BUF_MODELICA_SIZE);
    i++;
  }

  // Quit with error if wait after imax iterations
  if(modelicaDataBuf==NULL && i>=imax)
  {
    printf("interface.c: Cosimulation failed due to error in mapping the shared memory for modelica data after %d loops\n."
          , i);
    exit(1);
  }

  // If previous data hasn't been read, wait
  while(modelicaDataBuf->status==1)
    Sleep(10000);

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

  // Continue check unitl get the map
  i = 0;
  while(hMapFile==NULL && i<imax)
  {
    hMapFile = OpenFileMapping(
                  FILE_MAP_ALL_ACCESS,   // read/write access
                  FALSE,                 // do not inherit the name
                  ffdDataName);               // name of mapping object
    i++;
  }

  // Quit with warning if cannot get map after imax times
  if(hMapFile==NULL && i>=imax)
  {
    printf("interface.c: Cosimulation failed due to error in opening file mapping for FFD data after %d loops\n."
          , i);
    exit(1);
  }

  // Get the wanted shared memory data
  ffdData = (ffdSharedData *) MapViewOfFile(hMapFile, // handle to map object
              FILE_MAP_ALL_ACCESS,  // read/write permission
              0,
              0,
              BUF_FFD_SIZE);

  // If the data is not ready or not updated, check again
  while(ffdData==NULL || ffdData->status!=1)
  {
    Sleep(1000);
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
  y2 = ffdData->status;
  strcpy(y3, ffdData->message);

  printf("\n FFD: time=%f, status=%d\n", ffdData->t,ffdData->status);
  printf("y1[0] = %f, y1[1] = %f, y1[2] = %f \n", y1[0], y1[1], y1[2]);
  printf("Modelica: time=%f, status=%d\n", modelicaData.t,modelicaData.status);
  printf("arr[0] = %f, arr[1] = %f, arr[2] = %f \n", modelicaData.arr[0], modelicaData.arr[1], modelicaData.arr[2]);

  //printf("y3 = %s\n", y3);
  //printf("ffdData->message=%s\n", ffdData->message);

  // Fixme: Try to update the memory directly
  // Update the data status
  ffdData->status = 0;
  //printf("ffdData->status = %d\n", ffdData->status);

  // Close the map and handle
  UnmapViewOfFile(ffdData);
  CloseHandle(hMapFile);
} // End of exchangeData()

/******************************************************************************
| Terminate the memory management program
******************************************************************************/
void terminate_cosimulation( )
{
  HANDLE hMapFile;
  ModelicaSharedData *modelicaData;

  hMapFile = OpenFileMapping(
                  FILE_MAP_ALL_ACCESS,   // read/write access
                  FALSE,                 // do not inherit the name
                  modelicaDataName);               // name of mapping object

  // Get the wanted shared memory data
  modelicaData = (ModelicaSharedData *) MapViewOfFile(hMapFile, // handle to map object
              FILE_MAP_ALL_ACCESS,  // read/write permission
              0,
              0,
              BUF_MODELICA_SIZE);

  // Set signal to close memory management
  modelicaData->status = -2;
  UnmapViewOfFile(modelicaData);
  CloseHandle(hMapFile);

} // End of terminate()