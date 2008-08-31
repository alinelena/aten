/*
	*** Aten Access
	*** src/aten/atenaccess.cpp
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

#include "main/aten.h"
#include "base/vaccess.h"

// Singleton declaration
AtenAccessors atenAccessors;

// Constructor
AtenAccessors::AtenAccessors()
{
	addAccessor("models",	VObject::ListArray,	VObject::ModelData,	FALSE);
}

// Retriever
void AtenAccessors::retrieveData(VObject source, int accessorid, VResult &result)
{
	// Cast accessorid into local enum
	if ((accessorid < 0) || (accessorid >= nAccessors))
	{
		printf("Critical error accessing object in Aten - accessor id %i is out of range.\n", accessorid);
		result.setType(VObject::NoDataSet);
		return;
	}
}