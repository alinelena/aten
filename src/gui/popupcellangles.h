/*
	*** Popup Widget - Cell Angles
	*** src/gui/popupcellangles.h
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

#ifndef ATEN_CELLANGLESPOPUP_H
#define ATEN_CELLANGLESPOPUP_H

#include "gui/ui_popupcellangles.h"
#include "gui/tpopupwidget.hui"
#include "parser/returnvalue.h"

// Forward Declarations (Qt)
class AtenWindow;

ATEN_BEGIN_NAMESPACE

// Forward Declarations (Aten)
class ReturnValue;

ATEN_END_NAMESPACE

ATEN_USING_NAMESPACE

// Popup Widget - Cell Angles
class CellAnglesPopup : public TPopupWidget
{
	// All Qt declarations derived from QObject must include this macro
	Q_OBJECT

	private:
	// Reference to main window
	AtenWindow& parent_;

	public:
	// Constructor / Destructor
	CellAnglesPopup(AtenWindow& parent, TMenuButton* buttonParent);
	// Main form declaration
	Ui::CellAnglesPopup ui;
	// Update controls (before show()) (virtual)
	void updateControls();
	// Call named method associated to popup
	bool callMethod(QString methodName, ReturnValue& rv);


	/*
	 * Reimplementations
	 */
	protected:
	void hideEvent(QHideEvent* event) { TPopupWidget::hideEvent(event); }


	/*
	 * Widget Functions
	 */
	private:
	// Adjust matrix of current model
	void adjustCurrentMatrix(int angleIndex, double value);

	private slots:
	void on_AlphaSpin_valueChanged(double value);
	void on_BetaSpin_valueChanged(double value);
	void on_GammaSpin_valueChanged(double value);
};

#endif
