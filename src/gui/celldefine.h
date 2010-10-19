/*
	*** Qt GUI: CellDefine Window
	*** src/gui/celldefine.h
	Copyright T. Youngs 2007-2010

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

#ifndef ATEN_CELLDEFINEWINDOW_H
#define ATEN_CELLDEFINEWINDOW_H

#include "gui/ui_celldefine.h"

// Cell definition window
class AtenCellDefine : public QDialog
{
	// All Qt declarations derived from QObject must include this macro
	Q_OBJECT

	/*
	// Window Functions
	*/
	private:
	void refreshMatrix();
	void refreshABC();
	public:
	void showWindow();
	void refresh();
	void cellChanged(int index, double newvalue);
	private slots:
	void on_CellDefinitionGroup_clicked(bool checked);
	void on_CellMatrixXXSpin_editingFinished();
	void on_CellMatrixXYSpin_editingFinished();
	void on_CellMatrixXZSpin_editingFinished();
	void on_CellMatrixYXSpin_editingFinished();
	void on_CellMatrixYYSpin_editingFinished();
	void on_CellMatrixYZSpin_editingFinished();
	void on_CellMatrixZXSpin_editingFinished();
	void on_CellMatrixZYSpin_editingFinished();
	void on_CellMatrixZZSpin_editingFinished();
	void on_DefineFromABCButton_clicked(bool checked);
	void on_CellSpacegroupSetButton_clicked(bool checked);
	void on_CellSpacegroupEdit_returnPressed();
	void on_CellSpacegroupRemoveButton_clicked(bool checked);
	void on_CellSpacegroupPackButton_clicked(bool checked);
	void dialogFinished(int result);

	/*
	// Local variables
	*/
	private:
	// Whether the window is refreshing
	bool refreshing_;

	/*
	// Dialog
	*/
	public:
	// Constructor / Destructor
	AtenCellDefine(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~AtenCellDefine();
	// Main form declaration
	Ui::CellDefineDialog ui;
};

#endif
