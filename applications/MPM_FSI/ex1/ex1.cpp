
#include "FemusInit.hpp"
#include "MultiLevelProblem.hpp"
#include "VTKWriter.hpp"
#include "TransientSystem.hpp"
#include "NonLinearImplicitSystem.hpp"
#include "Marker.hpp"
#include "Line.hpp"

#include "Fluid.hpp"
#include "Solid.hpp"
#include "Parameter.hpp"

#include "NumericVector.hpp"
#include "adept.h"

#include "../include/mpmFsi.hpp"

using namespace femus;




double SetVariableTimeStep (const double time) {
  double dt =  0.005/*0.008*/;
  return dt;
}

bool SetBoundaryCondition (const std::vector < double >& x, const char name[], double& value, const int facename, const double t) {
  bool test = 1; //dirichlet
  value = 0.;

  double H = 1.e-4;
  double U = 0.05;
  double t2 = t * t;

  if (!strcmp (name, "DX")) {
    if (3 == facename) {
      test = 0;
      value = 0;
    }
  }
  else if (!strcmp (name, "DY")) {
    if (2 == facename || 4 == facename) {
      test = 0;
      value = 0;
    }
  }
  else if (!strcmp (name, "VX")) {
    if (2 == facename) {
      test = 0;
      value = 0;
    }
    else if (4 == facename) {
      value = (U * t2 / sqrt (pow ( (0.04 - t2), 2.) + pow ( (0.1 * t), 2.))) * 4. * (H - x[1]) * x[1] / (H * H);
    }
  }
  else if (!strcmp (name, "VY")) {
    if (2 == facename) {
      test = 0;
      value = 0;
    }
  }
  else if (!strcmp (name, "P")) {
    test = 0;
    value = 0;
  }
  else if (!strcmp (name, "M")) {
    if (1 == facename) {
      test = 0;
      value = 0;
    }
  }

  return test;

}

