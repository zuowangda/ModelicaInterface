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
  ModelicaSharedData *modelicaDataBuf;
  ffdSharedData *ffdData;
  int i, imax = 10000;
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

  // Looking for the shared memory
  i = 0;
  while(modelicaDataBuf==NULL && i<imax)
  {
    Sleep(10000);
    printf("Wait for shared memory to be created\n");
    modelicaDataBuf = (ModelicaSharedData *) MapViewOfFile(hMapFile, // handle to map object
              FILE_MAP_ALL_ACCESS,  // read/write permission
              0,
              0,
              BUF_MODELICA_SIZE);
    i++;
  }

  if(modelicaDataBuf==NULL && i>=imax)
  {
    printf("interface.c: Cosimulation failed due to error in mapping the shared memory for modelica data after %d loops\n."
          , i);
    exit(1);
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
  if(hMapFile == NULL && i >= imax)
  {
    printf("interface.c: Cosimulation failed due to error in opening file mapping for FFD data after %d loops\n."
          , i);
    exit(1);
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
} // End of exchangeData()
