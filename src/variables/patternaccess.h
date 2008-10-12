/*
	*** Pattern Accessors
	*** src/variables/patternaccess.h
	Copyright T. Youngs 2007,2008

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

#ifndef ATEN_PATTERNACCESS_H
#define ATEN_PATTERNACCESS_H

#include "variables/vaccess.h"

// Pattern Accessor
class PatternAccessors : public VAccess
{
	public:
	// Constructor
	PatternAccessors();
	// Accessor list
	enum Accessors { Angles, Atoms, Bonds, FirstAtom, FirstAtomId, FField, LastAtom, LastAtomId, Name, NAngles, NAtoms, NBonds, NMolAtoms, NMols, NTorsions, Torsions, nAccessors };

	private:
	// Array of acessor pointers for look-up
	Variable *accessorPointers[PatternAccessors::nAccessors];

	public:
	// Retrieve specified data
	bool retrieve(void *classptr, AccessStep *step, ReturnValue &rv);
	// Set specified data
	bool set(void *classptr, AccessStep *step, Variable *sourcevar);
};

extern PatternAccessors patternAccessors;

#endif