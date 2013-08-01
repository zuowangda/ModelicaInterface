 __declspec(dllexport)
extern int instantiate(int nSur, int nConExtWin, int nPorts, int sha,
                char **name, float *are, float *til, int *bouCon);

 __declspec(dllexport)
extern void exchangeData(double t0, double dt, double *temHea0, double heaConvec, 
                  double *shaConSig, double *shaAbsRad, double p, 
                  double *mFloRatPor0, double *TPor0, double *XiPor0, double *CPor0,
                  double t1, double *temHea1, double TRoo, double *TSha, 
                  double *TPor1, double *XiPor1, double *CPor1);

__declspec(dllexport)
extern void terminate_cosimulation( );


