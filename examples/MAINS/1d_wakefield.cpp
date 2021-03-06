/*   Copyright 2014-2016 - Andrea Sgattoni, Luca Fedeli, Stefano Sinigardi   */

/******************************************************************************
* This file is part of piccante.                                              *
*                                                                             *
* piccante is free software: you can redistribute it and/or modify            *
* it under the terms of the GNU General Public License as published by        *
* the Free Software Foundation, either version 3 of the License, or           *
* (at your option) any later version.                                         *
*                                                                             *
* piccante is distributed in the hope that it will be useful,                 *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
* GNU General Public License for more details.                                *
*                                                                             *
* You should have received a copy of the GNU General Public License           *
* along with piccante. If not, see <http://www.gnu.org/licenses/>.            *
******************************************************************************/

#include <mpi.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <vector>

#include "commons.h"
#include "grid.h"
#include "structures.h"
#include "current.h"
#include "em_field.h"
#include "particle_species.h"
#include "output_manager.h"
#include "utilities.h"

#define DIMENSIONALITY 1
#define NPROC_ALONG_Y 1
#define NPROC_ALONG_Z 1

#define _RESTART_FROM_DUMP 1
#define _DO_RESTART false
#define DO_DUMP true
#define TIME_BTW_DUMP 50

#define DIRECTORY_OUTPUT "TEST"
#define DIRECTORY_DUMP "TEST"
#define RANDOM_NUMBER_GENERATOR_SEED 5489
#define FREQUENCY_STDOUT_STATUS 5

