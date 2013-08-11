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
__declspec(dllexport)
extern int instantiate(char **name, double *A, double *til, int *bouCon, 
                int nPorts, char** portName, int haveSensor,
                char **sensorName, int haveShade, int nSur, int nSen,
                int nConExtWin, int nC, int nXi);

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
 __declspec(dllexport)
extern int exchangeData(double t0, double dt, double *u, int nU, int nY,
                 double *t1, double *y);


 ///////////////////////////////////////////////////////////////////////////////
/// Terminate the FFD simulation
///
///\return 0 if no error occurred
///////////////////////////////////////////////////////////////////////////////
 __declspec(dllexport)
extern int stopCosim( );
