within ;
package DllTest "Test package for dll"
  function createSharedMemory "Creat shared memory for data exchange"
  output String ffdDatNam[100];
  output String modDatNam[100];

  external"C" instantiate(ffdDatNam, modDatNam);
    annotation (Include="#include <interface_ffd.h>", Library=
          "ModelicaInterface");
  end createSharedMemory;

  model ChangeDll
    Real x1[4];
    Integer x2;
    Real y[3];
  initial equation
  DllTest.instantiate();
  equation
    x1[1] = 1;
    x1[2] = 2;
    x1[3] = 3;
    x1[4] = 4;
    x2 = 0;
    when sample(0, 0.1) then
      y = DllTest.change(x1, time);

    end when;

    //when terminal() then
      //Fixme: Need to free the memory
      //DllTest.terminate();
    //end when;

    annotation (experiment(
        StopTime=1.1,
        __Dymola_fixedstepsize=0.1,
        __Dymola_Algorithm="Euler"),
                      __Dymola_experimentSetupOutput);
  end ChangeDll;

  function change "change the variable in salve"
    input Real x1[4];
    input Real t;
    output Real y[3];
  external"C" exchangeData(
        x1,
        t,
        "Modelica Data",
        y);
    annotation (Include="#include <interface_ffd.h>", Library=
          "ModelicaInterface");
  end change;

  function terminate "End the cosimulation"
  external"C" terminate_cosimulation();
    annotation (Include="#include <interface_ffd.h>", Library=
          "ModelicaInterface");
  end terminate;

  function instantiate "Start the cosimulation"
  protected
    String ffdDatNam[100];
    String modDatNam[100];
  algorithm
    (ffdDatNam, modDatNam) :=DllTest.createSharedMemory();
    //Modelica.Utilities.Streams.print(ffdDatNam, "modelica.log");

    DllTest.launchFFD(ffdDatNam, modDatNam);
  end instantiate;

  function launchFFD "Launch FFD simulation"
    input String ffdDatNam[100];
    input String modDatNam[100];

  external"C" ffd_dll(ffdDatNam, modDatNam);
    annotation (Include="#include <ffd_dll.h>", Library=
          "FFD-DLL");
  end launchFFD;

  model test

  equation
  DllTest.launchFFD();
  end test;
  annotation (uses(Modelica(version="3.2")));
end DllTest;
