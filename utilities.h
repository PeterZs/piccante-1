/* Copyright 2014 - Andrea Sgattoni, Luca Fedeli, Stefano Sinigardi */

/*******************************************************************************
This file is part of piccante.

piccante is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

piccante is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with piccante.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <iomanip>
#if defined(_MSC_VER)
#include <cstdint>
#else
#include <stdint.h>
#endif
#include <vector>
#include <list>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include "commons.h"
#if defined(USE_BOOST)
#include <boost/filesystem.hpp>
#endif
#include "grid.h"
#include "structures.h"
#include "em_field.h"
#include "particle_species.h"

void moveWindow(GRID* _mygrid, EM_FIELD* _myfield, std::vector<SPECIE*> _myspecies);

void restartFromDump(int *dumpID, GRID* _mygrid, EM_FIELD* _myfield, std::vector<SPECIE*> _myspecies);
void dumpFilesForRestart(int *dumpID, GRID* _mygrid, EM_FIELD* _myfield, std::vector<SPECIE*> _myspecies);


#endif // UTILITIES_H