int main (int argc, char** args) {

  // init Petsc-MPI communicator
  FemusInit mpinit (argc, args, MPI_COMM_WORLD);

  MultiLevelMesh mlMsh;
  double scalingFactor = 10000.;
  unsigned numberOfUniformLevels = 5; //for refinement in 3D
  //unsigned numberOfUniformLevels = 1;
  unsigned numberOfSelectiveLevels = 0;

  double Lref = 1.;
  double Uref = 1.;
  double rhos = 7850;
  double rhof = 1000;
  double nu = 0.3;
  double E = 2.e05;
  double muf = 1.0e-3;

  beta = 0.3;
  Gamma = 0.5;


  Parameter par (Lref, Uref);

  // Generate Solid Object
  Solid solid (par, E, nu, rhos, "Neo-Hookean");
  Fluid fluid (par, muf, rhof, "Newtonian");

  mlMsh.ReadCoarseMesh ("../input/fsi_bnc_2D.neu", "fifth", scalingFactor);
  mlMsh.RefineMesh (numberOfUniformLevels + numberOfSelectiveLevels, numberOfUniformLevels , NULL);

  mlMsh.EraseCoarseLevels (numberOfUniformLevels - 1);
  numberOfUniformLevels = 1;

  unsigned dim = mlMsh.GetDimension();

  MultiLevelSolution mlSol (&mlMsh);
  // add variables to mlSol
  mlSol.AddSolution ("DX", LAGRANGE, SECOND, 2);
  if (dim > 1) mlSol.AddSolution ("DY", LAGRANGE, SECOND, 2);
  if (dim > 2) mlSol.AddSolution ("DZ", LAGRANGE, SECOND, 2);

  mlSol.AddSolution ("VX", LAGRANGE, SECOND, 2);
  if (dim > 1) mlSol.AddSolution ("VY", LAGRANGE, SECOND, 2);
  if (dim > 2) mlSol.AddSolution ("VZ", LAGRANGE, SECOND, 2);

  mlSol.AddSolution ("P", DISCONTINUOUS_POLYNOMIAL, ZERO, 0);

  mlSol.AddSolution ("M", LAGRANGE, SECOND, 2);
  mlSol.AddSolution ("Mat", DISCONTINUOUS_POLYNOMIAL, ZERO, 0, false); 
  mlSol.AddSolution ("NodeFlag", LAGRANGE, SECOND, 0, false);
  mlSol.AddSolution ("NodeDist", LAGRANGE, SECOND, 0, false);
  //mlSol.AddSolution ("NodeDistS", LAGRANGE, SECOND, 2);

  mlSol.Initialize ("All");
  
   mlSol.AttachSetBoundaryConditionFunction (SetBoundaryCondition);

  // ******* Set boundary conditions *******
  mlSol.GenerateBdc ("DX", "Steady");
  if (dim > 1) mlSol.GenerateBdc ("DY", "Steady");
  if (dim > 2) mlSol.GenerateBdc ("DZ", "Steady");
  mlSol.GenerateBdc ("VX", "Time_dependent");
  if (dim > 1) mlSol.GenerateBdc ("VY", "Steady");
  if (dim > 2) mlSol.GenerateBdc ("VZ", "Steady");
  mlSol.GenerateBdc ("P", "Steady");
  mlSol.GenerateBdc ("M", "Steady");

  MultiLevelProblem ml_prob (&mlSol);

  ml_prob.parameters.set<Solid> ("SolidMPM") = solid;
  ml_prob.parameters.set<Solid> ("SolidFEM") = solid;
  ml_prob.parameters.set<Fluid> ("FluidFEM") = fluid;

  // ******* Add MPM system to the MultiLevel problem *******
  TransientNonlinearImplicitSystem& system = ml_prob.add_system < TransientNonlinearImplicitSystem > ("MPM_FSI");
  system.AddSolutionToSystemPDE ("DX");
  if (dim > 1) system.AddSolutionToSystemPDE ("DY");
  if (dim > 2) system.AddSolutionToSystemPDE ("DZ");
  system.AddSolutionToSystemPDE ("VX");
  if (dim > 1) system.AddSolutionToSystemPDE ("VY");
  if (dim > 2) system.AddSolutionToSystemPDE ("VZ");
  system.AddSolutionToSystemPDE ("P");

  // ******* System MPM-FSI Assembly *******
  system.SetAssembleFunction (AssembleMPMSys);
  //system.SetAssembleFunction (AssembleMPMSysOld);
  //system.SetAssembleFunction(AssembleFEM);
  // ******* set MG-Solver *******
  system.SetMgType (V_CYCLE);

  system.SetAbsoluteLinearConvergenceTolerance (1.0e-10);
  system.SetMaxNumberOfLinearIterations (1);
  system.SetNonLinearConvergenceTolerance (1.e-9);
  system.SetMaxNumberOfNonLinearIterations (20);

  system.SetNumberPreSmoothingStep (1);
  system.SetNumberPostSmoothingStep (1);

  // ******* Set Preconditioner *******
  system.SetLinearEquationSolverType (FEMuS_DEFAULT);

  system.init();

  // ******* Set Smoother *******
  system.SetSolverFineGrids (GMRES);

  system.SetPreconditionerFineGrids (ILU_PRECOND);

  system.SetTolerances (1.e-10, 1.e-15, 1.e+50, 40, 40);

//   unsigned rows = 2*60;
//   unsigned columns = 2*120;
//   unsigned size = rows * columns;
//
//   std::vector < std::vector < double > > x; // marker
//   std::vector < MarkerType > markerType;
//
//   x.resize(size);
//   markerType.resize(size);
//
//   std::vector < std::vector < std::vector < double > > > line(1);
//   std::vector < std::vector < std::vector < double > > > line0(1);
//
//   for (unsigned j = 0; j < size; j++) {
//     x[j].assign(dim, 0.);
//     markerType[j] = VOLUME;
//   }
//
//   //BEGIN initialization
//   for (unsigned i = 0; i < rows; i++) {
//     for (unsigned j = 0; j < columns; j++) {
//
//       x[i * columns + j][0] = -0.5 + ((0.625 - 0.000001) / (columns - 1)) * j;
//       x[i * columns + j][1] = -0.0625 + ((0.25 - 0.000001) / (rows - 1)) * i;
//       if (dim == 3) {
//         x[j][2] = 0.;
//       }
//     }
//   }
//   //END



  //return 1;

  double L = 50e-06; //beam dimensions
  double H = 5e-06;

  double xc = 0.98e-04 + 0.5 * H;
  double yc = 0.;

  double H0 = /*5. / 5.*/ 3. / 5.* H; //0.15: 3ref, 0.2: 4 ref, 0.225: 5 ref
  double L0 = L - (H - H0) / 2.;
  unsigned rows = 20; // 20: 3 ref, 40: 4 ref, 80: 5 ref
  double DH = H0 / (rows - 1);
  unsigned columns = static_cast < unsigned > (ceil (L0 / DH)) + 1;
  double DL = L0 / (columns - 1);
  unsigned size = rows * columns;

  std::vector < std::vector < double > > x; // marker
  std::vector < MarkerType > markerType;

  x.resize (size);
  markerType.resize (size);

  for (unsigned j = 0; j < size; j++) {
    x[j].assign (dim, 0.);
    markerType[j] = VOLUME;
  }

  //BEGIN initialization
  for (unsigned i = 0; i < rows; i++) {
    for (unsigned j = 0; j < columns; j++) {

      x[i * columns + j][1] = yc + (L0 / (columns - 1)) * j;
      x[i * columns + j][0] = xc - 0.5 * H0 + (H0 / (rows - 1)) * i;
      if (dim == 3) {
        x[j][2] = 0.;
      }
    }
  }
  //END

  double MASS = L0 * H0 * rhos;
  std::vector < double > mass (x.size(), MASS / x.size()); // uniform marker volume

//    x.resize(0);
//    mass.resize(0);

  if (fabs (H - H0) > 1.0e-10) {

    //double factor = 1.148; //1.148: 3 ref, 1.224: 5 ref, 1.2: 4 ref --> 21 layers.
    double factor = 1.; //1.148: 3 ref, 1.224: 5 ref, 1.2: 4 ref --> 21 layers.  
    unsigned NL = getNumberOfLayers (0.5 * (H - H0) / DH, factor);
    std::cout << NL << std::endl;

    double xs = xc - 0.5 * H0;
    double ys = yc;
    for (unsigned i = 1; i < NL; i++) {
      DL = DL / factor;
      DH = DH / factor;
      L0 += DL;
      H0 += 2 * DH;
      xs -= DL;
      columns = static_cast < unsigned > (ceil (L0 / DL)) + 1;
      rows = static_cast < unsigned > (ceil (H0 / DH)) + 1;

      double DL1 = L0 / (columns - 1);
      double DH1 = H0 / (rows - 1);

      unsigned sizeOld = x.size();
      x.resize (sizeOld  + 2 * columns + (rows - 2));
      for (unsigned s = sizeOld; s < x.size(); s++) {
        x[s].resize (dim, 0.);
      }
      double x1 = xs;
      double y1 = ys;
      unsigned counter = 0;
      for (unsigned j = 0; j < columns; j++) {
        x[sizeOld + counter][0] = x1;
        x[sizeOld + counter][1] = y1;
        counter++;
        y1 += DL1;
      }
      x1 += DH1;
      y1 -= DL1;
      for (unsigned j = 1; j < rows; j++) {
        x[sizeOld + counter][0] = x1;
        x[sizeOld + counter][1] = y1;
        counter++;
        x1 += DH1;
      }
      x1 -= DH1;
      y1 -= DL1;
      for (unsigned j = 1; j < columns; j++) {
        x[sizeOld + counter][0] = x1;
        x[sizeOld + counter][1] = y1;
        counter++;
        y1 -= DL1;
      }
      mass.resize (x.size(), rhos * DL * DH);
      markerType.resize (x.size(), VOLUME);
    }
  }

  double totalMass = 0;
  for (unsigned i = 0; i < mass.size(); i++) {
    totalMass += mass[i];
  }

  std::cout << totalMass << " " << rhos * H * L << std::endl;


  unsigned solType = 2;
  solidLine = new Line (x, mass, markerType, mlSol.GetLevel (numberOfUniformLevels - 1), solType);

  std::vector < std::vector < std::vector < double > > > lineS (1);
  solidLine->GetLine (lineS[0]);
  PrintLine (DEFAULT_OUTPUTDIR, "solidLine", lineS, 0);

  x.resize (0);
  mass.resize (0);
  markerType.resize (0);

  double H1 = 4. * H; //TODO tune the factor 4
  if (fabs (H - H0) > 1.0e-10) {

//     double factor = 1.148; //1.148: 3 ref, 1.224: 5 ref, 1.2: 4 ref --> 21 layers. //TODO
    double factor = 1.; //1.148: 3 ref, 1.224: 5 ref, 1.2: 4 ref --> 21 layers. //TODO
    unsigned NL = getNumberOfLayers (0.5 * (H1 - H0) / DH, factor, false);
    std::cout << NL << std::endl;

    double xs = xc - 0.5 * H0;
    double ys = yc;
    for (unsigned i = 1; i < NL; i++) {
      DL = DL * factor;
      DH = DH * factor;
      L0 += DL;
      H0 += 2 * DH;
      xs -= DL;
      columns = static_cast < unsigned > (ceil (L0 / DL)) + 1;
      rows = static_cast < unsigned > (ceil (H0 / DH)) + 1;

      double DL1 = L0 / (columns - 1);
      double DH1 = H0 / (rows - 1);

      unsigned sizeOld = x.size();
      x.resize (sizeOld  + 2 * columns + (rows - 2));
      for (unsigned s = sizeOld; s < x.size(); s++) {
        x[s].resize (dim, 0.);
      }
      double x1 = xs;
      double y1 = ys;
      unsigned counter = 0;
      for (unsigned j = 0; j < columns; j++) {
        x[sizeOld + counter][0] = x1;
        x[sizeOld + counter][1] = y1;
        counter++;
        y1 += DL1;
      }
      x1 += DH1;
      y1 -= DL1;
      for (unsigned j = 1; j < rows; j++) {
        x[sizeOld + counter][0] = x1;
        x[sizeOld + counter][1] = y1;
        counter++;
        x1 += DH1;
      }
      x1 -= DH1;
      y1 -= DL1;
      for (unsigned j = 1; j < columns; j++) {
        x[sizeOld + counter][0] = x1;
        x[sizeOld + counter][1] = y1;
        counter++;
        y1 -= DL1;
      }
      mass.resize (x.size(), rhof * DL * DH);
      markerType.resize (x.size(), VOLUME);
    }
  }

  totalMass = 0.;
  for (unsigned i = 0; i < mass.size(); i++) {
    totalMass += mass[i];
  }
  std::cout << totalMass << " " << rhof * (( 4 * H * (L + 1.5 * H) )- ( H * L )) << std::endl;

  fluidLine = new Line (x, mass, markerType, mlSol.GetLevel (numberOfUniformLevels - 1), solType);

  std::vector < std::vector < std::vector < double > > > lineF (1);
  fluidLine->GetLine (lineF[0]);
  PrintLine (DEFAULT_OUTPUTDIR, "fluidLine", lineF, 0);

   
  fluidLine->GetParticlesToGridMaterial (false);
  solidLine->GetParticlesToGridMaterial (true);
  GetParticlesToNodeFlag1(mlSol, *solidLine, *fluidLine);
   
  
  // ******* Print solution *******
  mlSol.SetWriter (VTK);

  std::vector<std::string> mov_vars;
  mov_vars.push_back ("DX");
  mov_vars.push_back ("DY");
  //mov_vars.push_back("DZ");
  mlSol.GetWriter()->SetMovingMesh (mov_vars);

  std::vector<std::string> print_vars;
  print_vars.push_back ("All");

  mlSol.GetWriter()->SetDebugOutput (true);
  mlSol.GetWriter()->Write (DEFAULT_OUTPUTDIR, "biquadratic", print_vars, 0);

  //return 1;


  system.AttachGetTimeIntervalFunction (SetVariableTimeStep);
  unsigned n_timesteps = 100;
  for (unsigned time_step = 1; time_step <= n_timesteps; time_step++) {

    if (time_step >= 3) {
      gravity[0]  = 0.;
    }

    system.CopySolutionToOldSolution();

    system.MGsolve();

    mlSol.GetWriter()->Write ("./output1", "biquadratic", print_vars, time_step);
    
    GridToParticlesProjection (ml_prob, *solidLine, *fluidLine);

    solidLine->GetLine (lineS[0]);
    PrintLine (DEFAULT_OUTPUTDIR, "solidLine", lineS, time_step);
    PrintLine ("./output1", "solidLine", lineS, time_step);

    fluidLine->GetLine (lineF[0]);
    PrintLine (DEFAULT_OUTPUTDIR, "fluidLine", lineF, time_step);
    PrintLine ("./output1", "fluidLine", lineF, time_step);

    mlSol.GetWriter()->Write (DEFAULT_OUTPUTDIR, "biquadratic", print_vars, time_step);
    
  }



  delete solidLine;
  delete fluidLine;
  return 0;

} //end main



