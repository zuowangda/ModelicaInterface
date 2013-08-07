 __declspec(dllexport)
extern int instantiate(char **name, double *A, double *til, int *bouCon, 
                int nPorts, char** portNames, int haveSensor,
                char **sensorName, int haveShade, int nSur, int nSen,
                int nConExtWin);

 __declspec(dllexport)
extern void exchangeData(double t0, double dt, double *u, int nU, int nY,
                 double *t1, double *y, int *retVal);

__declspec(dllexport)
extern void terminate_cosimulation( );


