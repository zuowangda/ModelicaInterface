#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <tchar.h>
#include "Debug/interface_ffd.h"
#include "../FFD-DLL/Fast-Fluid-Dynamics/modelica_ffd_common.h"

#pragma comment(lib, "user32.lib")

CosimulationData *cosim;

///////////////////////////////////////////////////////////////////////////////
/// Start the cosimulation
///
/// Allocate memory for the data exchange and launch FFD simulation
///
///\param name Pointer to the names of surfaces and fluid ports
///\param A Pointer to the area of surfaces in the same order of name
///\param til Pointer to the tilt of surface in the same order of name
///\param bouCon Pointer to the type of thermal bundary condition in the 
///       same order of name
///\param nPorts Number of fluid ports
///\param portName Pointer to the name of fluid ports
///\param haveSensor Flag: 1->have sensor; 0->No sensor
///\param sensorName Pointer to the names of the sensors used in CFD
///\param haveShade Flag: 1->have shade; 0->no shade
///\param nSur Number of surfaces
///\param nSen Number of sensors
///\param nConExtWin Number of exterior construction with windows
///\param nC Number of trace substances
///\param nXi Number of species
///
///\return 0 if no error occurred
///////////////////////////////////////////////////////////////////////////////
int instantiate(char **name, double *A, double *til, int *bouCon, 
                int nPorts, char** portName, int haveSensor,
                char **sensorName, int haveShade, int nSur, int nSen,
                int nConExtWin, int nC, int nXi) {
  int i, nBou;
  //*******************For call FFD-DLL.dll**********************
  typedef int (*MYPROC)(CosimulationData *);
  HINSTANCE hinstLib; 
  MYPROC ProcAdd;
//*************************************************************

  printf("interface_ffd.c: Start to allcoate memory for data exchange.\n");

  cosim = (CosimulationData *) malloc(sizeof(CosimulationData));
  cosim->para = (ParameterSharedData *) malloc(sizeof(ParameterSharedData));  
  cosim->modelica = (ModelicaSharedData *) malloc(sizeof(ModelicaSharedData)); 
  cosim->ffd = (ffdSharedData *) malloc(sizeof(ffdSharedData)); 
  
  /*---------------------------------------------------------------------------
  | allocate the memory and assign the data
  --------------------------------------------------------------------------*/
  cosim->para->nSur = nSur;
  cosim->para->nSen = nSen;
  cosim->para->nConExtWin= nConExtWin;
  cosim->para->nPorts = nPorts;
  cosim->para->sha = haveShade;
  cosim->para->nC = nC;
  cosim->para->nXi = nXi;

  nBou = nSur + nPorts;

  cosim->para->name = (char**) malloc(nSur*sizeof(char *));
  cosim->para->are = (float *) malloc(nSur*sizeof(float));
  cosim->para->til = (float *) malloc(nSur*sizeof(float));
  cosim->para->bouCon = (int *) malloc(nSur*sizeof(int));

  for(i=0; i<nSur; i++) { 
    cosim->para->name[i] = (char *)malloc(sizeof(char) *strlen(name[i]));
    strcpy(cosim->para->name[i], name[i]);
    printf("Boundary name:%s\n", cosim->para->name[i]);

    cosim->para->are[i] = (float) A[i];
    printf("\tA->Area:%f->%f [m2]\n", A[i], cosim->para->are[i]);

    cosim->para->til[i] = (float) til[i];
    printf("\tTilt->Tilt:%f->%f [deg]\n", til[i], cosim->para->til[i]);

    cosim->para->bouCon[i] = bouCon[i];
    printf("\tbouCon->bouCon:%d->%d \n\n", bouCon[i], cosim->para->bouCon[i]);
  }

  cosim->para->portName = (char**) malloc(nPorts*sizeof(char *));

  for(i=0; i<nPorts; i++) {
    cosim->para->portName[i] = (char *)malloc(sizeof(char)*strlen(portName[i]));
    strcpy(cosim->para->portName[i], portName[i]);
    printf("Boundary name:%s\n", cosim->para->portName[i]);
  }

  if(haveSensor) {
    cosim->para->sensorName = (char **) malloc(nSen*sizeof(char *));
    for(i=0; i<nSen; i++) {
      cosim->para->sensorName[i] = (char *)malloc(sizeof(char)*strlen(sensorName[i]));
      strcpy(cosim->para->sensorName[i], sensorName[i]);
      printf("Sensor Name:%s\n", cosim->para->sensorName[i]);
    }
  }

  // Set the flag to initial value
  cosim->modelica->flag = 0;
  cosim->ffd->flag = 0;
  cosim->para->flag = 1;

  cosim->modelica->temHea = (float *) malloc(nSur*sizeof(float));
  // Having a shade for window
  if(haveShade==1) {
    cosim->modelica->shaConSig = (float *) malloc(nConExtWin*sizeof(float));
    cosim->modelica->shaAbsRad = (float *) malloc(nConExtWin*sizeof(float));
  }
  cosim->modelica->mFloRatPor = (float *) malloc(nPorts*sizeof(float));
  cosim->modelica->TPor = (float *) malloc(nPorts*sizeof(float));

  cosim->modelica->XiPor = (float **) malloc(nPorts*sizeof(float *));
  cosim->ffd->XiPor = (float **) malloc(nPorts*sizeof(float *));
  for(i=0; i<cosim->para->nXi; i++) {
    cosim->modelica->XiPor[i] = (float *) malloc(cosim->para->nXi*sizeof(float));
    cosim->ffd->XiPor[i] = (float *) malloc(cosim->para->nXi*sizeof(float));
  }

  cosim->modelica->CPor = (float **) malloc(nPorts*sizeof(float *));
  cosim->ffd->CPor = (float **) malloc(nPorts*sizeof(float *));
  for(i=0; i<cosim->para->nC; i++) {
    cosim->modelica->CPor[i] = (float *) malloc(cosim->para->nC*sizeof(float));
    cosim->ffd->CPor[i] = (float *) malloc(cosim->para->nC*sizeof(float));
  }

  cosim->ffd->temHea = (float *) malloc(nSur*sizeof(float));
  if(haveShade==1) cosim->ffd->TSha = (float *) malloc(nConExtWin*sizeof(float));
  cosim->ffd->TPor = (float *) malloc(nPorts*sizeof(float));
  
  printf("interface_ffd.c: Allocated memory for cosimulation data.\n");

  //**********************************************************************
  // Get a handle to the DLL module.
  hinstLib = LoadLibrary(TEXT("../../../FFD/FFD-DLL/FFD-DLL/Debug/FFD-DLL.dll")); 

  // If the handle is valid, try to get the function address.
  if(hinstLib != NULL) {
    ProcAdd = (MYPROC) GetProcAddress(hinstLib, "ffd_dll");
  }
  else {
    printf("instantiate(): Could not find dll handle.\n");
    return 1;
  }

  // If the function address is valid, call the function.
    if (NULL!=ProcAdd) {
      ProcAdd(cosim);   //call function: passing pointer of NAME struct
    }
    else{
      printf("instantiate(): Could not find dll function address.\n");
      return 1;
    }

  return 0;
} // End of instantiate()


