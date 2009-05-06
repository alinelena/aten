/*
	*** Pattern Variable
	*** src/parser/pattern.cpp
	Copyright T. Youngs 2007-2009

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

#include "parser/pattern.h"
#include "parser/stepnode.h"
#include "base/atom.h"
#include "base/pattern.h"
#include "base/elements.h"
#include "model/model.h"
#include <string.h>

// Constructor
PatternVariable::PatternVariable(Pattern *ptr, bool constant) : patternData_(ptr)
{
	// Private variables
	returnType_ = VTypes::PatternData;
	readOnly_ = constant;
}

// Destructor
PatternVariable::~PatternVariable()
{
}

/*
// Set / Get
*/

// Set value of variable
bool PatternVariable::set(ReturnValue &rv)
{
	if (readOnly_)
	{
		msg.print("A constant value (in this case a pattern&) cannot be assigned to.\n");
		return FALSE;
	}
	bool success;
	patternData_ = rv.asPointer(VTypes::PatternData, success);
	return success;
}

// Reset variable
void PatternVariable::reset()
{
	patternData_ = NULL;
}

// Return value of node
bool PatternVariable::execute(ReturnValue &rv)
{
	// If this vector is a constant, read the three stored expressions to recreate it
	rv.set(VTypes::PatternData, patternData_);
	return TRUE;
}

// Print node contents
void PatternVariable::nodePrint(int offset, const char *prefix)
{
	// Construct tabbed offset
	char *tab;
	tab = new char[offset+32];
	tab[0] = '\0';
	for (int n=0; n<offset-1; n++) strcat(tab,"\t");
	if (offset > 1) strcat(tab,"   |--> ");
	strcat(tab,prefix);
	// Output node data
	if (readOnly_) printf("%s%li (pattern) (constant value)\n", tab, patternData_);
	else printf("%s%li (pattern) (variable, name=%s)\n", tab, patternData_, name_.get());
	delete[] tab;
}

/*
// Accessors
*/

// Accessor data
Accessor PatternVariable::accessorData[PatternVariable::nAccessors] = {
	{ "angles", 	VTypes::PatternBoundData,	-1, TRUE },
	{ "atoms", 	VTypes::ForcefieldAtomData,	-1, TRUE },
	{ "bonds", 	VTypes::PatternBoundData,	-1, TRUE },
	{ "cog", 	VTypes::VectorData,		-1, TRUE },
	{ "com", 	VTypes::VectorData,		-1, TRUE },
	{ "firstatom",	VTypes::AtomData,		0, TRUE },
	{ "firstatomid",VTypes::IntegerData,		0, TRUE },
	{ "forcefield",	VTypes::ForcefieldData,		0, FALSE },
	{ "lastatom",	VTypes::AtomData,		0, TRUE },
	{ "lastatomid",	VTypes::IntegerData,		0, TRUE },
	{ "name",	VTypes::StringData,		0, FALSE },
	{ "nangles",	VTypes::IntegerData,		0, TRUE },
	{ "natoms",	VTypes::IntegerData,		0, TRUE },
	{ "nbonds",	VTypes::IntegerData,		0, TRUE },
	{ "nmolatoms",	VTypes::IntegerData,		0, TRUE },
	{ "nmols",	VTypes::IntegerData,		0, TRUE },
	{ "ntorsions",	VTypes::IntegerData,		0, TRUE },
	{ "torsions",	VTypes::PatternBoundData,	-1, TRUE }
};

// Search variable access list for provided accessor (call private static function)
StepNode *PatternVariable::findAccessor(const char *s, TreeNode *arrayindex)
{
	return PatternVariable::accessorSearch(s, arrayindex);
}

// Private static function to search accessors
StepNode *PatternVariable::accessorSearch(const char *s, TreeNode *arrayindex)
{
	msg.enter("PatternVariable::accessorSearch");
	StepNode *result = NULL;
	int i = 0;
	for (i = 0; i < nAccessors; i++) if (strcmp(accessorData[i].name,s) == 0) break;
	if (i == nAccessors)
	{
		msg.print("Error: Type 'pattern&' has no member named '%s'.\n", s);
		msg.exit("PatternVariable::accessorSearch");
		return NULL;
	}
	// Create a suitable AccessNode to return...
	msg.print(Messenger::Parse, "Accessor match = %i (%s)\n", i, accessorData[i].name);
	result = new StepNode(i, VTypes::PatternData, arrayindex, accessorData[i].returnType, accessorData[i].isReadOnly, accessorData[i].arraySize != 0);
	msg.exit("PatternVariable::accessorSearch");
	return result;
}

