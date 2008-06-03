/*
	*** Master structure
	*** src/base/master.cpp
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

#include "model/model.h"
#include "classes/pattern.h"
#include "classes/grid.h"
#include "classes/clipboard.h"
#include "base/spacegroup.h"
#include "base/master.h"
#include "gui/gui.h"
#include "gui/mainwindow.h"
#include "gui/disorder.h"
#include "parse/parser.h"
#include <fstream>

Master master;

// Constructor
Master::Master()
{
	// Models
	modelId_ = 0;

	// Modes
	programMode_ = Master::GuiMode;

	// Store pointers to member functions
	initCommands();

	// Misc 
	sketchElement_ = 6;
	homeDir_ = "/tmp";
	defaultForcefield_ = NULL;

	// Clipboard
	userClipboard = new Clipboard;
}

// Destructor
Master::~Master()
{
	clear();
	delete userClipboard;
}

// Clear
void Master::clear()
{
	models_.clear();
	forcefields_.clear();
	userClipboard->clear();
	scripts.clear();
	for (int i=0; i<Filter::nFilterTypes; i++) filters_[i].clear();
}

// Sets the current program mode
void Master::setProgramMode(ProgramMode pm)
{
	programMode_ = pm;
}

// Return the current program mode
Master::ProgramMode Master::programMode()
{
	return programMode_;
}

// Set current drawing element
void Master::setSketchElement(short int el)
{
	sketchElement_ = el;
}

// Return current drawing element
short int Master::sketchElement()
{
	return sketchElement_;
}

/*
// GUI Routines
*/

// Set the active model
void Master::setCurrentModel(Model *m)
{
	dbgBegin(Debug::Calls,"Master::setCurrentModel");
	// Set current.m and tell the mainview canvas to display it
	current.m = m;
	current.rs = (current.m == NULL ? NULL : current.m->renderSource());
	// Set other Bundle objects based on model
	current.p = m->patterns();
	current.i = NULL;
	current.m->renderSource()->calculateViewMatrix();
	current.m->renderSource()->projectAll();
	dbgEnd(Debug::Calls,"Master::setCurrentModel");
}

/*
// Model Management routines
*/

// Return current active model for editing
Model *Master::currentModel() const
{
	return current.m;
}

// Return first item in the model list
Model *Master::models() const
{
	return models_.first();
}

// Return nth item in the model list
Model *Master::model(int n)
{
	return models_[n];
}

// Return the current model's index in the model list
int Master::currentModelId() const
{
	return models_.indexOf(current.m);
}

// Return index of specified model
int Master::modelIndex(Model *m) const
{
	return models_.indexOf(m);
}

// Return the number of models in the model list
int Master::nModels() const
{
	return models_.nItems();
}

// Add model
Model *Master::addModel()
{
	dbgBegin(Debug::Calls,"Master::addModel");
	Model *m = models_.add();
	char newname[16];
	sprintf(newname,"Unnamed%03i", ++modelId_);
	m->setName(newname);
	if (gui.exists())
	{
		gui.addModel(m);
		gui.disorderDialog->refresh();
	}
	setCurrentModel(m);
	dbgEnd(Debug::Calls,"Master::addModel");
	return m;
}

// Remove model
void Master::removeModel(Model *xmodel)
{
	// Remove this model from the model_list in the main window
	dbgBegin(Debug::Calls,"Master::removeModel");
	Model *m;
	// Unset the datamodel for the canvas
	// Delete the current model, but don't allow there to be zero models_...
	// (if possible, set the active row to the next model, otherwise  the previous)
	if (models_.nItems() == 1) m = master.addModel();
	else m = (xmodel->next != NULL ? xmodel->next : xmodel->prev);
	setCurrentModel(m);
	// Delete the old model (GUI first, then master)
	int id = models_.indexOf(xmodel);
	models_.remove(xmodel);
	if (gui.exists())
	{
		gui.removeModel(id);
		gui.disorderDialog->refresh();
	}
	dbgEnd(Debug::Calls,"Master::removeModel");
}

// Find model by name
Model *Master::findModel(const char *s) const
{
	// Search model list for name 's' (script function)
	dbgBegin(Debug::Calls,"Master::findModel");
	Model *result = NULL;
	for (result = models_.first(); result != NULL; result = result->next) if (strcmp(s,result->name()) == 0) break;
	dbgEnd(Debug::Calls,"Master::findModel");
	return result ;
}

