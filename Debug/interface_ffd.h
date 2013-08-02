 __declspec(dllexport)
extern int instantiate(char **name, double *A, double *til, int *bouCon, int haveSensor,
                char **sensorName, int haveShade, int nSur, int nSen,
                int nConExtWin, int nPorts );

 __declspec(dllexport)
extern int exchangeData(double t0, double dt, double *u, int nU, int nY, 
                 double t1, double *y);

__declspec(dllexport)
extern void terminate_cosimulation( );