///////////////////////////////////////////////////////////////////////////////
/// Exchange the data between Modelica and CFD
///
/// Allocate memory for the data exchange and launch FFD simulation
///
///\param t0 Current time of integration for Modelica
///\param dt Time step size for next synchronization define dby Modelica
///\param u Pointer to the input data from Modleica to CFD
///\param nU Number of inputs from Modelica to CFD
///\param nY Number of outputs from CFD to Modelica
///\param y Pointer to the output data from CFD to Modelica
///
///\return 0 if no error occurred
///////////////////////////////////////////////////////////////////////////////
int exchangeData(double t0, double dt, double *u, int nU, int nY,
                 double *t1, double *y) {
  int i, j, k, imax = 10000;
  
  printf("---------------------------------------------------\n");
  printf("exchangeData(): start to exchagne data at t=%f\n", t0);
  /*--------------------------------------------------------------------------
  | Write data to FFD
  | Command: 
  | -1: feak data
  |  0: data has been read by the other program
  |  1: data waiting for the other program to read
  --------------------------------------------------------------------------*/
  // If previous data hasn't been read, wait
  while(cosim->modelica->flag==1) {
    printf("exchangeData(): Waiting for the FFD to read my data.\n");
    Sleep(1000);
  }

  printf("exchangeData(): Start to write data");
  cosim->modelica->t = (float) t0;
  cosim->modelica->dt = (float) dt;

  printf("exchangeData():: wrtie data at %f with dt=%f\n", 
         cosim->modelica->t, cosim->modelica->dt);

  // Copy the modelica data to shared memory
  for(i=0; i<cosim->para->nSur; i++) {
    cosim->modelica->temHea[i] = (float) u[i];
    printf("temHea[%d] = %f\n", i, cosim->modelica->temHea[i]); 
  }

  if(cosim->para->sha==1)
    for(j=0; j<cosim->para->nConExtWin; j++) {
      cosim->modelica->shaConSig[j] = (float) u[i+j];
      cosim->modelica->shaAbsRad[j] = (float) u[i+j+cosim->para->nConExtWin];
      printf("shaConSig[%d] = %f, shaAbsRad[%d] = %f\n", 
             cosim->modelica->shaConSig[j],
             cosim->modelica->shaAbsRad[j]);
    }
  i = i + 2*cosim->para->nConExtWin;
  
  cosim->modelica->heaConvec = (float) u[i]; 
  printf("heaConvec = %f\n", cosim->modelica->heaConvec);
  i++;
  
  cosim->modelica->latentHeat = (float) u[i];
  i++;

  cosim->modelica->p = (float) u[i];
  i++;
 
  for(j=0; j<cosim->para->nPorts; j++) {
    cosim->modelica->mFloRatPor[j] = (float) u[i+j];    
    cosim->modelica->TPor[j] = (float) u[i+j+cosim->para->nPorts];
  }

  i = i + 2*cosim->para->nPorts;
  for(j=0; j<cosim->para->nPorts; j++)
    for(k=0; k<cosim->para->nXi; k++)
      cosim->modelica->XiPor[j][k] = (float) u[i+j*cosim->para->nXi+k];

  i = i + cosim->para->nPorts*cosim->para->nXi;
  for(j=0; j<cosim->para->nPorts; j++)
    for(k=0; k<cosim->para->nC; k++)
    cosim->modelica->CPor[j][k] = (float) u[i+j*cosim->para->nC+k];

  // Set the flag to new data
  cosim->modelica->flag = 1;

  // If the data is not ready or not updated, check again
  while(cosim->ffd->flag!=1)
    Sleep(1000);

  /*-----------------------------------------------------------------------
  | Copy data from FFD
  ------------------------------------------------------------------------*/
  for(i=0; i<cosim->para->nSur; i++) 
    y[i] = cosim->ffd->temHea[i];

  y[i] = cosim->ffd->TRoo;
  
  if(cosim->para->sha==1)
    for(j=0; j<cosim->para->nConExtWin; j++) {
      y[i+j] = cosim->ffd->TSha[j];
    }

  i = i + cosim->para->nConExtWin;

  for(j=0; j<cosim->para->nPorts; j++)
    y[j+i] = cosim->ffd->TPor[j];

  i = i + cosim->para->nPorts;

  for(j=0; j<cosim->para->nPorts; j++)
    for(k=0; k<cosim->para->nXi; k++)
       y[i+j*cosim->para->nXi+k] = cosim->ffd->XiPor[j][k];
  
  i = i + cosim->para->nPorts*cosim->para->nC;
  for(j=0; j<cosim->para->nPorts; j++)
    for(k=0; k<cosim->para->nC; k++)
       y[i+j*cosim->para->nC+k] = cosim->ffd->CPor[j][k];

  printf("\n FFD: \t\ttime=%f, status=%d\n", cosim->ffd->t, cosim->ffd->flag);
  printf("Modelica: \ttime=%f, status=%d\n", cosim->modelica->t, cosim->modelica->flag);

  // Update the data status
  cosim->ffd->flag = 0;

  *t1 = cosim->ffd->t;
  for(i=0; i<nY; i++)
    y[i] = 273.15;

  return 0;
} // End of exchangeData()

///////////////////////////////////////////////////////////////////////////////
/// Terminate the co-simulation
///
///\return 0 if no error occurred
///////////////////////////////////////////////////////////////////////////////
int sendStopCommand( ) {
  cosim->para->flag = 0;
  printf("sendStopCommand( ): Set cosim->para->flag = %d\n", cosim->para->flag);
} // End of ssendStopCommand 

///////////////////////////////////////////////////////////////////////////////
/// Terminate the co-simulation
///
///\return 0 if no error occurred
///////////////////////////////////////////////////////////////////////////////
int stopCosim( ) {
  int i = 0, imax = 10000, flag;
  
  // Wait for the feedback from FFD
  while(cosim->para->flag==0 && i<imax) {
    Sleep(10000);
    i++;
  }
  
  if(i<imax) {
    printf("Successfully stopped the FFD simulation.\n");
    flag = 0;
  }
  else {
    printf("stopFFD(): Could not stop the FFD simulation in reqruied time.\n");
    flag = 1;
  }

  free(cosim->para);
  free(cosim->modelica);
  free(cosim->ffd);
  free(cosim);

  return flag;
} // End of stopCosim


