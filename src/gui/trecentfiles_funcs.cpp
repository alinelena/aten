/*
	*** Recent Files Widget
	*** src/gui/trecentfiles_funcs.cpp
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

#include "gui/trecentfiles.hui"
#include "base/namespace.h"
#include <QVBoxLayout>
#include <QTableWidget>
#include <QFileInfo>
#include <QMenu>

ATEN_USING_NAMESPACE

// Constructor
TRecentFiles::TRecentFiles(QWidget* parent) : QWidget(parent), maxRecentFiles_(100)
{
	// Create a suitable layout to contain our controls
	layout_ = new QVBoxLayout(this);
	layout_->setMargin(2);
	layout_->setSpacing(2);

	// Create widgets
	filesTable_ = new QTableWidget(this);
	layout_->addWidget(filesTable_);

	refreshing_ = false;

	// Connect signal for context menu on FilesTable
	connect(filesTable_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(filesTableContextMenuRequested(QPoint)));
	connect(filesTable_, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(filesTableItemClicked(QTableWidgetItem*)));
}

// Update controls
void TRecentFiles::updateControls()
{
	refreshing_ = true;

	// Recreate the list
	filesTable_->clear();
	filesTable_->setColumnCount(1);
	filesTable_->setRowCount(recentFiles_.count());

	QTableWidgetItem* item;
	int count = 0;
	for (int n=0; n<recentFiles_.count(); ++n)
	{
		QFileInfo fileInfo(recentFiles_.at(n));
		if (!fileInfo.exists())
		{
			recentFiles_.removeAt(n);
			continue;
		}

		item = new QTableWidgetItem(recentFiles_.at(n));
		item->setToolTip(recentFiles_.at(n));
		item->setData(Qt::UserRole, count);
		filesTable_->setItem(count++, 0, item);
	}

	filesTable_->setColumnWidth(0, width());
	filesTable_->setRowCount(count);

	refreshing_ = false;
}

/*
 * Data
 */

// Add file to list
void TRecentFiles::addFile(QString fileName)
{
	// Get file information for the supplied filename
	QFileInfo newFileInfo(fileName);

	// Check to see if the file already exists in the list
	for (int n=0; n<recentFiles_.count(); ++n)
	{
		QFileInfo oldFileInfo(recentFiles_.at(n));
		if (newFileInfo == oldFileInfo)
		{
			recentFiles_.move(n, 0);
			return;
		}
	}

	// Not in the list, so add it to the top
	recentFiles_.prepend(newFileInfo.absoluteFilePath());

	// Remove files until we reach the max number (or lower) of allowable files
	while (recentFiles_.count() > maxRecentFiles_) recentFiles_.removeLast();
}

// Return maximum allowable files in list
int TRecentFiles::maxFiles()
{
	return maxRecentFiles_;
}

// Return number of files in list
int TRecentFiles::nFiles()
{
	return recentFiles_.count();
}

// Return nth recent file in list
QString TRecentFiles::file(int n)
{
	return recentFiles_.at(n);
}

/*
 * Widget Functions
 */

void TRecentFiles::filesTableItemClicked(QTableWidgetItem* item)
{
	if (item == NULL) return;

	// Item selected, so emit notification signal
	emit(fileSelected(item->text()));
}

// Context menu requested for FilesTable
void TRecentFiles::filesTableContextMenuRequested(const QPoint& point)
{
	// Is there an item under the pointer?
	QTableWidgetItem* item = filesTable_->itemAt(point);
	if (!item) return;

	// Build the context menu to display
	QMenu contextMenu;
	QAction* removeAction = contextMenu.addAction("&Remove Entry");
// 	QAction* pinAction = contextMenu.addAction("&Pin");
// 	QAction* unpinAction = contextMenu.addAction("&Unpin");
	
	// Show it
	QAction* menuResult = contextMenu.exec(QCursor::pos());

	// What was clicked?
	if (menuResult == removeAction)
	{
		recentFiles_.removeAt(item->data(Qt::UserRole).toInt());
		updateControls();
	}
// 	else if (menuResult == pinAction)
// 	{
// 	}
// 	else if (menuResult == unpinAction)
// 	{
// 	}
}