/*
	*** Popup Widget - Transform Torsion
	*** src/gui/popuptransformtorsion.h
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

#ifndef ATEN_TRANSFORMTORSIONPOPUP_H
#define ATEN_TRANSFORMTORSIONPOPUP_H

#include "gui/ui_popuptransformtorsion.h"
#include "gui/tpopupwidget.hui"
#include "parser/returnvalue.h"

// Forward Declarations (Qt)
class AtenWindow;

ATEN_BEGIN_NAMESPACE

// Forward Declarations (Aten)
class ReturnValue;

ATEN_END_NAMESPACE

ATEN_USING_NAMESPACE

// Popup Widget - Transform/Set/Torsion
class TransformTorsionPopup : public TPopupWidget
{
	// All Qt declarations derived from QObject must include this macro
	Q_OBJECT

	private:
	// Reference to main window
	AtenWindow& parent_;

	public:
	// Constructor / Destructor
	TransformTorsionPopup(AtenWindow& parent, TMenuButton* buttonParent);
	// Main form declaration
	Ui::TransformTorsionPopup ui;
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
	 * Local Variables
	 */
	private:
	// Current angle (if three bound atoms are selected)
	double currentTorsion_;


	/*
	 * Widget Functions
	 */
	private slots:
	void on_CopyCurrentTorsionButton_clicked(bool checked);
	void on_SetTorsionButton_clicked(bool checked);
	void on_IncreaseTorsionButton_clicked(bool checked);
	void on_DecreaseTorsionButton_clicked(bool checked);
};

#endif
