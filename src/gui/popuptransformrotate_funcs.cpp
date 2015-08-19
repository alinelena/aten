/*
	*** Popup Widget - Transform Rotate Functions
	*** src/gui/popuptransformrotate_funcs.cpp
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

#include "gui/popuptransformrotate.h"
#include "main/aten.h"
#include "gui/mainwindow.h"
#include "base/namespace.h"
#include "command/commands.h"

ATEN_USING_NAMESPACE

// Constructor
TransformRotatePopup::TransformRotatePopup(AtenWindow& parent, TMenuButton* buttonParent) : TMenuButtonPopupWidget(buttonParent), parent_(parent)
{
	// Set up interface
	ui.setupUi(this);
}

// Update controls (before show()) (virtual)
void TransformRotatePopup::updateControls()
{
	refreshing_ = true;

	refreshing_ = false;
}

// Call named method associated to popup
bool TransformRotatePopup::callMethod(QString methodName, ReturnValue& rv)
{
	if (methodName == "TEST") return true;
	else if (methodName == "hideEvent")
	{
		return true;
	}
	else printf("No method called '%s' is available in this popup.\n", qPrintable(methodName));
	return false;
}

/*
 * Widget Functions
 */

void TransformRotatePopup::on_DefineAxisButton_clicked(bool checked)
{
	// Get geometric centre of selection and current origin
	Vec3<double> v, o;
	v = parent_.aten().currentModelOrFrame()->selectionCentreOfGeometry();
	o.x = ui.OriginXSpin->value();
	o.y = ui.OriginYSpin->value();
	o.z = ui.OriginZSpin->value();

	// Set widgets
	v -= o;
	ui.AxisXSpin->setValue(v.x);
	ui.AxisYSpin->setValue(v.y);
	ui.AxisZSpin->setValue(v.z);
}

void rotatePickAxisButton_callback(RefList<Atom,int>* picked)
{
// 	ui.PickAxisButton->setChecked(false); ATEN2 TODO
// 	// If there are not two atoms in the list then the mode must have been canceled
// 	if (picked->nItems() != 2) return;
// 	Vec3<double> v = picked->last()->item->r();
// 	v -= picked->first()->item->r();
// 	ui.AxisXSpin->setValue(v.x);
// 	ui.AxisYSpin->setValue(v.y);
// 	ui.AxisZSpin->setValue(v.z);
}

void TransformRotatePopup::on_PickAxisButton_clicked(bool checked)
{
	// Enter manual picking mode
	(UserAction::RotatePickAxisAction,2,&rotatePickAxisButton_callback);
}

void TransformRotatePopup::on_DefineOriginButton_clicked(bool checked)
{
	// Get geometric centre of selection
	Vec3<double> v = parent_.aten().currentModelOrFrame()->selectionCentreOfGeometry();

	// Set widgets
	ui.OriginXSpin->setValue(v.x);
	ui.OriginYSpin->setValue(v.y);
	ui.OriginZSpin->setValue(v.z);
}

void TransformRotatePopup::on_RotateClockwiseButton_clicked(bool checked)
{
	// Grab vectors
	Vec3<double> v, o;
	v.x = ui.AxisXSpin->value();
	v.y = ui.AxisYSpin->value();
	v.z = ui.AxisZSpin->value();
	o.x = ui.OriginXSpin->value();
	o.y = ui.OriginYSpin->value();
	o.z = ui.OriginZSpin->value();

	// Run command
	CommandNode::run(Commands::AxisRotate, "ddddddd", v.x, v.y, v.z, ui.AngleSpin->value(), o.x, o.y, o.z);

	// Update
	parent_.updateWidgets(AtenWindow::MainViewTarget+AtenWindow::AtomsTableTarget);
}

void TransformRotatePopup::on_RotateAnticlockwiseButton_clicked(bool checked)
{
	// Grab vectors
	Vec3<double> v, o;
	v.x = ui.AxisXSpin->value();
	v.y = ui.AxisYSpin->value();
	v.z = ui.AxisZSpin->value();
	o.x = ui.OriginXSpin->value();
	o.y = ui.OriginYSpin->value();
	o.z = ui.OriginZSpin->value();

	// Run command
	CommandNode::run(Commands::AxisRotate, "ddddddd", v.x, v.y, v.z, -ui.AngleSpin->value(), o.x, o.y, o.z);

	// Update
	parent_.updateWidgets(AtenWindow::MainViewTarget+AtenWindow::AtomsTableTarget);
}
