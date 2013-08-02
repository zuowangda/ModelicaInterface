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
int instantiate(char **name, double *A, double *til, int *bouCon, int haveSensor,
                char **sensorName, int haveShade, int nSur, int nSen,
                int nConExtWin, int nPorts )
{
  int i, nBou;
  printf("interface_ffd.c: Start to create shared memory.\n");
  getchar();
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
                BUF_BOUNDARY_SIZE,                // maximum object size (low-order DWORD)
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

  printf("interface-ffd.c: Created file objects.\n");
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
  boundaryDataBuf = (BoundarySharedData *) MapViewOfFile(boundaryDataMapFile,   // handle to map object
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

  printf("interface-ffd.c: Created mapping.\n");

  /*---------------------------------------------------------------------------
  | allocate the memory and assign the data
  --------------------------------------------------------------------------*/
  boundaryDataBuf->nSur = nSur;
  boundaryDataBuf->nSen = nSen;
  boundaryDataBuf->nConExtWin = nConExtWin;
  boundaryDataBuf->nPorts = nPorts;
  boundaryDataBuf->sha = haveShade;

  printf("interface_ffd.c: Number of surfaces: %d\n", boundaryDataBuf->nSur);
  printf("interface_ffd.c: Number of sensors: %d\n", boundaryDataBuf->nSen);
  printf("interface_ffd.c: Number of exterior construction with windows: %d\n", boundaryDataBuf->nConExtWin);
  printf("interface_ffd.c: have shade: %d\n", boundaryDataBuf->sha);


  nBou = nSur + nPorts;

  boundaryDataBuf->name = (char**) malloc(nBou*sizeof(char *));
  boundaryDataBuf->are = (float *) malloc(nSur*sizeof(float));
  boundaryDataBuf->til = (float *) malloc(nSur*sizeof(float));
  boundaryDataBuf->bouCon = (int *) malloc(nSur*sizeof(int));
  
  for(i=0; i<nBou; i++) {
    boundaryDataBuf->name[i] = (char *)malloc(sizeof(char) *strlen(name[i]));
    strcpy(boundaryDataBuf->name[i], name[i]);
    printf("Boundary name:%s\n", boundaryDataBuf->name[i]);

    boundaryDataBuf->are[i] = A[i];
    printf("\tA->Area:%f->%f [m2]\n", A[i], boundaryDataBuf->are[i]);

    boundaryDataBuf->til[i] = til[i];
    printf("\tTilt->Tilt:%f->%f [deg]\n", til[i], boundaryDataBuf->til[i]);

    boundaryDataBuf->bouCon[i] = bouCon[i];
    printf("\tbouCon->bouCon:%d->%d \n\n", bouCon[i], boundaryDataBuf->bouCon[i]);
  }

  if(haveSensor) {
    boundaryDataBuf->sensorName = (char **) malloc(nSen*sizeof(char *));
    for(i=0; i<nSen; i++) {
      boundaryDataBuf->sensorName[i] = (char *)malloc(sizeof(char) * strlen(sensorName[i]));
      strcpy(boundaryDataBuf->sensorName[i], sensorName[i]);
      printf("Sensor Name:%s\n", boundaryDataBuf->sensorName[i]);
    }
  }

  modelicaDataBuf->flag = -1;
  ffdDataBuf->flag = -1;

  modelicaDataBuf->temHea = (float *) malloc(nSur * sizeof(float));
  // Having a shade for window
  if(haveShade==1) {
    modelicaDataBuf->shaConSig = (float *) malloc(nConExtWin * sizeof(float));
    modelicaDataBuf->shaAbsRad = (float *) malloc(nConExtWin * sizeof(float));
  }
  modelicaDataBuf->mFloRatPor = (float *) malloc(nPorts * sizeof(float));
  modelicaDataBuf->TPor = (float *) malloc(nPorts * sizeof(float));
  modelicaDataBuf->XiPor = (float *) malloc(nPorts * sizeof(float));
  modelicaDataBuf->CPor = (float *) malloc(nPorts * sizeof(float));


  ffdDataBuf->temHea = (float *) malloc(nSur * sizeof(float));
  if(haveShade==1) ffdDataBuf->TSha = (float *) malloc(nConExtWin * sizeof(float));
  ffdDataBuf->TPor = (float *) malloc(nPorts * sizeof(float));
  ffdDataBuf->XiPor = (float *) malloc(nPorts * sizeof(float));
  ffdDataBuf->CPor = (float *) malloc(nPorts * sizeof(float));
  
  printf("interface_ffd.c: initialized shared memory.\n");
  getchar();
  return 0;
} // End of instantiate()


/******************************************************************************
| Exchange data between Modelica and Shared Memory
******************************************************************************/
int exchangeData(double t0, double dt, double *u, int nU, int nY, 
                 double t1, double *y)
{
  int i, j, imax = 10000;

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

  // Copy the modelica data to shared memory
  for(i=0; i<boundaryDataBuf->nSur; i++) 
    modelicaDataBuf->temHea[i] = (float) u[i];

  if(boundaryDataBuf->sha==1)
    for(j=0; j<boundaryDataBuf->nConExtWin; j++) {
      modelicaDataBuf->shaConSig[j] = (float) u[i+j];
      modelicaDataBuf->shaAbsRad[j] = (float) u[i+j+boundaryDataBuf->nConExtWin];
    }
  i = i + 2*boundaryDataBuf->nConExtWin;
  
  modelicaDataBuf->heaConvec = (float) u[i]; 
  i++;
  
  modelicaDataBuf->latentHeat = (float) u[i];
  i++;

  modelicaDataBuf->p = (float) u[i];
  i++;
 
  for(j=0; j<boundaryDataBuf->nPorts; j++) {
    modelicaDataBuf->mFloRatPor[j] = (float) u[i+j];
    modelicaDataBuf->TPor[j] = (float) u[i+j+boundaryDataBuf->nPorts];
    modelicaDataBuf->XiPor[j] = (float) u[i+j+2*boundaryDataBuf->nPorts];
    modelicaDataBuf->CPor[j] = (float) u[i+j+3*boundaryDataBuf->nPorts];;
  }

  // Set the flag to new data
  modelicaDataBuf->flag = 1;

  // If the data is not ready or not updated, check again
  while(ffdDataBuf->flag!=1)
    Sleep(1000);

  /*-----------------------------------------------------------------------
  | Copy data memory from shared memory
  ------------------------------------------------------------------------*/
  for(i=0; i<boundaryDataBuf->nSur; i++) 
    y[i] = ffdDataBuf->temHea[i];


  y[i] = ffdDataBuf->TRoo;
  
  if(boundaryDataBuf->sha==1)
    for(j=0; j<boundaryDataBuf->nConExtWin; j++) {
      y[i+j] = ffdDataBuf->TSha[j];
    }

  i = i + boundaryDataBuf->nConExtWin;

  for(j=0; j<boundaryDataBuf->nPorts; j++) {
    y[j+i] = ffdDataBuf->TPor[j];
    y[j+i+boundaryDataBuf->nPorts] = ffdDataBuf->XiPor[j];
    y[j+i+2*boundaryDataBuf->nPorts] = ffdDataBuf->CPor[j];
  }

  printf("\n FFD: time=%f, status=%d\n", ffdDataBuf->t,ffdDataBuf->flag);
//  printf("y1[0] = %f, y1[1] = %f, y1[2] = %f \n", y1[0], y1[1], y1[2]);
  printf("Modelica: time=%f, status=%d\n", modelicaDataBuf->t,modelicaDataBuf->flag);
//  printf("arr[0] = %f, arr[1] = %f, arr[2] = %f \n", modelicaDataBuf->arr[0], modelicaDataBuf->arr[1], modelicaDataBuf->arr[2]);

  // Update the data status
  ffdDataBuf->flag = 0;

  return 0;
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