/*
// Grid Management Routines
*/

// Return list of surfaces
Grid *Master::grids() const
{
	return grids_.first();
}

// Return number of surfaces loaded
int Master::nGrids() const
{
	return grids_.nItems();
}

// Return specified surface
Grid *Master::grid(int id)
{
	return grids_[id];
}

// Add new surface
Grid *Master::addGrid()
{
	current.g = grids_.add();
	return current.g;
}

// Remove surface
void Master::removeGrid(Grid *xgrid)
{
	xgrid->next != NULL ? current.g = xgrid->next : current.g = xgrid->prev;
	// Finally, delete the old surface
	grids_.remove(xgrid);
}

/*
// Forcefield Management routines
*/

// Add Forcefield
Forcefield *Master::addForcefield()
{
	current.ff = forcefields_.add();
	return current.ff;
}

// Load forcefield
Forcefield *Master::loadForcefield(const char *filename)
{
	dbgBegin(Debug::Calls,"Master::loadForcefield");
	// Try some different locations to find the supplied forcefield.
	static char s[512];
	bool result;
	Forcefield *newff = forcefields_.add();
	// First try - actual / absolute path
	msg(Debug::Verbose,"Looking for forcefield in absolute path (%s)...\n",filename);
	if (fileExists(filename)) result = newff->load(filename);
	else
	{
		// Second try - master.dataDir/ff
		sprintf(s,"%s/%s", dataDir_.get(), filename);
		msg(Debug::Verbose,"Looking for forcefield in installed location (%s)...\n",s);
		if (fileExists(s)) result = newff->load(s);
		else
		{
			// Last try - user home datadir/ff
			sprintf(s,"%s/.aten/ff/%s", homeDir_.get(), filename);
			msg(Debug::Verbose,"Looking for forcefield in user's data directory (%s)...\n",s);
			if (fileExists(s)) result = newff->load(s);
			else msg(Debug::None,"Can't find forcefield file '%s' in any location.\n", filename);
		}
	}
	if (result) current.ff = newff;
	else
	{
		msg(Debug::None,"Couldn't load forcefield file '%s'.\n", filename);
		forcefields_.remove(newff);
		newff = NULL;
	}
	dbgEnd(Debug::Calls,"Master::loadForcefield");
	return newff;
}

// Unload forcefield from the master's list
void Master::removeForcefield(Forcefield *xff)
{
	dbgBegin(Debug::Calls,"Master::removeForcefield");
	Forcefield *newff;
	// If possible, set the active row to the next model. Otherwise, the previous.
	xff->next != NULL ? newff = xff->next : newff = xff->prev;
	current.ff = newff;
	dereferenceForcefield(xff);
	// Finally, delete the ff
	forcefields_.remove(xff);
	dbgEnd(Debug::Calls,"Master::removeForcefield");
}

// Find forcefield by name
Forcefield *Master::findForcefield(const char *s) const
{
	// Search forcefield list for name 's' (script function)
	dbgBegin(Debug::Calls,"Master::findForcefield");
	Forcefield *ff;
	for (ff = forcefields_.first(); ff != NULL; ff = ff->next) if (strcmp(s,ff->name()) == 0) break;
	if (ff == NULL) msg(Debug::None,"Forcefield '%s' is not loaded.\n",s);
	dbgEnd(Debug::Calls,"Master::findForcefield");
	return ff;
}

// Dereference forcefield
void Master::dereferenceForcefield(Forcefield *xff)
{
	// Remove references to the forcefield in the models
	dbgBegin(Debug::Calls,"Master::dereferenceForcefield");
	for (Model *m = models_.first(); m != NULL; m = m->next)
	{
		if (m->forcefield() == xff)
		{
			m->removeTyping();
			m->setForcefield(NULL);
		}
		for (Pattern *p = m->patterns(); p != NULL; p = p->next)
		{
			if (p->forcefield() == xff)
			{
				Atom *i = p->firstAtom();
				for (int n=0; n<p->totalAtoms(); n++)
				{
					i->setType(NULL);
					i = i->next;
				}
				p->setForcefield(NULL);
			}
		}
	}
	dbgEnd(Debug::Calls,"Master::dereferenceForcefield");
}

// Set the default forcefield
void Master::setDefaultForcefield(Forcefield *ff)
{
	defaultForcefield_ = ff;
	if (defaultForcefield_ == NULL) msg(Debug::None,"Default forcefield has been unset.\n");
	else msg(Debug::None,"Default forcefield is now '%s'.\n", defaultForcefield_->name());
}

