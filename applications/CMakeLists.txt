#############################################################################################
### Build Applications
#############################################################################################

IF(BUILD_NAVIERSTOKESAPPS)
  set(NS_FOLDER NavierStokes/)
  # Steady NavierStokes
  ADD_SUBDIRECTORY(${NS_FOLDER}/SteadyNavierStokesParallel)

  # Time-dependent BE NavierStokes
  ADD_SUBDIRECTORY(${NS_FOLDER}/BackwardEulerNavierStokesParallel/)

  # Time-dependent Generalized-alpha NavierStokes
  ADD_SUBDIRECTORY(${NS_FOLDER}/TaylorGreenVortexFlowGeneralisedalpha/)

  # Steady Stokes
  ADD_SUBDIRECTORY(${NS_FOLDER}/SteadyStokes/)

  # Steady Navier Stokes steady
  ADD_SUBDIRECTORY(${NS_FOLDER}/NavierStokesTemp_AMR/)
  unset(NS_FOLDER)
ENDIF(BUILD_NAVIERSTOKESAPPS)


IF(BUILD_FSIAPPS)
  set(FSI_FOLDER FSI/)
  ADD_SUBDIRECTORY(${FSI_FOLDER})
  unset(FSI_FOLDER)
  
ENDIF(BUILD_FSIAPPS)


IF(BUILD_OPTCONTROLAPPS)
  ADD_SUBDIRECTORY(OptimalControl/)
ENDIF(BUILD_OPTCONTROLAPPS)

IF(BUILD_POISSON)
  ADD_SUBDIRECTORY(Poisson/)
ENDIF(BUILD_POISSON)

IF(BUILD_POISSON_AMR)
  ADD_SUBDIRECTORY(Poisson_AMR/)
ENDIF(BUILD_POISSON_AMR)

IF(BUILD_TUTORIAL)
  ADD_SUBDIRECTORY(tutorial/)
ENDIF(BUILD_TUTORIAL)

IF(BUILD_WILLMORE)
  ADD_SUBDIRECTORY(Willmore/)
ENDIF(BUILD_WILLMORE)

IF(BUILD_MGAMR)
  ADD_SUBDIRECTORY(MGAMR)
ENDIF(BUILD_MGAMR)

IF(BUILD_PCFIELDSPLIT)
  ADD_SUBDIRECTORY(PCFieldSplit)
ENDIF(BUILD_PCFIELDSPLIT)

IF(BUILD_AMR)
  ADD_SUBDIRECTORY(AMR)
ENDIF(BUILD_AMR)

IF(BUILD_ISM)
  ADD_SUBDIRECTORY(ISM/)
ENDIF(BUILD_ISM)

IF(BUILD_MPM_FEM)
  ADD_SUBDIRECTORY(MPM_FEM/)
ENDIF(BUILD_MPM_FEM)

IF(BUILD_MPM_FSI)
  ADD_SUBDIRECTORY(MPM_FSI/)
ENDIF(BUILD_MPM_FSI)

IF(BUILD_REFINEMENT)
  ADD_SUBDIRECTORY(Refinement/)
ENDIF(BUILD_REFINEMENT)

IF(BUILD_PETSC_DEBUG)
  ADD_SUBDIRECTORY(petscDebug/)
ENDIF(BUILD_PETSC_DEBUG)

IF(BUILD_UQ)
  ADD_SUBDIRECTORY(UQ/)
ENDIF(BUILD_UQ)

IF(BUILD_NONLOCAL)
  ADD_SUBDIRECTORY(NonLocal/)
ENDIF(BUILD_NONLOCAL)

IF(BUILD_SW)
  ADD_SUBDIRECTORY(SW/)
ENDIF(BUILD_SW)

IF(BUILD_TUMOR)
  ADD_SUBDIRECTORY(Tumor/)
ENDIF(BUILD_TUMOR)

IF(BUILD_RK)
  ADD_SUBDIRECTORY(RK/)
ENDIF(BUILD_RK)

IF(BUILD_CONFORMAL)
  ADD_SUBDIRECTORY(Conformal/)
ENDIF(BUILD_CONFORMAL)

IF(BUILD_NITSCHE)
  ADD_SUBDIRECTORY(Nitsche/)
ENDIF(BUILD_NITSCHE)
