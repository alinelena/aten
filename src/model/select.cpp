/*
	*** Model select functions
	*** src/model/select.cpp
	Copyright T. Youngs 2007-2016

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

#include "model/model.h"
#include "undo/atom_select.h"
#include "undo/undostate.h"
#include "base/neta_parser.h"
#include "base/pattern.h"

ATEN_USING_NAMESPACE

// Select atoms based on ranges, atom pointers etc.
bool Model::selectAtoms(TreeNode* node, bool deselect, bool testQuietly)
{
	QString from, to;
	int i, j, n, plus = 0;
	bool range;

	// Execute argument to get result
	ReturnValue value;
	if (!node->execute(value)) return false;

	// If the argument is an atom or integer variable, (de)select the corresponding atom. Otherwise, perform ranged selections
	if ((value.type() == VTypes::AtomData) || (value.type() == VTypes::IntegerData))
	{
		if (!testQuietly)
		{
			Atom* ii = value.type() == VTypes::IntegerData ? atom(value.asInteger()-1) : (Atom*) value.asPointer(VTypes::AtomData);
			beginUndoState("%select (%i)", deselect ? "Des" : "S", ii->id()+1);
			deselect ? deselectAtom(ii) : selectAtom(ii);
			endUndoState();
		}
	}
	else if (value.type() == VTypes::ElementData)
	{
		Element* elem = (Element*) value.asPointer(VTypes::ElementData);
		if (elem == NULL) return false;
		if (!testQuietly)
		{
			beginUndoState("%select element (%s)", deselect ? "Des" : "S", elem->symbol);
			deselect ? deselectElement(elem->z) : selectElement(elem->z);
			endUndoState();
		}
	}
	else if (value.type() == VTypes::PatternData)
	{
		if (!testQuietly)
		{
			Pattern* pp = (Pattern*) value.asPointer(VTypes::PatternData);
			beginUndoState("%select pattern '%s' (%i atoms)", deselect ? "Des" : "S", qPrintable(pp->name()), pp->totalAtoms());
			selectPattern(pp, false, deselect);
			endUndoState();
		}
	}
	else if (value.type() == VTypes::StringData)
	{
		// Use a parser to split up the line, in case there are multiple selections separated by commas
		LineParser parser;
		parser.getArgsDelim(0, value.asString());
		for (int arg=0; arg<parser.nArgs(); ++arg)
		{
			// If arg contains a '-', select by range
			if (parser.argc(arg).contains('-'))
			{
				range = true;
				QStringList items = parser.argc(arg).split('-');
				from = items.at(0);
				to = items.at(1);

				// Arguments for ranges cannot have '+' in them
				if ((from.contains('+')) || (to.contains('+')))
				{
					if (!testQuietly) Messenger::print("Range symbol (+) cannot be given in static range X-Y (input was '%s-%s').", qPrintable(from), qPrintable(to));
					return false;
				}

				// Check for empty parts of ranges
				if (from.isEmpty() || to.isEmpty()) 
				{
					if (!testQuietly) Messenger::print("One or both limits in range X-Y are empty (input was '%s-%s').", qPrintable(from), qPrintable(to));
					return false;
				}
			}
			else
			{
				range = false;
				from = parser.argc(arg);
				if (!from.contains('+')) plus = 0;
				else if (from.at(0) == '+') plus = -1;
				else if (from.at(from.length()-1) == '+') plus = 1;
				else
				{
					if (!testQuietly) Messenger::print("Invalid range symbol (+) given in middle of selection element '%s'.", qPrintable(from));
					return false;
				}
			}

			// Do the selection
			if (!testQuietly) beginUndoState("%select (%s)", deselect ? "Des" : "S", qPrintable(parser.argc(arg)));
			if (!range)
			{
				if (VTypes::determineType(from) == VTypes::IntegerData)
				{
					// Integer atom ID selection
					i = from.toInt();
					if (!testQuietly)
					{
						if (plus == 0) (deselect ? deselectAtom(i-1) : selectAtom(i-1));
						else if (plus == -1) for (n=0; n < i; n++) (deselect ? deselectAtom(n) : selectAtom(n));
						else if (plus == 1) for (n=i-1; n < nAtoms(); n++) (deselect ? deselectAtom(n) : selectAtom(n));
					}
				}
				else
				{
					i = ElementMap::find(from, ElementMap::AlphaZMap);
					if (i == 0)
					{
						if (!testQuietly) Messenger::print("Unrecognised element (%s) in select.", qPrintable(from));
						return false;
					}

					if (!testQuietly)
					{
						if (plus == 0) (deselect ? deselectElement(i) : selectElement(i));
						else if (plus == -1) for (n=1; n <= i; n++) (deselect ? deselectElement(n) : selectElement(n));
						else if (plus == 1) for (n=i; n <= ElementMap::nElements(); n++) (deselect ? deselectElement(n) : selectElement(n));
					}
				}
			}
			else
			{
				// Range of id's or elements
				if (VTypes::determineType(from) == VTypes::IntegerData)
				{
					i = from.toInt();
					j = to.toInt();
					if (!testQuietly) for (n=i-1; n<j; n++) (deselect ? deselectAtom(n) : selectAtom(n));
				}
				else
				{
					i = ElementMap::find(from, ElementMap::AlphaZMap);
					if (i == 0)
					{
						if (!testQuietly) Messenger::print("Unrecognised element (%s) on left-hand side of range.", qPrintable(from));
						return false;
					}
					j = ElementMap::find(to, ElementMap::AlphaZMap);
					if (j == 0)
					{
						if (!testQuietly) Messenger::print("Unrecognised element (%s) on right-hand side of range.", qPrintable(to));
						return false;
					}
					if (!testQuietly) for (n=i; n <= j; n++) (deselect ? deselectElement(n) : selectElement(n));
				}
			}
			if (!testQuietly) endUndoState();
		}
	}
	else
	{
		if (!testQuietly) Messenger::print("Cannot (de)select atoms based on supplied %s.", VTypes::dataType(value.type()));
		return false;
	}
	return true;
}

// Return the number of selected atoms
int Model::nSelected() const
{
	return selection_.nItems();
}

// Return the number of marked atoms
int Model::nMarked() const
{
	return marked_.nItems();
}

// Return list of selected (marked) atoms
const RefList<Atom,int> Model::selectedAtoms(bool markOnly) const
{
	return (markOnly ? marked_ : selection_);
}

// Mark all atoms in model
void Model::markAll()
{
	selectAll(true);
	Messenger::print(Messenger::Verbose, "All atoms marked.");
}

// Match marked atoms to current selection
void Model::markSelectedAtoms()
{
	selectNone(true);
	for (RefListItem<Atom,int>* ri = selection_.first(); ri != NULL; ri = ri->next) selectAtom(ri->item, true);
	Messenger::print(Messenger::Verbose, "There are now %i atoms marked.", marked_.nItems());
}

// Select marked atoms
void Model::selectMarkedAtoms()
{
	for (RefListItem<Atom,int>* ri = marked_.first(); ri != NULL; ri = ri->next) selectAtom(ri->item);
}

// Put selected atom in specified array
bool Model::selectedAtoms(int nAtoms, Atom** array)
{
	int n = 0;
	for (RefListItem<Atom,int>* ri = selection_.first(); ri != NULL; ri = ri->next)
	{
		array[n] = ri->item;
		++n;
		if (n == nAtoms) break;
	}

	// Did we get the requested number of Atom pointers?
	if (n != nAtoms)
	{
		printf("Model::selectedAtoms() : Failed to get requested number of atoms (%i)\n", nAtoms);
		return false;
	}

	return true;
}

// Select Atom
void Model::selectAtom(Atom* i, bool markonly)
{
	if (markonly)
	{
		if (!i->isSelected(true))
		{
			i->setSelected(true, true);
			// Add at correct position in list
			RefListItem<Atom,int>* ri = marked_.first();
			if (ri == NULL) marked_.add(i);
			else if (ri->item->id() > i->id()) marked_.addStart(i);
			else
			{
				for (ri = marked_.last(); ri != NULL; ri = ri->prev) if (ri->item->id() < i->id()) break;
				marked_.addAfter(ri,i);
			}
		}
	}
	else
	{
		if (!i->isSelected())
		{
			i->setSelected(true);
			selection_.add(i);
			logChange(Log::Selection);
			// Add the change to the undo state (if there is one)
			if (recordingState_ != NULL)
			{
				AtomSelectEvent* newchange = new AtomSelectEvent;
				newchange->set(true, i->id());
				recordingState_->addEvent(newchange);
			}
		}
	}
}

// Select Atom by ID
void Model::selectAtom(int id, bool markonly)
{
	Atom* i = atom(id);
	if (i == NULL) Messenger::print("Can't %s atom %i", markonly ? "mark" : "select", id+1);
	else selectAtom(i, markonly);
}

// Deselect Atom
void Model::deselectAtom(Atom* i, bool markonly)
{
	if (markonly)
	{
		if (i->isSelected(true))
		{
			i->setSelected(false, true);
			marked_.remove(i);
		}
	}
	else
	{
		if (i->isSelected())
		{
			i->setSelected(false);
			selection_.remove(i);
			logChange(Log::Selection);
			// Add the change to the undo state (if there is one)
			if (recordingState_ != NULL)
			{
				AtomSelectEvent* newchange = new AtomSelectEvent;
				newchange->set(false, i->id());
				recordingState_->addEvent(newchange);
			}
		}
	}
}

// Deselect Atom by ID
void Model::deselectAtom(int id, bool markonly)
{
	Atom* i = atom(id);
	if (i == NULL) Messenger::print("Can't %s atom %i", markonly ? "unmark" : "deselect", id+1);
	else deselectAtom(i, markonly);
}

// Toggle Selection State
void Model::selectionToggle(Atom* i, bool markonly)
{
	i->isSelected(markonly) ? deselectAtom(i, markonly) : selectAtom(i, markonly);
}

// Invert Current Selection
void Model::selectionInvert(bool markonly)
{
	Messenger::enter("Model::selectionInvert");
	for (Atom* i = atoms_.first(); i != NULL; i = i->next)
		i->isSelected(markonly) ? deselectAtom(i, markonly) : selectAtom(i, markonly);
	Messenger::exit("Model::selectionInvert");
}

// Delete Selected Atoms
void Model::selectionDelete(bool markonly)
{
	Messenger::enter("Model::selectionDelete");
	Atom* i, *tempi;
	int count = 0;
	bool cancelled = false;
	// Attempt to be clever here for the sake of undo/redo, while avoiding renumbering at every step.
	// 1) First, delete all measurements and bonds to the selected atoms
	RefListItem<Bond,int>* bref;
	for (RefListItem<Atom,int>* ri = selection(markonly); ri != NULL; ri = ri->next)
	{
		i = ri->item;
		// Remove measurements
		removeMeasurements(i);
		// Delete All Bonds To Specific Atom
		bref = i->bonds();
		while (bref != NULL)
		{
			// Need to detach the bond from both atoms involved
			Bond* b = bref->item;
			Atom* j = b->partner(i);
			unbondAtoms(i,j,b);
			bref = i->bonds();
		}
		if (cancelled) break;
	}
	// 2) Delete the actual atoms
	if (!cancelled)
	{
		i = atoms_.last();
		while (i != NULL)
		{
			if (i->isSelected(markonly))
			{
				tempi = i->prev;
				removeAtom(i, true);
				i = tempi;
			}
			else i = i->prev;
		}
	}
	// Renumber remaining atoms
	renumberAtoms();
	Messenger::exit("Model::selectionDelete");
}

// Expand Current Selection
void Model::selectionExpand(bool markonly)
{
	Messenger::enter("Model::selectionExpand");
	Atom* i;
	RefListItem<Bond,int>* bref;
	// Store the current selection state in i->tempBit_
	for (i = atoms_.first(); i != NULL; i = i->next) i->setBit(i->isSelected(markonly));
	// Now use the temporary state to find atoms where we select atomic neighbours
	for (i = atoms_.first(); i != NULL; i = i->next)
		if (i->bit()) for (bref = i->bonds(); bref != NULL; bref = bref->next) selectAtom(bref->item->partner(i), markonly);
	Messenger::exit("Model::selectionExpand");
}

// Select All Atoms
void Model::selectAll(bool markonly)
{
	Messenger::enter("Model::selectAll");
	if (markonly)
	{
		// Quicker to reconstruct the whole list, since it must be in ID order
		marked_.clear();
		for (Atom* i = atoms_.first(); i != NULL; i = i->next)
		{
			i->setSelected(true, true);
			marked_.add(i);
		}
	}
	else
	{
		// Here, just add atoms which are not currently selected (i.e. we assume the atom selection flags and selection_ list reflect each other)
		int nChanges = 0;
		for (Atom* i = atoms_.first(); i != NULL; i = i->next)
		{
			if (!i->isSelected())
			{
				i->setSelected(true);

				// Add the change to the undo state (if there is one)
				if (recordingState_ != NULL)
				{
					AtomSelectEvent* newchange = new AtomSelectEvent;
					newchange->set(true, i->id());
					recordingState_->addEvent(newchange);
				}

				selection_.add(i);
				++nChanges;
			}
		}
		if (nChanges) logChange(Log::Selection);
	}
	Messenger::exit("Model::selectAll");
}

// Deselect All Atoms
void Model::selectNone(bool markonly)
{
	Messenger::enter("Model::selectNone");
	if (markonly)
	{
		for (RefListItem<Atom,int>* ri = marked_.first(); ri != NULL; ri = ri->next) ri->item->setSelected(false, true);
		marked_.clear();
	}
	else
	{
		int nChanges = 0;
		for (Atom* i = atoms_.first(); i != NULL; i = i->next)
		{
			if (i->isSelected())
			{
				i->setSelected(false);

				// Add the change to the undo state (if there is one)
				if (recordingState_ != NULL)
				{
					AtomSelectEvent* newchange = new AtomSelectEvent;
					newchange->set(false, i->id());
					recordingState_->addEvent(newchange);
				}

				++nChanges;
			}
		}
		if (nChanges) logChange(Log::Selection);
		selection_.clear();
	}
	Messenger::exit("Model::selectNone");
}

// Atom at Screen Coordinates
Atom* Model::atomOnScreen(double x1, double y1)
{
	// See if an atom exists under the canvas coordinates x1,y1
	Messenger::enter("Model::atomOnScreen");
	Atom* closest = NULL;
	Vec3<double> wr;
	Vec4<double> sr;
	double closestz = 10000.0, dist, nclip = prefs.clipNear();

	for (Atom* i = atoms_.first(); i != NULL; i = i->next)
	{
		if (i->isHidden()) continue;

		// Get draw style for atom - if stick, 
		wr = -modelToWorld(i->r(), &sr, styleRadius(i->style(), i->element()));
		if (wr.z > nclip)
		{
			dist = sqrt((sr.x - x1)*(sr.x - x1) + (sr.y - y1)*(sr.y - y1));
			if (dist < sr.w)	// Mouse is inside bounding circle
			{
				if ((closest == NULL) || (wr.z < closestz))
				{
					closest = i;
					closestz = wr.z;
				}
			}
		}
	}
	Messenger::exit("Model::atomOnScreen");
	return closest;
}

// Select atoms within bounding box
void Model::selectBox(double x1, double y1, double x2, double y2, bool deselect)
{
	// Box selection - choose all the atoms within the selection area
	Messenger::enter("Model::selectBox");
	double t;
	Atom* i;
	Vec4<double> sr;
	// Handle 'reverse ranges' - make sure x1 < x2 and y1 < y2
	if (x1 > x2)
	{
		t=x1;
		x1=x2;
		x2=t;
	}
	if (y1 > y2)
	{
		t=y1;
		y1=y2;
		y2=t;
	}
	for (i = atoms_.first(); i != NULL; i = i->next)
	{
		if (i->isHidden()) continue;
		modelToWorld(i->r(), &sr);
		if ((sr.x >= x1) && (sr.x <= x2) && (sr.y >= y1) && (sr.y <= y2)) (deselect ? deselectAtom(i) : selectAtom(i));
	}
	Messenger::exit("Model::selectBox");
}

// Tree Select
void Model::selectTree(Atom* i, bool markonly, bool deselect, Bond* omitbond)
{
	// The passed atom node is the starting point for the algorithm.
	// From here, select all atoms that are bound - if they are already
	// selected then ignore them. If they are not already selected, then
	// recursively call the routine on that atom.
	//Messenger::enter("Model::selectTree");
	bool status;
	Atom* j;
	deselect ? deselectAtom(i, markonly) : selectAtom(i, markonly);
	for (RefListItem<Bond,int>* bref = i->bonds(); bref != NULL; bref = bref->next)
	{
		if (bref->item == omitbond) continue;
		j = bref->item->partner(i);
		status = j->isSelected(markonly);
		if (deselect) status = !status;
		if (!status)
		{
			if (deselect) deselectAtom(j, markonly);
			else selectAtom(j, markonly);
			selectTree(j, markonly, deselect, omitbond);
		}
	}
	//Messenger::exit("Model::selectTree");
}

// Select by Element
void Model::selectElement(Atom* target, bool markonly, bool deselect)
{
	// Select all atoms which are the same element as the atom i
	selectElement(target->element(), markonly, deselect);
}

// Select by element (from element)
void Model::selectElement(int el, bool markonly, bool deselect)
{
	// Select all atoms which are the same element as the atom with id 'target'
	Messenger::enter("Model::selectElement");
	for (Atom* i = atoms_.first(); i != NULL; i = i->next)
		if (i->element() == el) (deselect ? deselectAtom(i, markonly) : selectAtom(i, markonly));
	Messenger::exit("Model::selectElement");
}

// Deselect by Element
void Model::deselectElement(int el, bool markonly)
{
	// Select all atoms which are the same element as the atom i
	Messenger::enter("Model::deselectElement");
	for (Atom* i = atoms_.first(); i != NULL; i = i->next)
		if (i->element() == el) deselectAtom(i, markonly);
	Messenger::exit("Model::deselectElement");
}

// Select all atoms which match the provided type
int Model::selectType(int element, QString typeDesc, bool markonly, bool deselect)
{
	Messenger::enter("Model::selectType");
	Neta testat;
	testat.setCharacterElement(element);
	if (!NetaParser::createNeta(&testat, typeDesc, NULL))
	{
		Messenger::print("Failed to create type description.");
		Messenger::exit("Model::selectType");
		return -1;
	}
	int count = 0, matchscore = 0, atomscore, n;
	// Prepare for typing
	describeAtoms();
	// Loop over patterns and select atoms
	for (Pattern* p = patterns_.first(); p != NULL; p = p->next)
	{
		Atom* i = p->firstAtom();
		for (n=0; n<p->nAtoms(); n++)
		{
			atomscore = testat.matchAtom(i,p->ringList(),this);
			if (atomscore > 0)
			{
				// Select this atom in all pattern molecules
				p->selectAtom(n, markonly, deselect);
				count += p->nMolecules();
				matchscore = atomscore;
			}
			i = i->next;
		}
	}
	// Update model
	logChange(Log::Selection);
	// Write results
	Messenger::print("Type description score = %i. Matched %i atoms.", matchscore, count);
	Messenger::exit("Model::selectType");
	return count;
}

// Select with bounding Sphere
void Model::selectRadial(Atom* target, double radius)
{
	// Select all atoms which are within the distance 'radius' from atom 'target'
	Messenger::enter("Model::selectRadial");
	Atom* i = atoms_.first();
	printf("Selection radius is %8.4f Angstroms\n",radius);
	while (i != NULL)
	{
		if (i == target) selectAtom(i);
		else if (distance(target,i) < radius) selectAtom(i);
		i = i->next;
	}
	Messenger::exit("Model::selectRadial");
}

// Select all atoms in specified pattern
void Model::selectPattern(Pattern* p, bool markonly, bool deselect)
{
	// Select all atoms covered by the specified pattern.
	Messenger::enter("Model::selectPattern");
	// Check that this pattern is valid and belongs to this model...
	bool found = false;
	for (Pattern* modelp = patterns_.first(); modelp != NULL; modelp = modelp->next) if (p == modelp) found = true;
	if (!found)
	{
		Messenger::print("Pattern does not belong to this model, or is out of date.");
		Messenger::exit("Model::selectPattern");
		return;
	}
	Atom* i = p->firstAtom();
	for (int n=0; n<p->totalAtoms(); n++)
	{
		deselect ? deselectAtom(i, markonly) : selectAtom(i, markonly);
		i = i->next;
	}
	Messenger::exit("Model::selectPattern");
}

// Get first selected
RefListItem<Atom,int>* Model::selection(bool markonly) const
{
	if (markonly) return marked_.first();
	else return selection_.first();
}

// Return the nth selected atom in the model
RefListItem<Atom,int>* Model::selected(int n)
{
	if ((n<0) || (n>=selection_.nItems())) printf("Array index for selection_ is out of bounds : %i.\n", n);
	else return selection_[n];
	return NULL;
}

// Move atoms 'up'
void Model::shiftSelectionUp(bool markOnly)
{
	Messenger::enter("Model::shiftSelectionUp");

	if (markOnly) for (RefListItem<Atom,int>* ri = marked_.last(); ri != NULL; ri = ri->prev) shiftAtomUp(ri->item);
	else for (RefListItem<Atom,int>* ri = selection_.last(); ri != NULL; ri = ri->prev) shiftAtomUp(ri->item);
	logChange(Log::Structure);
	Messenger::exit("Model::shiftSelectionUp");
}

// Move atoms 'down'
void Model::shiftSelectionDown(bool markOnly)
{
	Messenger::enter("Model::shiftSelectionDown");
	if (markOnly) for (RefListItem<Atom,int>* ri = marked_.first(); ri != NULL; ri = ri->next) shiftAtomDown(ri->item);
	else for (RefListItem<Atom,int>* ri = selection_.first(); ri != NULL; ri = ri->next) shiftAtomDown(ri->item);
	logChange(Log::Structure);
	Messenger::exit("Model::shiftSelectionDown");
}

// Move atoms to start
void Model::moveSelectionToStart(bool markOnly)
{
	Messenger::enter("Model::moveSelectionToStart");
	if (markOnly) for (RefListItem<Atom,int>* ri = marked_.last(); ri != NULL; ri = ri->prev) moveAtomAfter(ri->item, NULL);
	else for (RefListItem<Atom,int>* ri = selection_.last(); ri != NULL; ri = ri->prev) moveAtomAfter(ri->item, NULL);
	logChange(Log::Structure);
	Messenger::exit("Model::moveSelectionToStart");
}

// Move atoms to end
void Model::moveSelectionToEnd(bool markOnly)
{
	Messenger::enter("Model::moveSelectionToEnd");
	if (markOnly) for (RefListItem<Atom,int>* ri = marked_.first(); ri != NULL; ri = ri->next) moveAtomAfter(ri->item, atoms_.last());
	else for (RefListItem<Atom,int>* ri = selection_.first(); ri != NULL; ri = ri->next) moveAtomAfter(ri->item, atoms_.last());
	logChange(Log::Structure);
	Messenger::exit("Model::moveSelectionToEnd");
}

// Select overlapping atoms
void Model::selectOverlaps(double tolerance, bool markonly)
{
	Messenger::enter("Model::selectOverlaps");
	int n, m, x, y, z, x2, y2, z2, checklist[8], count;
	double dist;
	Atom* i, *j;
	selectNone(markonly);
	// Create cuboid lists
	initialiseBondingCuboids();
	// Add all atoms to cuboid list
	for (i = atoms_.first(); i != NULL; i = i->next) addAtomToCuboid(i);
	// Loop over cuboids, checking distances with atoms in adjacent boxes
	RefListItem<Atom,double>* ri, *rj;
	x = 0;
	y = 0;
	z = 0;
	count = 0;
	for (n = 0; n<nCuboids_; n++)
	{
		if (bondingCuboids_[n].nItems() != 0)
		{
// 			if (bondingCuboids_[n].nItems() > 0) printf("On cuboid %i (%ix%ix%i) which contains %i atoms\n", n, x, y, z, bondingCuboids_[n].nItems());
			// For each of the atoms in the cuboid, check distance with each atom in eight of the closest
			// overlay boxes. 
			checklist[0] = n;
			x2 = (x == (cuboidBoxes_.x-1) ? 0 : x+1);
			y2 = (y == (cuboidBoxes_.y-1) ? 0 : y+1);
			z2 = (z == (cuboidBoxes_.z-1) ? 0 : z+1);
// 			if (bondingCuboids_[n].nItems() > 0) printf("....xyz = %i,%i,%i, xyz2 = %i,%i,%i\n", x, y, z, x2, y2, z2);
			checklist[1] = x2*cuboidYZ_+y*cuboidBoxes_.z+z;
			checklist[2] = x2*cuboidYZ_+y2*cuboidBoxes_.z+z;
			checklist[3] = x2*cuboidYZ_+y*cuboidBoxes_.z+z2;
			checklist[4] = x2*cuboidYZ_+y2*cuboidBoxes_.z+z2;
			checklist[5] = x*cuboidYZ_+y2*cuboidBoxes_.z+z;
			checklist[6] = x*cuboidYZ_+y*cuboidBoxes_.z+z2;
			checklist[7] = x*cuboidYZ_+y2*cuboidBoxes_.z+z2;
			for (ri = bondingCuboids_[n].first(); ri != NULL; ri = ri->next)
			{
				i = ri->item;
				if (i->isSelected(markonly)) continue;
				for (m=0; m<8; m++)
				{
					for (rj = bondingOverlays_[checklist[m]].first(); rj != NULL; rj = rj->next)
					{
						j = rj->item;
						if ((j->isSelected(markonly)) || (i == j)) continue;
						dist = cell_.distance(i,j);
						if (dist < tolerance)
						{
							Messenger::print(Messenger::Verbose, "Atom %i (%s) is %f from atom %i (%s).", j->id()+1, ElementMap::symbol(j), dist, i->id()+1, ElementMap::symbol(i));
							selectAtom(j, markonly);
							++count;
						}
					}
				}
			}
		}
		// Increase x,y,z indices for box lookup
		z ++;
		if (z == cuboidBoxes_.z)
		{
			z = 0;
			y ++;
			if (y == cuboidBoxes_.y)
			{
				y = 0;
				x ++;
			}
		}
	}
	// Free bonding cuboids
	freeBondingCuboids();
	Messenger::print("%i overlapping atoms selected.", count);
	Messenger::exit("Model::selectOverlaps");
}


// Select atoms (or molecule COGs) inside of the current unit cell
void Model::selectInsideCell(bool moleculecogs, bool markonly)
{
	Messenger::enter("Model::selectInsideCell");
	Vec3<double> pos;
	// If using molecule COGs, need a valid pattern definition
	if (moleculecogs)
	{
		createPatterns();
		int m,n,id = 0;
		for (Pattern* p = patterns_.first(); p != NULL; p = p->next)
		{
			for (m=0; m<p->nMolecules(); ++m)
			{
				// Get COG of molecule
				pos = cell_.realToFrac(p->calculateCog(m));
				if ((pos.x >= 0) && (pos.x <= 1) && (pos.y >= 0) && (pos.y <= 1) && (pos.z >= 0) && (pos.z <= 1)) for (n=0; n<p->nAtoms(); ++n) selectAtom(id+n, markonly);
				id += p->nAtoms();
			}
		}
	}
	else
	{
		for (Atom* i = atoms_.first(); i != NULL; i = i->next)
		{
			pos = cell_.realToFrac(i->r());
			if ((pos.x >= 0) && (pos.x <= 1) && (pos.y >= 0) && (pos.y <= 1) && (pos.z >= 0) && (pos.z <= 1)) selectAtom(i, markonly);
		}
	}
	Messenger::exit("Model::selectInsideCell");
}

// Select atoms (or molecule COGs) outside of the current unit cell
void Model::selectOutsideCell(bool moleculecogs, bool markonly)
{
	Messenger::enter("Model::selectOutsideCell");
	Vec3<double> pos;
	// If using molecule COGs, need a valid pattern definition
	if (moleculecogs)
	{
		createPatterns();
		int m,n,id = 0;
		for (Pattern* p = patterns_.first(); p != NULL; p = p->next)
		{
			for (m=0; m<p->nMolecules(); ++m)
			{
				// Get COG of molecule
				pos = cell_.realToFrac(p->calculateCog(m));
				if ((pos.x > 1) || (pos.y > 1) || (pos.z > 1)) for (n=0; n<p->nAtoms(); ++n) selectAtom(id+n, markonly);
				id += p->nAtoms();
			}
		}
	}
	else
	{
		for (Atom* i = atoms_.first(); i != NULL; i = i->next)
		{
			pos = cell_.realToFrac(i->r());
			if ((pos.x > 1) || (pos.y > 1) || (pos.z > 1)) selectAtom(i, markonly);
		}
	}
	Messenger::exit("Model::selectOutsideCell");
}

// Perform a Miller 'selection' on the cell contents
void Model::selectMiller(int h, int k, int l, bool inside, bool markOnly)
{
	Messenger::enter("Model::selectMiller");

	if (cell_.type() == UnitCell::NoCell)
	{
		Messenger::print("Can't use Miller planes on a non-periodic model.");
		Messenger::exit("Model::selectMiller");
		return;
	}

	// Determine the Miller planes for the hkl provided
	Plane firstPlane, secondPlane;
	cell_.millerPlanes(h, k, l, firstPlane, secondPlane);

	// The normals for the defined planes always point away from the cell centre.
	// Grab the centroids of the defining Miller planes, and use the dp of the atom position (relative to the centres)
	// with the plane normals to decide where the atom is
	Vec3<double> centroid1 = firstPlane.centroid(), centroid2 = secondPlane.centroid();
	Vec3<double> normal1 = firstPlane.normal(), normal2 = secondPlane.normal();
	for (Atom* i = atoms_.first(); i != NULL; i = i->next)
	{
		// Test dot product of atom position (minus centre) with normals - if either is positive, select it
		if (((i->r()-centroid1).dp(normal1) > 0.0) || ((i->r()-centroid2).dp(normal2) > 0.0)) selectAtom(i, markOnly);
	}

	Messenger::exit("Model::selectMiller");
}

// Select atoms within distance from a line (i.e. cylinder select)
void Model::selectLine(Vec3<double> line, Vec3<double> point, double dr, bool markonly)
{
	Messenger::enter("Model::selectLine");
	// See: A Programmers Geometry, Bowyer and Woodwark, Butterworths (pub.), 1983, p99
	// Line equation is :
	//		x = point.x + line.x * t
	//		y = point.y + line.y * t
	//		z = point.z + line.z * t
	// Check line parameters
	double denom = line.dp(line);
	if (denom < 1.0e-6)
	{
		Messenger::print("Line parameters appear to be corrupt.");
		Messenger::exit("Model::selectLine");
		return;
	}
	line.normalise();
	Vec3<double> r, v, dummy, origin;
	double xyyx, xzzx, yzzy, dist;
	for (int pass = 0; pass < 4; ++pass)
	{
		origin = point;
		if (pass > 0) origin += cell_.axes().columnAsVec3(pass-1);
		for (Atom* i = atoms_.first(); i != NULL; i = i->next)
		{
			if (i->isSelected()) continue;
			r = i->r() - origin;
			xyyx = line.x*r.y - line.y*r.x;
			xzzx = line.x*r.z - line.z*r.x;
			yzzy = line.y*r.z - line.z*r.y;
			v.x = line.y*xyyx + line.z*xzzx;
			v.y = line.z*yzzy - line.x*xyyx;
			v.z = -line.x*xzzx - line.y*yzzy;
			v = cell_.mimVector(v, dummy);
			dist = v.magnitude();
			if (dist < dr) selectAtom(i, markonly);
		}
	}

}