// Return the first ff in the list
Forcefield *Master::forcefields() const
{
	return forcefields_.first();
}

// Return the nth ff in the list
Forcefield *Master::forcefield(int n)
{
	return forcefields_[n];
}

// Return the number of loaded forcefields
int Master::nForcefields() const
{
	return forcefields_.nItems();
}

// Set active forcefield
void Master::setCurrentForcefield(Forcefield *ff)
{
	current.ff = ff;
}

// Set active forcefield by ID
void Master::setCurrentForcefield(int id)
{
	current.ff = forcefields_[id];
}

// Return the active forcefield
Forcefield *Master::currentForcefield() const
{
	return current.ff;
}

// Return ID of current forcefield
int Master::currentForcefieldId() const
{
	return forcefields_.indexOf(current.ff);
}

// Get the current default forcefield
Forcefield *Master::defaultForcefield() const
{
	return defaultForcefield_;
}

/*
// Locations
*/

// Set location of users's home directory
void Master::setHomeDir(const char *path)
{
	homeDir_ = path;
}

// Return the home directory path
const char *Master::homeDir()
{
	return homeDir_.get();
}

// Set working directory
void Master::setWorkDir(const char *path)
{
	workDir_ = path;
}

// Return the working directory path
const char *Master::workDir()
{
	return workDir_.get();
}

// Set data directory
void Master::setDataDir(const char *path)
{
	dataDir_ = path;
}

// Return the data directory path
const char *Master::dataDir()
{
	return dataDir_.get();
}

/*
// Filters
*/

// Load filters
bool Master::openFilters()
{
	dbgBegin(Debug::Calls,"Master::openFilters");
	char filename[512], path[512];
	bool found = FALSE, failed = FALSE;
	ifstream *listfile;

	// If ATENDATA is set, take data from there
	if (!master.dataDir_.empty())
	{
		printf("$ATENDATA points to '%s'.\n", dataDir_.get());
		sprintf(path,"%s%s", dataDir_.get(), "/filters/");
		sprintf(filename,"%s%s", dataDir_.get(), "/filters/index");
		listfile = new ifstream(filename,ios::in);
		if (listfile->is_open())
		{
			if (parseFilterIndex(path, listfile)) found = TRUE;
			else failed = TRUE;
		}
		listfile->close();
	}
	else printf("$ATENDATA has not been set. Searching default locations...\n");
	if ((!found) && (!failed))
	{
		// Try a list of known locations. Set dataDir again if we find a valid one
		sprintf(path,"%s%s", "/usr/share/aten", "/filters/");
		msg(Debug::None,"Looking for filter index in '%s'...\n", path);
		sprintf(filename,"%s%s", path, "index");
		listfile = new ifstream(filename, ios::in);
		if (listfile->is_open())
		{
			if (parseFilterIndex(path, listfile))
			{
				found = TRUE;
				dataDir_ = "/usr/share/aten/";
			}
			else failed = TRUE;
		}
		listfile->close();

		if ((!found) && (!failed))
		{
			sprintf(path,"%s%s", "/usr/local/share/aten", "/filters/");
			msg(Debug::None,"Looking for filter index in '%s'...\n", path);
			sprintf(filename,"%s%s", path, "index");
			listfile = new ifstream(filename, ios::in);
			if (listfile->is_open())
			{
				if (parseFilterIndex(path, listfile))
				{
					found = TRUE;
					dataDir_ = "/usr/local/share/aten/";
				}
				else failed = TRUE;
			}
			listfile->close();
		}

		if ((!found) && (!failed))
		{
			sprintf(path,"%s%s", qPrintable(gui.app->applicationDirPath()), "/../share/aten/filters/");
			msg(Debug::None,"Looking for filter index in '%s'...\n", path);
			sprintf(filename,"%s%s", path, "index");
			listfile = new ifstream(filename, ios::in);
			if (listfile->is_open())
			{
				if (parseFilterIndex(path, listfile))
				{
					found = TRUE;
					dataDir_ = qPrintable(gui.app->applicationDirPath() + "/../share/aten/");
				}
				else failed = TRUE;
			}
			listfile->close();
		}

		if ((!found) && (!failed))
		{
			sprintf(path,"%s%s", qPrintable(gui.app->applicationDirPath()), "/../SharedSupport/filters/");
			msg(Debug::None,"Looking for filter index in '%s'...\n", path);
			sprintf(filename,"%s%s", path, "index");
			listfile = new ifstream(filename, ios::in);
			if (listfile->is_open())
			{
				if (parseFilterIndex(path, listfile))
				{
					found = TRUE;
					dataDir_ = qPrintable(gui.app->applicationDirPath() + "/../SharedSupport/");
				}
				else failed = TRUE;
			}
			listfile->close();
		}

		if (!failed)
		{
			// Try for user filter index...
			sprintf(path,"%s%s", homeDir_.get(), "/.aten/filters/");
			msg(Debug::None,"Looking for user filter index in '%s'...\n", path);
			sprintf(filename, "%s/.aten/filters/index/", homeDir_.get());
			listfile = new ifstream(filename, ios::in);
			if (listfile->is_open())
			{
				if (parseFilterIndex(path, listfile)) found = TRUE;
				else failed = TRUE;
			}
			listfile->close();
		}

		if (!found)
		{
			printf("No filter index found in any of these locations.\n");
			printf("Set $ATENDATA to point to the (installed) location of the 'data' directory, or to the directory that contains Aten's 'filters' directory.\n");
			printf("e.g. (in bash) 'export ATENDATA=/usr/share/aten/' on most systems.\n");
		}
	}
	// Print out info and partner filters if all was successful
	if ((!failed) && found)
	{
		partnerFilters();
		msg(Debug::None,"Found (import/export):  Models (%i/%i) ", filters_[Filter::ModelImport].nItems(), filters_[Filter::ModelExport].nItems());
		msg(Debug::None,"Trajectory (%i/%i) ", filters_[Filter::TrajectoryImport].nItems(), filters_[Filter::TrajectoryExport].nItems());
		msg(Debug::None,"Expression (%i/%i) ", filters_[Filter::ExpressionImport].nItems(), filters_[Filter::ExpressionExport].nItems());
		msg(Debug::None,"Grid (%i/%i)\n", filters_[Filter::GridImport].nItems(), filters_[Filter::GridExport].nItems());
	}
	dbgEnd(Debug::Calls,"Master::openFilters");
	if (failed || (!found)) return FALSE;
	else return TRUE;
}

