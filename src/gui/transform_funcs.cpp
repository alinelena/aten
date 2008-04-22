/*
	*** Qt atom transform interface
	*** src/gui/transform_funcs.cpp
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

#include "base/master.h"
#include "gui/mainwindow.h"
#include "gui/gui.h"
#include "model/model.h"

void AtenForm::on_RotateClockwiseButton_clicked(bool on)
{
	rotateSelection(1);
}

void AtenForm::on_RotateAnticlockwiseButton_clicked(bool on)
{
	rotateSelection(-1);
}

/*
// Rotations
*/

void AtenForm::on_RotateDefineOriginButton_clicked(bool on)
{
	// Get geometric centre of selection
	Vec3<double> v = master.currentModel()->selectionCog();
	// Set widgets
	ui.RotateOriginXSpin->setValue(v.x);
	ui.RotateOriginYSpin->setValue(v.y);
	ui.RotateOriginZSpin->setValue(v.z);
}

void AtenForm::on_RotateDefineAxisButton_clicked(bool on)
{
	// Get geometric centre of selection and current origin
	Vec3<double> v, o;
	v = master.currentModel()->selectionCog();
	o.x = ui.RotateOriginXSpin->value();
	o.y = ui.RotateOriginYSpin->value();
	o.z = ui.RotateOriginZSpin->value();
	// Set widgets
	v -= o;
	ui.RotateAxisXSpin->setValue(v.x);
	ui.RotateAxisYSpin->setValue(v.y);
	ui.RotateAxisZSpin->setValue(v.z);
}

void AtenForm::rotateSelection(double direction)
{
	Vec3<double> v, o;
	v.x = ui.RotateAxisXSpin->value();
	v.y = ui.RotateAxisYSpin->value();
	v.z = ui.RotateAxisZSpin->value();
	o.x = ui.RotateOriginXSpin->value();
	o.y = ui.RotateOriginYSpin->value();
	o.z = ui.RotateOriginZSpin->value();
	char s[128];
	Model *m = master.currentModel();
	sprintf(s,"Rotate %i atom(s)\n",m->nSelected());
	m->beginUndostate(s);
	m->rotateSelectionVector(o, v, direction * ui.RotateAngleSpin->value());
	m->endUndostate();
	m->updateMeasurements();
	gui.mainView.postRedisplay();
}
