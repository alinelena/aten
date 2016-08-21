/*
        *** Common functions for MOPAC71 plugins
        *** src/plugins/method_mopac71/common.cpp
        Copyright T. Youngs 2016-2016

        This file is part of Aten.
    
        Aten is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or 
        (at your option) any later version.
                               
        Aten is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with Aten.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "plugins/method_mopac71/common.h"

ATEN_USING_NAMESPACE

// Initialise Plugin Options 
void MOPAC71Common::initialiseOptions(KVMap& options)
{
	options.add("jobtype", "BFGS");
	options.add("hamiltonian", "PM3");
	options.add("scftype", "RHF");
	options.add("state", "SINGLET");
	options.add("charge", "0");
	options.add("internal", "false");
	options.add("precise", "false");
	options.add("campking", "false");
	options.add("extra", "");
}