int main(int narg, char **args)
{
  GRID grid(DIMENSIONALITY);
  EM_FIELD myfield;
  CURRENT current;
  std::vector<SPECIE*> species;
  std::vector<SPECIE*>::const_iterator spec_iterator;
  int istep;
  my_rng_generator rng;

  //*******************************************BEGIN GRID DEFINITION*******************************************************

  grid.setXrange(-50.0, 0.0);
  grid.setYrange(-1.0, 1.0);
  grid.setZrange(-1.0, +1.0);

  grid.setNCells(2500, 0, 0);
  grid.setNProcsAlongY(NPROC_ALONG_Y);
  grid.setNProcsAlongZ(NPROC_ALONG_Z);

  //grid.enableStretchedGrid();
  //grid.setXandNxLeftStretchedGrid(-15.0,1000);
  //grid.setYandNyLeftStretchedGrid(-5.0, 70);
  //grid.setXandNxRightStretchedGrid(15.0,1000);
  //grid.setYandNyRightStretchedGrid(5.0, 70);

  grid.setBoundaries(xOpen | yOpen | zPBC);
  grid.mpi_grid_initialize(&narg, args);
  grid.setCourantFactor(0.98);

  grid.setSimulationTime(60.0);

  grid.withParticles = YES;
  grid.withCurrent = YES;

  grid.setStartMovingWindow(0);
  grid.setBetaMovingWindow(1.0);
  //grid.setFrequencyMovingWindow(FREQUENCY);

  grid.setMasterProc(0);

  srand((unsigned int)time(NULL));
  grid.initRNG(rng, RANDOM_NUMBER_GENERATOR_SEED);

  grid.finalize();

  grid.visualDiag();
  //********************************************END GRID DEFINITION********************************************************

  //*******************************************BEGIN FIELD DEFINITION*********************************************************

  myfield.allocate(&grid);
  myfield.setAllValuesToZero();

  laserPulse pulse1;
  pulse1.setCos2PlaneWave();
  pulse1.setPPolarization();
  pulse1.setDurationFWHM(5.0);
  pulse1.setPulseInitialPosition(-6.0);
  pulse1.setLambda(1.0);
  pulse1.setNormalizedAmplitude(1.0);

  //pulse1.setWaist(3.0);
  //pulse1.setFocusPosition(0.0);
  //pulse1.setRotationAngleAndCenter(2.0*M_PI*(-90.0 / 360.0),0.0);

  myfield.addPulse(&pulse1);

  myfield.boundary_conditions();

  current.allocate(&grid);
  current.setAllValuesToZero();

  //*******************************************END FIELD DEFINITION***********************************************************

  //*******************************************BEGIN SPECIES DEFINITION*********************************************************

  PLASMA plasma1;
  plasma1.density_function = left_soft_ramp;
  plasma1.setXRangeBox(0.0, 100.0);
  plasma1.setYRangeBox(grid.rmin[1], grid.rmax[1]);
  plasma1.setZRangeBox(grid.rmin[2], grid.rmax[2]);
  plasma1.setRampLength(20.0);
  plasma1.setDensityCoefficient(0.01);
  plasma1.setRampMinDensity(0.0);


  SPECIE  electrons1(&grid);
  electrons1.plasma = plasma1;
  electrons1.setParticlesPerCellXYZ(100, 1, 1);
  electrons1.setName("ELE1");
  electrons1.type = ELECTRON;
  electrons1.creation();
  species.push_back(&electrons1);

  SPECIE ions1(&grid);
  ions1.plasma = plasma1;
  ions1.setParticlesPerCellXYZ(100, 1, 1);
  ions1.setName("ION1");
  ions1.type = ION;
  ions1.Z = 6.0;
  ions1.A = 12.0;
  ions1.creation();
  species.push_back(&ions1);

  tempDistrib distribution;
  distribution.setMaxwell(1.0);

  electrons1.add_momenta(rng, 0.0, 0.0, 0.0, distribution);
  ions1.add_momenta(rng, 0.0, 0.0, 0.0, distribution);

  //*******************************************END SPECIES DEFINITION***********************************************************

  //*******************************************BEGIN DIAG DEFINITION**************************************************

  OUTPUT_MANAGER manager(&grid, &myfield, &current, species);

  manager.addEFieldFrom(0.0, 2.0);
  manager.addBFieldFrom(0.0, 2.0);

  manager.addSpeciesDensityFrom(electrons1.name, 0.0, 2.0);
  manager.addSpeciesDensityFrom(ions1.name, 0.0, 2.0);

  manager.addCurrentFrom(0.0, 5.0);

  manager.addSpeciesPhaseSpaceFrom(electrons1.name, 0.0, 5.0);

  manager.addDiagFrom(0.0, 1.0);

  manager.initialize(DIRECTORY_OUTPUT);

  //*******************************************END DIAG DEFINITION**************************************************

  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ MAIN CYCLE (DO NOT MODIFY) @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  if (grid.myid == grid.master_proc) {
    printf("----- START temporal cycle -----\n");
    fflush(stdout);
  }

  int Nstep = grid.getTotalNumberOfTimesteps();
  int dumpID = 1, dumpEvery;
  if (DO_DUMP) {
    dumpEvery = (int)(TIME_BTW_DUMP / grid.dt);
  }
  grid.istep = 0;
  if (_DO_RESTART) {
    dumpID = _RESTART_FROM_DUMP;
    UTILITIES::restartFromDump(&dumpID, &grid, &myfield, species);
  }
  while (grid.istep <= Nstep)
  {

    grid.setFrequencyStdoutStatus(FREQUENCY_STDOUT_STATUS);


    manager.callDiags(grid.istep);

    myfield.openBoundariesE_1();
    myfield.new_halfadvance_B();
    myfield.boundary_conditions();

    current.setAllValuesToZero();

    for (spec_iterator = species.begin(); spec_iterator != species.end(); spec_iterator++) {
      (*spec_iterator)->current_deposition_standard(&current);
    }

    current.pbc();

    for (spec_iterator = species.begin(); spec_iterator != species.end(); spec_iterator++) {
      (*spec_iterator)->position_parallel_pbc();
    }

    myfield.openBoundariesB();
    myfield.new_advance_E(&current);

    myfield.boundary_conditions();

    myfield.openBoundariesE_2();
    myfield.new_halfadvance_B();

    myfield.boundary_conditions();

    for (spec_iterator = species.begin(); spec_iterator != species.end(); spec_iterator++) {
      (*spec_iterator)->momenta_advance(&myfield);
    }

    grid.time += grid.dt;


    UTILITIES::moveWindow(&grid, &myfield, species);

    grid.istep++;
    if (DO_DUMP) {
      if (grid.istep != 0 && !(grid.istep % (dumpEvery))) {
        UTILITIES::dumpFilesForRestart(&dumpID, &grid, &myfield, species);
      }
    }
  }

  manager.close();
  MPI_Finalize();

  return 0;

}
