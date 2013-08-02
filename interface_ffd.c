#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <tchar.h>
#include "Debug/interface_ffd.h"
#include "../FFD-DLL/Fast-Fluid-Dynamics/modelica_ffd_common.h"

#define BUF_SIZE 256

#pragma comment(lib, "user32.lib")

//typedef struct {
//  float t;
//  int status;
//  float number[3];
//  char message[20];
//}ffdSharedData;
//
//typedef struct {
//  float t;
//  int status;
//  float arr[3];
//  float *testdata;
//  char message[30];
//}ModelicaSharedData;

HANDLE ffdDataMapFile;
HANDLE modelicaDataMapFile;
HANDLE boundaryDataMapFile;
ffdSharedData *ffdDataBuf;
ModelicaSharedData *modelicaDataBuf;
BoundarySharedData *boundaryDataBuf;

/******************************************************************************
| Start the memory management and FFD
******************************************************************************/
int instantiate(int nSur, int nConExtWin, int nPorts, int sha,
                char **name, float *are, float *til, int *bouCon)
{
  int i, nBou;
  printf("interface_ffd.c: start to create shared memory.\n");

  /*---------------------------------------------------------------------------
  | Create named file mapping objects for specified files
  ---------------------------------------------------------------------------*/
  ffdDataMapFile = CreateFileMapping(
                INVALID_HANDLE_VALUE,    // use paging file
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                0,                       // maximum object size (high-order DWORD)
                BUF_FFD_SIZE,                // maximum object size (low-order DWORD)
                ffdDataName);                 // name of mapping object
  modelicaDataMapFile = CreateFileMapping(
                INVALID_HANDLE_VALUE,    // use paging file
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                0,                       // maximum object size (high-order DWORD)
                BUF_MODELICA_SIZE,                // maximum object size (low-order DWORD)
                modelicaDataName);                 // name of mapping object
  boundaryDataMapFile = CreateFileMapping(
                INVALID_HANDLE_VALUE,    // use paging file
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                0,                       // maximum object size (high-order DWORD)
                BUF_MODELICA_SIZE,                // maximum object size (low-order DWORD)
                boundaryDataName);                 // name of mapping object

  // Send warning if can not create shared memory
  if(ffdDataMapFile==NULL)
  {
    printf("Could not create file mapping object (%d).\n", 
            GetLastError());
    return GetLastError();
  }
  else if(modelicaDataMapFile==NULL)
  {
    printf("Could not create file mapping object (%d).\n", 
            GetLastError());
    return GetLastError();
  }
  else if(boundaryDataMapFile==NULL)
  {
    printf("Could not create file mapping object (%d).\n", 
            GetLastError());
    return GetLastError();
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
  boundaryDataBuf = (BoundarySharedData *) MapViewOfFile(modelicaDataMapFile,   // handle to map object
                      FILE_MAP_ALL_ACCESS, // read/write permission
                      0,
                      0,
                      BUF_BOUNDARY_SIZE);

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
  if(boundaryDataBuf == NULL) 
  {
    printf("Could not map view of file (%d).\n",
            GetLastError());
    CloseHandle(boundaryDataMapFile);
    return 1;
  }

  /*---------------------------------------------------------------------------
  | allocate the memory and assign the data
  --------------------------------------------------------------------------*/
  boundaryDataBuf->nSur = nSur;
  boundaryDataBuf->nConExtWin = nConExtWin;
  boundaryDataBuf->nPorts = nPorts;
  boundaryDataBuf->sha = sha;

  nBou = nSur + nPorts;

  name = (char**) malloc(nBou*sizeof(char *));
  are = (float *) malloc(nSur*sizeof(float));
  til = (float *) malloc(nSur*sizeof(float));
  bouCon = (int *) malloc(nSur*sizeof(int));

  for(i=0; i<nBou; i++) {
    boundaryDataBuf->name[i] = (char *)malloc(sizeof(name[i]));
    strcpy(boundaryDataBuf->name[i], name[i]);
    boundaryDataBuf->are[i] = are[i];
    boundaryDataBuf->til[i] = til[i];
    boundaryDataBuf->bouCon[i] = bouCon[i];
  }

  modelicaDataBuf->flag = -1;
  ffdDataBuf->flag = -1;

  modelicaDataBuf->temHea = (float *) malloc(nSur * sizeof(float));
  // Having a shade for window
  if(sha==1) {
    modelicaDataBuf->shaConSig = (float *) malloc(nConExtWin * sizeof(float));
    modelicaDataBuf->shaAbsRad = (float *) malloc(nConExtWin * sizeof(float));
  }
  modelicaDataBuf->mFloRatPor = (float *) malloc(nPorts * sizeof(float));
  modelicaDataBuf->TPor = (float *) malloc(nPorts * sizeof(float));
  modelicaDataBuf->XiPor = (float *) malloc(nPorts * sizeof(float));
  modelicaDataBuf->CPor = (float *) malloc(nPorts * sizeof(float));


  ffdDataBuf->temHea = (float *) malloc(nSur * sizeof(float));
  if(sha==1) ffdDataBuf->TSha = (float *) malloc(nConExtWin * sizeof(float));
  ffdDataBuf->TPor = (float *) malloc(nPorts * sizeof(float));
  ffdDataBuf->XiPor = (float *) malloc(nPorts * sizeof(float));
  ffdDataBuf->CPor = (float *) malloc(nPorts * sizeof(float));
  
  printf("interface_ffd.c: initialized shared memory.\n");
  return 0;
} // End of instantiate()


/******************************************************************************
| Exchange data between Modelica and Shared Memory
******************************************************************************/
void exchangeData(double t0, double dt, double *temHea0, double heaConvec, 
                  double *shaConSig, double *shaAbsRad, double p, 
                  double *mFloRatPor0, double *TPor0, double *XiPor0, double *CPor0,
                  double t1, double *temHea1, double TRoo, double *TSha, 
                  double *TPor1, double *XiPor1, double *CPor1)
{
  int i, imax = 10000;

  /*--------------------------------------------------------------------------
  | Write data to FFD
  | Command: 
  | -1: feak data
  |  0: data has been read by the other program
  |  1: data waiting for the other program to read
  --------------------------------------------------------------------------*/
  // If previous data hasn't been read, wait
  while(modelicaDataBuf->flag==1)
    Sleep(1000);

  modelicaDataBuf->t = (float) t0;
  modelicaDataBuf->dt = (float) dt;
  modelicaDataBuf->p = (float) p;
  modelicaDataBuf->heaConvec = (float) heaConvec;

  // Copy the modelica data to shared memory
  for(i=0; i<boundaryDataBuf->nSur; i++) 
    modelicaDataBuf->temHea[i] = (float) temHea0[i];

  if(boundaryDataBuf->sha==1)
    for(i=0; i<boundaryDataBuf->nConExtWin; i++) {
      modelicaDataBuf->shaConSig[i] = (float) shaConSig[i];
      modelicaDataBuf->shaAbsRad[i] = (float) shaAbsRad[i];
    }

  for(i=0; i<boundaryDataBuf->nPorts; i++) {
    modelicaDataBuf->mFloRatPor[i] = (float) mFloRatPor0[i];
    modelicaDataBuf->TPor[i] = (float) TPor0[i];
    modelicaDataBuf->XiPor[i] = (float) XiPor0[i];
    modelicaDataBuf->CPor[i] = (float) CPor0[i];
  }

  // Set the flag to new data
  modelicaDataBuf->flag = 1;

  // If the data is not ready or not updated, check again
  while(ffdDataBuf->flag!=1)
    Sleep(1000);

  // Copy data memory from shared memory
  for(i=0; i<boundaryDataBuf->nSur; i++) 
    temHea1[i] = ffdDataBuf->temHea[i];

  TRoo = ffdDataBuf->TRoo;
  
  if(boundaryDataBuf->sha==1)
    for(i=0; i<boundaryDataBuf->nConExtWin; i++) {
      TSha[i] = ffdDataBuf->TSha[i];
    }


  for(i=0; i<boundaryDataBuf->nPorts; i++) {
    TPor1[i] = ffdDataBuf->TPor[i];
    XiPor1[i] = ffdDataBuf->XiPor[i];
    CPor1[i] = ffdDataBuf->CPor[i];
  }

  printf("\n FFD: time=%f, status=%d\n", ffdDataBuf->t,ffdDataBuf->flag);
//  printf("y1[0] = %f, y1[1] = %f, y1[2] = %f \n", y1[0], y1[1], y1[2]);
  printf("Modelica: time=%f, status=%d\n", modelicaDataBuf->t,modelicaDataBuf->flag);
//  printf("arr[0] = %f, arr[1] = %f, arr[2] = %f \n", modelicaDataBuf->arr[0], modelicaDataBuf->arr[1], modelicaDataBuf->arr[2]);

  // Update the data status
  ffdDataBuf->flag = 0;

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





