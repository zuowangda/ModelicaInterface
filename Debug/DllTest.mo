within ;
package DllTest "Test package for dll"
  function mapSharedMemory "map shared memory for data exchange"
    output String ffdDatNam;
    output String modDatNam;
  external"C" create_mapping(ffdDatNam, modDatNam);
    annotation (Include="#include <interface_ffd.h>", Library=
          "ModelicaInterface");
  end mapSharedMemory;

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
        __Dymola_Algorithm="Euler"), __Dymola_experimentSetupOutput);
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

  function instantiate "Start teh cosimulation"

  protected
    String ffdDatNam;
    String modDatNam;

  algorithm
    (ffdDatNam,modDatNam) := DllTest.launchFFD();
    (ffdDatNam,modDatNam) := DllTest.launchFFD();
    //DllTest.mapSharedMemory(ffdDatNam, modDatNam);
    DllTest.mapSharedMemory(ffdDatNam, modDatNam);
  end instantiate;

  function launchFFD "Launch FFD simulation"
    input String ffdDatNam;
    input String modDatNam;
  external"C" ffd_dll(ffdDatNam, modDatNam);
    annotation (Include="#include <ffd_dll.h>", Library="FFD-DLL");
  end launchFFD;

  model test

  equation
    DllTest.launchFFD();
  end test;
  annotation (uses(Modelica(version="3.2")));
end DllTest;
