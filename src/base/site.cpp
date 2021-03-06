/*
	*** Molecule site
	*** src/base/site.cpp
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

#include "base/site.h"
#include "base/pattern.h"
#include "base/sysfunc.h"

ATEN_USING_NAMESPACE

// Site types
const char* ST_strings[ST_NITEMS] = { "Molecule COM", "Molecule COG", "Atom(s) COM", "Atom(s) COG" };
const char* ST_keywords[ST_NITEMS] = { "molcom", "molcog", "atomcom", "atomcog" };
const char* text_from_ST(SiteType i)
	{ return ST_strings[i]; }
SiteType ST_from_text(const char* s)
	{ return (SiteType) enumSearch("site type",ST_NITEMS,ST_keywords,s); }

// Constructor
Site::Site() : ListItem<Site>()
{
	// Private variables
	type_ = ST_MOLCOM;
	pattern_ = NULL;
	centre_.zero();
}

// Set the pattern pointer for the atom
void Site::setPattern(Pattern* p)
{
	pattern_ = p;
}

// Returns the current pattern for the atom
Pattern* Site::pattern()
{
	return pattern_;
}

// Set name of site
void Site::setName(QString name)
{
	name_ = name;
}

// Get name of site
QString Site::name()
{
	return name_;
}

// Set type of site centre
void Site::setType(SiteType type)
{
	type_ = type;
}

// Set centre of site
void Site::setCentre(Vec3<double> v)
{
	centre_ = v;
}

// Set axes of site
void Site::setAxes(Matrix axes)
{
	axes_ = axes;
}
