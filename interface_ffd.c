#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <tchar.h>
#include "Debug/interface_ffd.h"

#define FFD_DATA_SIZE (sizeof(ffdDat))
#define MODELICA_DATA_SIZE (sizeof(modDat))

#pragma comment(lib, "user32.lib")

//char ffdDatNam[50] = "FFDDataMappingObject";
//TCHAR ffdDatNam[] = TEXT("FFDDataMappingObject");
//TCHAR modelicaDataName[] = TEXT("ModelicaDataMappingObject");

typedef struct {
  float t;
  int status;
  float number[3];
  char message[20];
}ffdDat;

typedef struct {
  float t;
  int status;
  float arr[3];
  char message[30];
}modDat;

HANDLE ffdDatMapFil;
HANDLE modDatMapFil;
ffdDat *ffdDatBuf;
modDat *modDatBuf;

///******************************************************************************
//| Start the memory management and FFD
//******************************************************************************/
//int instantiate()
//{
//  printf("interface_ffd.c: start to create shared memory.\n");
//
//  /*---------------------------------------------------------------------------
//  | Create named file mapping objects for specified files
//  ---------------------------------------------------------------------------*/
//  ffdDataMapFile = CreateFileMapping(
//                INVALID_HANDLE_VALUE,    // use paging file
//                NULL,                    // default security
//                PAGE_READWRITE,          // read/write access
//                0,                       // maximum object size (high-order DWORD)
//                BUF_FFD_SIZE,                // maximum object size (low-order DWORD)
//                ffdDatNam);                 // name of mapping object
//  modelicaDataMapFile = CreateFileMapping(
//                INVALID_HANDLE_VALUE,    // use paging file
//                NULL,                    // default security
//                PAGE_READWRITE,          // read/write access
//                0,                       // maximum object size (high-order DWORD)
//                BUF_MODELICA_SIZE,                // maximum object size (low-order DWORD)
//                modelicaDataName);                 // name of mapping object
//
//  // Send warning if can not create shared memory
//  if(ffdDataMapFile==NULL)
//  {
//    printf("Could not create file mapping object (%d).\n", 
//            GetLastError());
//    return 1;
//  }
//  if(modelicaDataMapFile==NULL)
//  {
//    printf("Could not create file mapping object (%d).\n", 
//            GetLastError());
//    return 1;
//  }
//
//  /*---------------------------------------------------------------------------
//  | Mps a view of a file mapping into the address space of a calling process
//  ---------------------------------------------------------------------------*/
//  ffdDatBuf = (ffdDat *) MapViewOfFile(ffdDataMapFile,   // handle to map object
//                      FILE_MAP_ALL_ACCESS, // read/write permission
//                      0,
//                      0,
//                      BUF_FFD_SIZE);
//  modDatBuf = (modDat *) MapViewOfFile(modelicaDataMapFile,   // handle to map object
//                      FILE_MAP_ALL_ACCESS, // read/write permission
//                      0,
//                      0,
//                      BUF_MODELICA_SIZE);
//
//  if(ffdDatBuf == NULL)
//  {
//    printf("Could not map view of file (%d).\n",
//            GetLastError());
//    CloseHandle(ffdDataMapFile);
//    return 1;
//  }
//
//  if(modDatBuf == NULL) 
//  {
//    printf("Could not map view of file (%d).\n",
//            GetLastError());
//    CloseHandle(modelicaDataMapFile);
//    return 1;
//  }
//
//  // Set the status to -1
//  ffdDatBuf->status = -1;
//  modDatBuf->status = -1;
//
//  printf("interface_ffd.c: initialized shared memory.\n");
//  return 0;
//} // End of instantiate()


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
  while(modDatBuf->status==1)
    Sleep(1000);

  // Copy a block of memory from modelicaData to modDatBuf
  modDatBuf->arr[1] = (float) x1[1];
  modDatBuf->arr[2] = (float) x1[2];
  modDatBuf->t = (float) t;
  modDatBuf->status = 1;
  strcpy(modDatBuf->message, x3);

  // If the data is not ready or not updated, check again
  while(ffdDatBuf->status!=1)
    Sleep(1000);

  // Copy a block of memory from ffdData
  y1[0] = ffdDatBuf->number[0];
  y1[1] = ffdDatBuf->number[1];
  y1[2] = ffdDatBuf->number[2];
  y2 = ffdDatBuf->status;
  strcpy(y3, ffdDatBuf->message);

  printf("\n FFD: time=%f, status=%d\n", ffdDatBuf->t,ffdDatBuf->status);
  printf("y1[0] = %f, y1[1] = %f, y1[2] = %f \n", y1[0], y1[1], y1[2]);
  printf("Modelica: time=%f, status=%d\n", modDatBuf->t,modDatBuf->status);
  printf("arr[0] = %f, arr[1] = %f, arr[2] = %f \n", modDatBuf->arr[0], modDatBuf->arr[1], modDatBuf->arr[2]);

  // Update the data status
  ffdDatBuf->status = 0;

} // End of exchangeData()

