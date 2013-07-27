within ;
package DllTest "Test package for dll"
  function instantiate "Start the cosimulation"
  external"C" instantiate();
    annotation (Include="#include <interface_ffd.h>", Library="ModelicaInterface");
  end instantiate;

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
    x1[4] = time;
    x2 = 0;
    y = DllTest.change(x1);
     annotation ();
  end ChangeDll;

  function change "change the variable in salve"
    input Real x1[4];
    output Real y[3];
  external"C"  exchangeData(x1, 0, "Modelica Data", y);
    annotation (Include="#include <interface_ffd.h>", Library="ModelicaInterface");
  end change;
  annotation (uses(Modelica(version="3.2")));
end DllTest;