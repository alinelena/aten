/*
	*** Energy expression 
	*** src/model/expression.cpp
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

#include "base/pattern.h"
#include "model/model.h"
#include "ff/forcefield.h"
#include "base/forcefieldatom.h"

ATEN_USING_NAMESPACE

// Return the forcefield used by the model
Forcefield* Model::forcefield()
{
	return forcefield_;
}

// Create a forcefield containing original atom names for the model
void Model::createNamesForcefield()
{
	if (namesForcefield_ != NULL) Messenger::print("Warning - an atom names forcefield already exists for model '%s'.", qPrintable(name_));
	Messenger::print("Creating atom names forcefield for model '%s'.", qPrintable(name_));
	namesForcefield_ = new Forcefield;
	namesForcefield_->setName(QString("Atom names for model ")+name_);
}

// Return the forcefield containing original atom names for the model
Forcefield* Model::namesForcefield() const
{
	return namesForcefield_;
}

// Add name to names forcefield
ForcefieldAtom* Model::addAtomName(int el, QString name)
{
	if (namesForcefield_ == NULL) createNamesForcefield();

	// Search for this typename in the ff
	ForcefieldAtom* ffa = namesForcefield_->findType(name, el);
	if (ffa == NULL)
	{
		ffa = namesForcefield_->addType();
		ffa->setName(name);
		ffa->setElement(el);
		ffa->neta()->setCharacterElement(el);
	}
	return ffa;
}

// Remove the names forcefield reference
void Model::removeNamesForcefield()
{
	namesForcefield_ = NULL;
}

// Return whether the expression is valid
bool Model::isExpressionValid() const
{
	return (expressionPoint_ == log(Log::Structure));
}

// Clear the current expression
void Model::clearExpression()
{
	Messenger::enter("Model::clearExpression");
	forcefieldAngles_.clear();
	forcefieldBonds_.clear();
	forcefieldTorsions_.clear();
	uniqueForcefieldTypes_.clear();
	for (Pattern* p = patterns_.first(); p != NULL; p = p->next) p->deleteExpression();
	expressionPoint_  = -1;
	Messenger::exit("Model::clearExpression");
}

// Manually invalidates the expression
void Model::invalidateExpression()
{
	expressionPoint_  = -1;
}

// Assign charges from forcefield
bool Model::assignForcefieldCharges()
{
	// Assign atom-type charges from the currently assigned atom types
	// Perform forcefield typing if necessary
	Messenger::enter("Model::assignForcefieldCharges");
	int nfailed = 0;
	double totalq = 0.0;
	for (Atom* i = atoms_.first(); i != NULL; i = i->next)
	{
		if (i->type() == NULL)
		{
			nfailed ++;
			Messenger::print("Could not assign charge to atom %i since it has no atomtype associated to it.", i->id()+1);
		}
		else
		{
			i->setCharge( i->type()->charge() );
			totalq += i->type()->charge();
		}
	}
	if (nfailed == 0) Messenger::print("Charges assigned successfully to all atoms.\nTotal charge in model is %f e.", totalq);
	else Messenger::print("Failed to assign charges to %i atoms.", nfailed);
	Messenger::exit("Model::assignForcefieldCharges");
	return (nfailed == 0);
}

// Clear charges
void Model::clearCharges()
{
	Messenger::enter("Model::clearCharges");
	for (Atom* i = atoms_.first(); i != NULL; i = i->next) atomSetCharge(i, 0.0);
	Messenger::exit("Model::clearCharges");
}

// Set model's forcefield
void Model::setForcefield(Forcefield* newff)
{
	// Change the associated forcefield of the model to 'newff'
	if (forcefield_ != newff)
	{
		invalidateExpression();
		forcefield_ = newff;
		if (forcefield_ == NULL) Messenger::print("Model '%s' has had its associated forcefield removed.", qPrintable(name_));
		else Messenger::print("Forcefield '%s' now associated with model '%s'.", qPrintable(forcefield_->name()), qPrintable(name_));
	}
}

// Create full forcefield expression for model
bool Model::createExpression(Choice vdwOnly, Choice allowDummy, Choice assignCharges, Forcefield* defaultForcefield)
{
	// This routine should be called before any operation (or series of operations) requiring calculation of energy / forces. Here, we check the validity / existence of an energy expression for the specified model, and create / recreate if necessary.
	Messenger::enter("Model::createExpression");
	
	// Resolve supplied choices
	vdwOnly.resolve(false);
	allowDummy.resolve(false);
	assignCharges.resolve(true);
	
	// 0) If the expression is already valid, just update scaling terms in pattern matrices and return
	if (isExpressionValid() && (vdwOnly == expressionVdwOnly_))
	{
		for (Pattern* p = patterns_.first(); p != NULL; p = p->next) p->updateScaleMatrices();
		Messenger::exit("Model::createExpression");
		return true;
	}

	// Reset some variables
	prefs.invalidateEwaldAuto();
	forcefieldAngles_.clear();
	forcefieldBonds_.clear();
	forcefieldTorsions_.clear();
	uniqueForcefieldTypes_.clear();
	allForcefieldTypes_.clear();
	expressionVdwOnly_ = vdwOnly;
	expressionPoint_ = -1;
	if (expressionVdwOnly_) Messenger::print("Creating VDW-only expression for model %s...",qPrintable(name_));
	else Messenger::print("Creating expression for model %s...",qPrintable(name_));
	
	// 1) Assign forcefield types to all atoms
	if (!typeAll(defaultForcefield))
	{
		Messenger::print("Couldn't type atoms.");
		Messenger::exit("Model::createExpression");
		return false;
	}
	
	// 2) Remove old expression data and create new
	bool done;
	for (Pattern* p = patterns_.first(); p != NULL; p = p->next)
	{
		done = false;
		while (!done)
		{
			p->deleteExpression();
			if (!p->createExpression(vdwOnly, allowDummy))
			{
				// Failed to create an expression for this pattern, so....
				// What to do?
				Tree dialog;
				QString title;
				title.sprintf("Expression for Pattern '%s'", qPrintable(p->name()));
				TreeGui& ui = dialog.defaultDialog();
				ui.setProperty(TreeGuiWidgetEvent::TextProperty, title);
				ui.addLabel("", "One or more forcefield terms are not availablefor this pattern:", 1, 1);
				ui.addRadioGroup("choice");
				ui.addRadioButton("cancel", "Cancel expression generation", "choice", 1, 1, 2);
				ui.addRadioButton("dummy", "Add dummy parameters to complete expression for this pattern", "choice", 0, 1, 3);
				ui.addRadioButton("alldummy", "Add dummy parameters to complete expression for this pattern and any others which need it", "choice", 0, 1, 4);
				
				// Run the custom dialog
				if (dialog.defaultDialog().execute())
				{
					int choice = ui.asInteger("choice");
					if (choice == 1)
					{
						Messenger::exit("Model::createExpression");
						return false;
					}
					else if (choice == 2)
					{
						// Flag generation of dummy terms in expression, and let the loop cycle
						p->setAddDummyTerms(true);
						done = false;
					}
					else if (choice == 3)
					{
						// Flag generation of dummy terms in expression for all patterns, and let the loop cycle
						allowDummy = Choice::Yes;
						done = false;
					}
				}
				else
				{
					Messenger::exit("Model::createExpression");
					return false;
				}
			}
			else done = true;
		}
		p->createMatrices();
	}

	// 3) Check the electrostatic setup for the model
	Electrostatics::ElecMethod emodel = prefs.electrostaticsMethod();
	switch (emodel)
	{
		case (Electrostatics::None):
			break;
		case (Electrostatics::Coulomb):
			if (cell_.type() != UnitCell::NoCell) Messenger::print("!!! Coulomb sum requested for periodic model.");
			break;
		default: // Ewald - issue warnings, but don't return false
			if (cell_.type() == UnitCell::NoCell)
			{
				Messenger::print("!!! Ewald sum cannot be used for a non-periodic model.");
				//Messenger::exit("Model::createExpression");
				//return false;
			}
			break;
	}
	
	// 4) Create master (Model) forcefield term lists
	createForcefieldLists();
	
	// 5) Assign charges to atoms (if requested)
	if (assignCharges && (!assignForcefieldCharges()))
	{
		Messenger::exit("Model::createExpression");
		return false;
	}
	
	// 6) Create VDW lookup table of combined parameters
	combinationTable_.clear();
	PointerPair<ForcefieldAtom,double>* pp;
	ForcefieldAtom* ffa, *ffb;
	CombinationRules::CombinationRule *crflags;
	int i;
	for (RefListItem<ForcefieldAtom,int>* rfa = allForcefieldTypes_.first(); rfa != NULL; rfa = rfa->next)
	{
		ffa = rfa->item;
		for (RefListItem<ForcefieldAtom,int>* rfb = rfa; rfb != NULL; rfb = rfb->next)
		{
			ffb = rfb->item;
			// Check for compatible interaction types between atoms
			if (ffa->vdwForm() != ffb->vdwForm())
			{
				Messenger::print("Conflicting van der Waals functional forms for atom types '%s' and '%s'.", VdwFunctions::functionData[ffa->vdwForm()].name, VdwFunctions::functionData[ffb->vdwForm()].name);
				Messenger::print("Expression for model cannot be completed.");
				Messenger::exit("Model::createExpression");
				return false;
			}

			// Create item in table
			pp = combinationTable_.add(ffa, ffb, VdwFunctions::functionData[ffa->vdwForm()].nParameters);

			// Combine parameters
			crflags = VdwFunctions::functionData[ffa->vdwForm()].combinationRules;
			for (i=0; i<VdwFunctions::functionData[ffa->vdwForm()].nParameters; ++i)
			{
				pp->setData(i, CombinationRules::combine( crflags[i], ffa->parameter(i), ffb->parameter(i) ) );
// 				printf("combined Parameter is %f, original = %f,%f\n", CombinationRules::combine( crflags[i], ffa->parameter(i), ffb->parameter(i)), ffa->parameter(i), ffb->parameter(i));
			}

		}
	}
	
	expressionPoint_ = log(Log::Structure);
	Messenger::exit("Model::createExpression");

	return true;
}

// Create lists of forcefield terms in the model
void Model::createForcefieldLists()
{
	Messenger::enter("Model::createForcefieldLists");

	Messenger::print(Messenger::Verbose, "Constructing global forcefield term lists for model...");
	forcefieldAngles_.clear();
	forcefieldBonds_.clear();
	forcefieldTorsions_.clear();
	uniqueForcefieldTypes_.clear();

	RefListItem<ForcefieldAtom,int>* ffa2;

	// Cycle over patterns, adding their unique forcefield terms to ours...
	for (Pattern* p = patterns_.first(); p != NULL; p = p->next)
	{
		Messenger::print(Messenger::Verbose, "Pattern '%s' uses %i atom types, %i bond terms, %i angle terms, and %i torsion terms.", qPrintable(p->name()), p->nUniqueForcefieldTypes(), p->nForcefieldBonds(), p->nForcefieldAngles(), p->nForcefieldTorsions());

		// Atom types. We only add types to the list that have a unique type name.
		for (RefListItem<ForcefieldAtom,int>* ffa1 = p->allForcefieldTypes(); ffa1 != NULL; ffa1 = ffa1->next)
		{
			// Add to list of unique (by pointer) types
			allForcefieldTypes_.addUnique(ffa1->item);
			// Add to list of unique (by name) types
			for (ffa2 = uniqueForcefieldTypes_.first(); ffa2 != NULL; ffa2 = ffa2->next) if (ffa1->item->name() == ffa2->item->name()) break;
			if (ffa2 != NULL) continue;
			uniqueForcefieldTypes_.add(ffa1->item);
		}

		// Bond terms
		for (RefListItem<ForcefieldBound,int>* ffb = p->forcefieldBonds(); ffb != NULL; ffb = ffb->next)
		{
			if (forcefieldBonds_.contains(ffb->item) == NULL) forcefieldBonds_.add(ffb->item);
		}

		// Angle terms
		for (RefListItem<ForcefieldBound,int>* ffb = p->forcefieldAngles(); ffb != NULL; ffb = ffb->next)
		{
			if (forcefieldAngles_.contains(ffb->item) == NULL) forcefieldAngles_.add(ffb->item);
		}

		// Torsion terms
		for (RefListItem<ForcefieldBound,int>* ffb = p->forcefieldTorsions(); ffb != NULL; ffb = ffb->next)
		{
			if (forcefieldTorsions_.contains(ffb->item) == NULL) forcefieldTorsions_.add(ffb->item);
		}
	}

	Messenger::print(Messenger::Verbose, "Model '%s' uses %i atom types, %i bond terms, %i angle terms, and %i torsion terms over all patterns.", qPrintable(name_), nUniqueForcefieldTypes(), nForcefieldBonds(), nForcefieldAngles(), nForcefieldTorsions());

	Messenger::exit("Model::createForcefieldLists");
}

// Return specified pair data from combination table
PointerPair<ForcefieldAtom,double>* Model::combinedParameters(ForcefieldAtom* at1, ForcefieldAtom* at2)
{
	if ((at1 == NULL) || (at2 == NULL))
	{
		printf("Warning - NULL atom type(s) passed to Model::combineParameters...\n");
		return NULL;
	}
	PointerPair<ForcefieldAtom,double>* pp = combinationTable_.find(at1, at2);
	if (pp == NULL) printf("Internal Error : Couldn't find combined parameters for atom types '%s' and '%s'.\n", qPrintable(at1->name()), qPrintable(at2->name()));
	return pp;
}
