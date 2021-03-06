/*
	*** Log Class
	*** src/base/log.cpp
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

#include "math/constants.h"
#include "base/messenger.h"
#include "base/log.h"
#include <stdio.h>

ATEN_USING_NAMESPACE

// Constructor
Log::Log()
{
	reset();
}

// Operator == (test equivalence of all log values we care about)
bool Log::operator==(Log &l) const
{
	if (logs_[Log::Structure] != l.logs_[Log::Structure]) return false;
	if (logs_[Log::Coordinates] != l.logs_[Log::Coordinates]) return false;
	if (logs_[Log::Cell] != l.logs_[Log::Cell]) return false;
	if (logs_[Log::Style] != l.logs_[Log::Style]) return false;
	if (logs_[Log::Selection] != l.logs_[Log::Selection]) return false;
	if (logs_[Log::Glyphs] != l.logs_[Log::Glyphs]) return false;
	if (logs_[Log::Grids] != l.logs_[Log::Grids]) return false;
	if (logs_[Log::Labels] != l.logs_[Log::Labels]) return false;
	if (logs_[Log::Misc] != l.logs_[Log::Misc]) return false;
	return true;
}

bool Log::operator!=(Log &l) const
{
	if (logs_[Log::Structure] != l.logs_[Log::Structure]) return true;
	if (logs_[Log::Coordinates] != l.logs_[Log::Coordinates]) return true;
	if (logs_[Log::Cell] != l.logs_[Log::Cell]) return true;
	if (logs_[Log::Style] != l.logs_[Log::Style]) return true;
	if (logs_[Log::Selection] != l.logs_[Log::Selection]) return true;
	if (logs_[Log::Glyphs] != l.logs_[Log::Glyphs]) return true;
	if (logs_[Log::Grids] != l.logs_[Log::Grids]) return true;
	if (logs_[Log::Labels] != l.logs_[Log::Labels]) return true;
	if (logs_[Log::Misc] != l.logs_[Log::Misc]) return true;
	return false;
}

// Reset all logs to zero
void Log::reset()
{
	for (int n=0; n<Log::nLogTypes; ++n) logs_[n] = 0;
	savePoint_ = 0;
}

// Set a log to a specific value
void Log::setLog(Log::LogType lt, int value)
{
	logs_[Log::Total] -= logs_[lt];
	logs_[lt] = value;
	logs_[Log::Total] += value;
	
}

// Log change
void Log::add(Log::LogType cl)
{
	logs_[cl] ++;
	logs_[Log::Total] ++;
}

// Return the log quantity specified
int Log::log(Log::LogType cl) const
{
	return logs_[cl];
}

// Return current 'save' point of the model (sum of specific logs)
int Log::currentLogPoint() const
{
	return (logs_[Log::Structure] + logs_[Log::Coordinates] + logs_[Log::Cell] + logs_[Log::Misc] + logs_[Log::Glyphs] + logs_[Log::Grids] + logs_[Log::Labels] + logs_[Log::Selection]);
}

// Set the save point log for the model
void Log::updateSavePoint()
{
	savePoint_ = currentLogPoint();
}

// Return if the log has been modified since last being saved
bool Log::isModified() const
{
	return (savePoint_ != currentLogPoint());
}

// Return is specified log quantity is same between this and supplied Log
bool Log::isSame(Log::LogType lt, Log &other)
{
	return (logs_[lt] == other.logs_[lt]);
}

// Print logs
void Log::print() const
{
	Messenger::print("Structure [%i], Coordinates [%i], Cell [%i], Style [%i], Selection [%i], Glyphs [%i], Grids [%i], Labels [%i], Misc [%i], Total [%i]", logs_[Log::Structure], logs_[Log::Coordinates], logs_[Log::Cell], logs_[Log::Style], logs_[Log::Selection], logs_[Log::Glyphs], logs_[Log::Grids], logs_[Log::Labels], logs_[Log::Misc], logs_[Log::Total]); 
}
