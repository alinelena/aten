/*
	*** Trajectory Commands
	*** src/command/trajectory.cpp
	Copyright T. Youngs 2007-2015

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

#include "command/commands.h"
#include "parser/commandnode.h"
#include "main/aten.h"
#include "model/model.h"
#include "gui/mainwindow.h"

ATEN_USING_NAMESPACE

// Add new frame to the current model's trajectory
bool Commands::function_AddFrame(CommandNode* c, Bundle& obj, ReturnValue& rv)
{
	if (obj.notifyNull(Bundle::ModelPointer)) return FALSE;
	obj.m->addTrajectoryFrame();
	obj.m->setRenderSource(Model::TrajectorySource);
	if (c->hasArg(0)) obj.rs()->setName(c->argc(0));
	rv.set(VTypes::ModelData, obj.rs());
	return TRUE;
}

// Clear any trajectory data in the current model
bool Commands::function_ClearTrajectory(CommandNode* c, Bundle& obj, ReturnValue& rv)
{
	if (obj.notifyNull(Bundle::ModelPointer)) return FALSE;
	if (obj.rs() != obj.m)
	{
		Messenger::print("Current model is a trajectory frame - resetting to the parent model...\n");
		obj.m->setRenderSource(Model::ModelSource);
	}
	obj.m->clearTrajectory();
	rv.reset();
	return TRUE;
}

// Finalise current trajectory frame
bool Commands::function_FinaliseFrame(CommandNode* c, Bundle& obj, ReturnValue& rv)
{
	if (obj.notifyNull(Bundle::ModelPointer)) return FALSE;
	if (obj.rs() == obj.m)
	{
		Messenger::print( "Current model does not appear to be a trajectory frame.\n");
		return FALSE;
	}
	// Do various necessary calculations
	if (prefs.coordsInBohr()) obj.rs()->bohrToAngstrom();
	obj.rs()->renumberAtoms();
	if (!prefs.keepView()) obj.rs()->resetView(aten_.atenWindow()->ui.MainView->contextWidth(), aten_.atenWindow()->ui.MainView->contextHeight());
	obj.rs()->calculateMass();
	obj.rs()->selectNone();
	obj.rs()->changeLog.reset();
	obj.rs()->changeLog.updateSavePoint();
	obj.rs()->setFilter(NULL);
	obj.rs()->setFilename("frame");
	obj.rs()->enableUndoRedo();
	//if (frame->cell()->type() != Cell::NoCell) frame->cell()->print();
	rv.reset();
	return TRUE;
}

// Skip to first frame ('firstframe')
bool Commands::function_FirstFrame(CommandNode* c, Bundle& obj, ReturnValue& rv)
{
	if (obj.notifyNull(Bundle::ModelPointer)) return FALSE;
	if (obj.m->nTrajectoryFrames() == 0)
	{
		Messenger::print("No trajectory associated to model '%s'.\n",obj.m->name());
		return FALSE;
	}
	obj.m->seekFirstTrajectoryFrame();
	rv.reset();
	return TRUE;
}

// Skip to last frame ('lastframe')
bool Commands::function_LastFrame(CommandNode* c, Bundle& obj, ReturnValue& rv)
{
	if (obj.notifyNull(Bundle::ModelPointer)) return FALSE;
	if (obj.m->nTrajectoryFrames() == 0)
	{
		Messenger::print("No trajectory associated to model '%s'.\n",obj.m->name());
		return FALSE;
	}
	obj.m->seekLastTrajectoryFrame();
	rv.reset();
	return TRUE;
}

// Open and associate trajectory ('loadtrajectory <file>')
bool Commands::function_LoadTrajectory(CommandNode* c, Bundle& obj, ReturnValue& rv)
{
	if (obj.notifyNull(Bundle::ModelPointer)) return FALSE;
	Tree* filter = aten_.probeFile(c->argc(0), FilterData::TrajectoryImport);
	if (filter == NULL) return FALSE;
	bool result = obj.m->initialiseTrajectory(c->argc(0), filter);
	rv.set(result);
	return TRUE;
}

// Go to next frame ('nextframe')
bool Commands::function_NextFrame(CommandNode* c, Bundle& obj, ReturnValue& rv)
{
	if (obj.notifyNull(Bundle::ModelPointer)) return FALSE;
	if (obj.m->nTrajectoryFrames() == 0)
	{
		Messenger::print("No trajectory associated to model '%s'.\n",obj.m->name());
		return FALSE;
	}
	obj.m->seekNextTrajectoryFrame();
	rv.reset();
	return TRUE;
}

// Go to previous frame ('prevframe')
bool Commands::function_PrevFrame(CommandNode* c, Bundle& obj, ReturnValue& rv)
{
	if (obj.notifyNull(Bundle::ModelPointer)) return FALSE;
	if (obj.m->nTrajectoryFrames() == 0)
	{
		Messenger::print("No trajectory associated to model '%s'.\n",obj.m->name());
		return FALSE;
	}
	obj.m->seekPreviousTrajectoryFrame();
	rv.reset();
	return TRUE;
}

// Seek to specified frame ('seekframe <n>')
bool Commands::function_SeekFrame(CommandNode* c, Bundle& obj, ReturnValue& rv)
{
	if (obj.notifyNull(Bundle::ModelPointer)) return FALSE;
	if (obj.m->nTrajectoryFrames() == 0)
	{
		Messenger::print("No trajectory associated to model '%s'.\n",obj.m->name());
		return FALSE;
	}
	obj.m->seekTrajectoryFrame(c->argi(0)-1);
	rv.reset();
	return TRUE;
}
