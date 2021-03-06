/*
	*** Undo State
	*** src/undo/undostate.h
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

#ifndef ATEN_UNDOSTATE_H
#define ATEN_UNDOSTATE_H

#include "base/log.h"
#include "base/namespace.h"
#include "templates/list.h"

ATEN_BEGIN_NAMESPACE

// Forward Declarations (Aten)
class Model;
class UndoEvent;

// UndoState (series of UndoEvents)
class UndoState : public ListItem<UndoState>
{
	public:
	// Constructor
	UndoState();

	/*
	 * Changelist
	 */
	private:
	// List of atomic changes for this level
	List<UndoEvent> events_;
	// Short text describing the change
	QString description_;
	// Logs at start of state
	Log startLogs_;
	// Logs at end of state
	Log endLogs_;

	public:
	// Add event to state
	void addEvent(UndoEvent* ue);
	// Return number of changes in list
	int nChanges() const;
	// Set logs at start of state
	void setStartLogs(Log source);
	// Get structure log point at start of state
	int startLog(Log::LogType log) const;
	// Set logs at end of state
	void setEndLogs(Log source);
	// Get structure log point at end of state
	int endLog(Log::LogType log) const;
	// Check difference between Change::StructureLog and Change::CoordinateLog between start/end points
	bool doLogsDiffer() const;
	// Set the text associated with the current undo state
	void setDescription(QString description);
	// Return the current text associated with the state
	QString description() const;
	// Undo the changes specified in the state
	void undo(Model* m);
	// Redo the changes specified in the state
	void redo(Model* m);
	// Print changes captured in state
	void print() const;
};

ATEN_END_NAMESPACE

#endif