// Parse filter index file
bool Master::parseFilterIndex(const char *path, ifstream *indexfile)
{
	dbgBegin(Debug::Calls,"Master::parseFilterIndex");
	// Read the list of filters to load in...
	// Read filter names from file and open them
	char filterfile[512];
	printf("--> ");
	while (!indexfile->eof())
	{
		strcpy(filterfile,path);
		if (parser.getArgsDelim(indexfile, Parser::SkipBlanks) != 0) break;
		strcat(filterfile, parser.argc(0));
		printf("%s  ",parser.argc(0));
		if (!loadFilter(filterfile))
		{
			dbgEnd(Debug::Calls,"Master::parseFilterIndex");
			return FALSE;
		}
	}
	printf("\n");
	dbgEnd(Debug::Calls,"Master::parseFilterIndex");
	return TRUE;
}

// Read commands from filter file
bool Master::loadFilter(const char *filename)
{
	dbgBegin(Debug::Calls,"Master::loadFilter");
	Filter::FilterType ft;
	Filter *newfilter;
	bool error;
	int success;
	ifstream filterfile(filename,ios::in);

	// Pre-read first line to check
	success = parser.getArgsDelim(&filterfile,Parser::UseQuotes+Parser::SkipBlanks);
	error = FALSE;
	while (!filterfile.eof())
	{
		// Get filter type from first argument
		ft = Filter::filterType(parser.argc(0));
		// Unrecognised filter section?
		if (ft == Filter::nFilterTypes)
		{
			msg(Debug::None,"Unrecognised section '%s' in filter.\n",parser.argc(0));
			error = TRUE;
			break;
		}
		// Add main filter section
		newfilter = filters_[ft].add();
		newfilter->setType(ft);
		// Call the filter to load its commands.
		// If the load is not successful, remove the filter we just created
		if (!newfilter->load(filterfile))
		{
			printf("Error reading '%s' section from file '%s'\n", Filter::filterType(newfilter->type()), filename);
			filters_[ft].remove(newfilter);
			error = TRUE;
			break;
		}
	}
	filterfile.close();
	//variables.print();
	dbgEnd(Debug::Calls,"Master::loadFilter");
	return (!error);
}