// Retrieve desired value
bool PatternVariable::retrieveAccessor(int i, ReturnValue &rv, bool hasArrayIndex, int arrayIndex)
{
	msg.enter("PatternVariable::retrieveAccessor");
	// Cast 'i' into Accessors enum value
	if ((i < 0) || (i >= nAccessors))
	{
		printf("Internal Error: Accessor id %i is out of range for Pattern type.\n");
		msg.exit("PatternVariable::retrieveAccessor");
		return FALSE;
	}
	Accessors acc = (Accessors) i;
	// Check for correct lack/presence of array index given
	if ((accessorData[i].arraySize == 0) && hasArrayIndex)
	{
		msg.print("Error: Unnecessary array index provided for member '%s'.\n", accessorData[i].name);
		msg.exit("PatternVariable::retrieveAccessor");
		return FALSE;
	}
	// Get current data from ReturnValue
	bool result = TRUE;
	Pattern *ptr= (Pattern*) rv.asPointer(VTypes::PatternData, result);
	if (result) switch (acc)
	{
		case (PatternVariable::Angles):
			rv.set(VTypes::PatternBoundData, ptr->angle(arrayIndex-1));
			break;
		case (PatternVariable::Atoms):
			rv.set(VTypes::AtomData, ptr->parent()->atom(arrayIndex-1));
			break;
		case (PatternVariable::Bonds):
			rv.set(VTypes::PatternBoundData, ptr->bond(arrayIndex-1));
			break;
		case (PatternVariable::Cog):
			rv.set(ptr->calculateCog(arrayIndex-1));
			break;
		case (PatternVariable::Com):
			rv.set(ptr->calculateCom(arrayIndex-1));
			break;
		case (PatternVariable::FirstAtom):
			rv.set(VTypes::AtomData, ptr->firstAtom());
			break;
		case (PatternVariable::FirstAtomId):
			rv.set(ptr->startAtom() + 1);
			break;
		case (PatternVariable::FField):
			rv.set(VTypes::ForcefieldData, ptr->forcefield());
			break;
		case (PatternVariable::LastAtom):
			rv.set(VTypes::AtomData, ptr->lastAtom());
			break;
		case (PatternVariable::LastAtomId):
			rv.set(ptr->endAtom() + 1);
			break;
		case (PatternVariable::Name):
			rv.set(ptr->name());
			break;
		case (PatternVariable::NAngles):
			rv.set(ptr->nAngles());
			break;
		case (PatternVariable::NAtoms):
			rv.set(ptr->totalAtoms());
			break;
		case (PatternVariable::NBonds):
			rv.set(ptr->nBonds());
			break;
		case (PatternVariable::NMolAtoms):
			rv.set(ptr->nAtoms());
			break;
		case (PatternVariable::NMols):
			rv.set(ptr->nMolecules());
			break;
		case (PatternVariable::NTorsions):
			rv.set(ptr->nTorsions());
			break;
		case (PatternVariable::Torsions):
			rv.set(VTypes::PatternBoundData, ptr->torsion(arrayIndex-1));
			break;
		default:
			printf("Internal Error: Access to member '%s' has not been defined in PatternVariable.\n", accessorData[i].name);
			result = FALSE;
			break;
	}
	msg.exit("PatternVariable::retrieveAccessor");
	return result;
}

// Set desired value
bool PatternVariable::setAccessor(int i, ReturnValue &sourcerv, ReturnValue &newvalue, bool hasArrayIndex, int arrayIndex)
{
	msg.enter("PatternVariable::setAccessor");
	// Cast 'i' into Accessors enum value
	if ((i < 0) || (i >= nAccessors))
	{
		printf("Internal Error: Accessor id %i is out of range for Pattern type.\n");
		msg.exit("PatternVariable::setAccessor");
		return FALSE;
	}
	Accessors acc = (Accessors) i;
	// Check for correct lack/presence of array index given
	if (accessorData[i].arraySize == 0)
	{
		if (hasArrayIndex) msg.print("Warning: Irrelevant array index provided for member '%s'.\n", accessorData[i].name);
	}
	else if (!hasArrayIndex)
	{
		msg.print("Error: No array index provided for member '%s'.\n", accessorData[i].name);
		msg.exit("PatternVariable::setAccessor");
		return FALSE;
	}
	// Get current data from ReturnValue
	bool result = TRUE;
	Pattern *ptr= (Pattern*) sourcerv.asPointer(VTypes::PatternData, result);
	// Set value based on enumerated id
	if (result) switch (acc)
	{
		case (PatternVariable::Name):
			ptr->setName(newvalue.asString());
			break;
		case (PatternVariable::FField):
 			ptr->setForcefield( (Forcefield*) newvalue.asPointer(VTypes::ForcefieldData));
			break;
		default:
			printf("PatternVariable::setAccessor doesn't know how to use member '%s'.\n", accessorData[acc].name);
			result = FALSE;
			break;
	}
	msg.exit("PatternVariable::setAccessor");
	return result;
}