/******************************************************************************
| Terminate the memory management program
******************************************************************************/
void terminate_cosimulation( )
{
  if(!UnmapViewOfFile(ffdDatBuf))
    printf("Error in closing map view %d\n", GetLastError());
  else
    printf("Successfully closed data buffer for FFD.\n");
  
  if(!UnmapViewOfFile(modDatBuf))
    printf("Error in closing map view %d\n", GetLastError());
  else
    printf("Successfully closed data buffer for Modelica.\n");
  
  if(!CloseHandle(ffdDatMapFil))
    printf("Error in closing handle %d\n", GetLastError());
  else
    printf("Successfully closed handle for FFD.\n");

  if(!CloseHandle(modDatMapFil))
    printf("Error in closing handle %d\n", GetLastError());
  else
    printf("Successfully closed handle for Modelica.\n");

} // End of terminate()

/******************************************************************************
| Creat mapping to the shared memory for data exchange
******************************************************************************/
int create_mapping(char *ffdDatNam, char *modDatNam)
{
  int i=0, imax=1000;

  /*---------------------------------------------------------------------------
  | Open the FFD file mapping object
  ---------------------------------------------------------------------------*/
  ffdDatMapFil = OpenFileMapping(
                    FILE_MAP_ALL_ACCESS,    // read/write access
                    FALSE,           // do not inherit the name
                    ffdDatNam);    // name of mapping object for FFD data

  while(i<imax && ffdDatMapFil==NULL)
  {
    Sleep(1000);
    ffdDatMapFil = OpenFileMapping(
                    FILE_MAP_ALL_ACCESS,    // read/write access
                    FALSE,           // do not inherit the name
                    ffdDatNam);    // name of mapping object for FFD data
    i++;
  }

  // Send warning if can not open shared memory
  if(ffdDatMapFil==NULL)
  {
    printf("Could not open FFD data file mapping object (%d).\n", GetLastError());
    return GetLastError();
  }

  /*---------------------------------------------------------------------------
  | Maps a view of the FFD file mapping into the address space 
  ---------------------------------------------------------------------------*/
  ffdDatBuf = (ffdDat *) MapViewOfFile(ffdDatMapFil,   // handle to map object
                      FILE_MAP_ALL_ACCESS, // read/write permission
                      0,
                      0,
                      FFD_DATA_SIZE);
  i = 0;
  while(ffdDatBuf==NULL && i<imax)
  {
    Sleep(1000);
    ffdDatBuf = (ffdDat *) MapViewOfFile(ffdDatMapFil,   // handle to map object
                    FILE_MAP_ALL_ACCESS, // read/write permission
                    0,
                    0,
                    FFD_DATA_SIZE);
    i++;
  }

  if(ffdDatBuf==NULL)
  {
    printf("Could not map view of FFD data file (%d).\n", GetLastError());
    CloseHandle(ffdDatMapFil);
    return GetLastError();
  }

  /*---------------------------------------------------------------------------
  | Open the Modelcia file mapping object
  ---------------------------------------------------------------------------*/
  modDatMapFil = OpenFileMapping(
                      FILE_MAP_ALL_ACCESS,    // read/write access
                      FALSE,           // do not inherit the name
                      modDatNam);    // name of mapping object for FFD data
  i = 0;
  while(i<imax && modDatMapFil==NULL)
  {
    Sleep(1000);
    modDatMapFil = OpenFileMapping(
                        FILE_MAP_ALL_ACCESS,    // read/write access
                        FALSE,           // do not inherit the name
                        modDatNam);    // name of mapping object for FFD data
    i++;
  }

  // Send warning if can not open shared memory
  if(modDatMapFil==NULL)
  {
    printf("Could not open Other data file mapping object (%d).", GetLastError());
    return GetLastError();
  }

  /*---------------------------------------------------------------------------
  | Maps a view of the Modelica file mapping into the address space 
  ---------------------------------------------------------------------------*/
  modDatBuf = (modDat *) MapViewOfFile(modDatMapFil,   // handle to map object
                      FILE_MAP_ALL_ACCESS, // read/write permission
                      0,
                      0,
                      MODELICA_DATA_SIZE);
  i = 0;
  while(modDatBuf==NULL && i<imax)
  {
    Sleep(1000);
    modDatBuf = (modDat *) MapViewOfFile(modDatMapFil,   // handle to map object
                    FILE_MAP_ALL_ACCESS, // read/write permission
                    0,
                    0,
                    MODELICA_DATA_SIZE);
    i++;
  }

  if(modDatBuf==NULL)
  {
    printf("Could not map view of Modelcia data file (%d).\n", GetLastError());
    CloseHandle(modDatMapFil);
    return GetLastError();
  }

  return 0;

} // End of create_mapping()