// Set filter partners
void Master::partnerFilters()
{
	dbgBegin(Debug::Calls,"Master::partnerFilters");
	// Loop through import filters and search / set export partners
	Filter *imp, *exp;
	int importid;
	printf("Model Formats:");
	for (imp = filters_[Filter::ModelImport].first(); imp != NULL; imp = imp->next)
	{
		printf(" %s[r", imp->nickname());
		importid = imp->id();
		exp = NULL;
		if (importid != -1)
		{
			// Search for export filter with same ID as the importfilter
			for (exp = filters_[Filter::ModelExport].first(); exp != NULL; exp = exp->next)
			{
				if (importid == exp->id())
				{
					msg(Debug::Verbose, "--- Partnering model filters for '%s', id = %i\n", imp->nickname(), imp->id());
					imp->setPartner(exp);
					printf("w]");
					break;
				}
			}
		}
		if (exp == NULL) printf("o]");
	}
	printf("\n");
	printf("Grid Formats:");
	for (imp = filters_[Filter::GridImport].first(); imp != NULL; imp = imp->next)
	{
		printf(" %s[r", imp->nickname());
		importid = imp->id();
		exp = NULL;
		if (importid != -1)
		{
			// Search for export filter with same ID as the importfilter
			for (exp = filters_[Filter::GridExport].first(); exp != NULL; exp = exp->next)
			{
				if (importid == exp->id())
				{
					msg(Debug::Verbose, "--- Partnering grid filters for '%s', id = %i\n", imp->nickname(), imp->id());
					imp->setPartner(exp);
					printf("w]");
					break;
				}
			}
		}
		if (exp == NULL) printf("o]");
	}
	printf("\n");
	dbgEnd(Debug::Calls,"Master::partnerFilters");
}

// Find filter with specified type and nickname
Filter *Master::findFilter(Filter::FilterType ft, const char *nickname) const
{
	dbgBegin(Debug::Calls,"Master::findFilter");
	Filter *result;
	for (result = filters_[ft].first(); result != NULL; result = result->next)
		if (strcmp(result->nickname(), nickname) == 0) break;
	if (result == NULL) msg(Debug::None,"No %s filter with nickname '%s' defined.\n", Filter::filterType(ft), nickname);
	dbgEnd(Debug::Calls,"Master::findFilter");
	return result;
}

// Return first filter in list (of a given type)
Filter *Master::filters(Filter::FilterType ft) const
{
	return filters_[ft].first();
}

/*
// Progress Indicators
*/

// Initialise a progress indicator
void Master::initialiseProgress(const char *jobtitle, int totalsteps)
{
	gui.progressCreate(jobtitle, totalsteps);
}

// Update the number of steps (returns if the dialog was canceled)
bool Master::updateProgress(int currentstep)
{
	return gui.progressUpdate(currentstep);
}

// Terminate the current progress
void Master::cancelProgress()
{
	gui.progressTerminate();
}

// Spacegroup name search
int Master::findSpacegroupByName(const char *name) const
{
	dbgBegin(Debug::Calls,"Master::findSpacegroupByName");
	static char lcname[256], lcsg[256];
	strcpy(lcname,lowerCase(name));
	int result = 0;
	for (int n=1; n<231; n++)
	{
		strcpy(lcsg,lowerCase(spacegroups[n].name));
		if (strcmp(lcsg,lcname) == 0)
		{
			result = n;
			break;
		}
	}
	dbgEnd(Debug::Calls,"Master::findSpacegroupByName");
	return result;
}

// Cell type from spacegrgoup
Cell::CellType Master::spacegroupCellType(int sg) const
{
	dbgBegin(Debug::Calls,"Master::spacegroupCellType");
	Cell::CellType result = Cell::NoCell;
	// None
	if (sg == 0) result = Cell::NoCell;
	// Triclinic and monoclinic
	else if (sg < 16) result = Cell::ParallelepipedCell;
	// Orthorhombic and tetragonal
	else if (sg < 143) result = Cell::OrthorhombicCell;
	// Trigonal
	else if (sg < 168) result = Cell::ParallelepipedCell;
	// Hexagonal
	else if (sg < 195) result = Cell::NoCell;
	// Cubic
	else result = Cell::CubicCell;
	dbgBegin(Debug::Calls,"Master::spacegroupCellType");
	return result;
}
