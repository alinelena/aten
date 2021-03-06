/*
	*** Viewer - Extra scene rendering
	*** src/gui/viewer_extras.cpp
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

#include "gui/viewer.hui"
#include "gui/mainwindow.h"
#include "main/aten.h"
#include "base/sysfunc.h"
#include "methods/partitiondata.h"
#include <QPen>
#include <QPainter>

// Render extras from specific popups / windows
void Viewer::renderExtras(Model* sourceModel)
{
        // Disorder Wizard
	if (atenWindow_->disorderWizard_.isVisible() && (sourceModel == atenWindow_->disorderWizard_.targetModel()))
	{
		PartitioningScheme *ps = NULL;
		Matrix mat;
		Vec4<GLfloat> colour;
		if (atenWindow_->disorderWizard_.currentId() > 2)
		{
			ps = atenWindow_->disorderWizard_.partitioningScheme();
			mat = atenWindow_->disorderWizard_.cell().axes();

			// Grab the grid size which the scheme uses and initialise colour counter
			Vec3<int> nPoints = ps->gridSize();
			mat.applyScaling(1.0/nPoints.x, 1.0/nPoints.y, 1.0/nPoints.z);
			int colcount = 0;

			for (PartitionData* pd = ps->partitions(); pd != NULL; pd = pd->next)
			{                       
				if (pd->id() == 0) continue;

				// Use first three bits of colcount to set our colour values
				colour.x = colcount%1 ? 1.0 : 0.0;
				colour.y = colcount%2 ? 1.0 : 0.0;
				colour.z = colcount%4 ? 1.0 : 0.0;
				colour.w = 0.75;

				// Partition surfaces will have already been constructed, so just display them
				renderGroup_.addTriangles(pd->primitive(), mat, colour, GL_FILL);

				// Increase colour counter
				++colcount;
				if (colcount > 7) colcount = 0;
			}
			
		}
	}

	// CellReplicate popup
	if (atenWindow_->ui.CellTransformReplicateButton->popupVisible())
	{
		Vec3<double> r1, r2, scale, translate;
		Vec3<int> iNeg, iPos;
		Vec4<GLfloat> colour;
		ReturnValue rv;
		atenWindow_->ui.CellTransformReplicateButton->callPopupMethod("negativeVector", rv);
		r1 = rv.asVector();
		atenWindow_->ui.CellTransformReplicateButton->callPopupMethod("positiveVector", rv);
		r2 = rv.asVector();
		iNeg.set(ceil(r1.x), ceil(r1.y), ceil(r1.z));
		iPos.set(floor(r2.x), floor(r2.y), floor(r2.z));
		prefs.copyColour(prefs.currentForegroundColour(), colour);
		Matrix A = sourceModel->modelViewMatrix() * sourceModel->cell().axes(), B;
		glMultMatrixd(A.matrix());
		for (int i = iNeg.x-1; i<=iPos.x; ++i)
		{
			// Construct translation vector (partial part)
			if (i == iNeg.x-1) { scale.x = iNeg.x-r1.x; translate.x = iNeg.x - 0.5*scale.x - 0.5; }
			else if (i == iPos.x) { scale.x = r2.x-iPos.x; translate.x = iPos.x-1 + 0.5*scale.x + 0.5; }
			else { scale.x = 1.0; translate.x = i*1.0; }
			for (int j = iNeg.y-1; j<=iPos.y; ++j)
			{
				// Construct translation vector (partial part)
				if (j == iNeg.y-1) { scale.y = iNeg.y-r1.y; translate.y = iNeg.y - 0.5*scale.y - 0.5; }
				else if (j == iPos.y) { scale.y = r2.y-iPos.y; translate.y = iPos.y-1 + 0.5*scale.y + 0.5; }
				else { scale.y = 1.0; translate.y = j*1.0; }

				for (int k = iNeg.z-1; k<=iPos.z; ++k)
				{
					// Construct translation vector (partial part)
					if (k == iNeg.z-1) { scale.z = iNeg.z-r1.z; translate.z = iNeg.z - 0.5*scale.z - 0.5; }
					else if (k == iPos.z) { scale.z = r2.z-iPos.z; translate.z = iPos.z-1 + 0.5*scale.z + 0.5; }
					else { scale.z = 1.0; translate.z = k*1.0; }

					// Check scaling factors
					if ((scale.x < 0.001) || (scale.y < 0.001) || (scale.z < 0.001)) continue;

					glPushMatrix();
					B = A;
					B.applyTranslation(translate);
					B.applyScaling(scale);
					renderGroup_.addTriangles(primitives_[primitiveSet_].wireCube(), B, colour, GL_LINES);
				}
			}
		}
	}

	// Miller Planes
	if (atenWindow_->ui.CellMillerShowButton->isChecked())
	{
		// Get hkl values
		int h = atenWindow_->ui.CellMillerHSpin->value();
		int k = atenWindow_->ui.CellMillerKSpin->value();
		int l = atenWindow_->ui.CellMillerLSpin->value();

		// Get Miller planes from model's cell
		Plane firstPlane, secondPlane;
		sourceModel->cell().millerPlanes(h, k, l, firstPlane, secondPlane);

		// Add triangle / quad
		Vec4<GLfloat> colour;
		prefs.copyColour(prefs.currentForegroundColour(), colour);
		colour.w = 0.3;
		if (firstPlane.nVertices() == 3)
		{
			// Draw triangle
			renderGroup_.addExtraSolidTriangle(firstPlane.vertex(0), firstPlane.vertex(1), firstPlane.vertex(2), firstPlane.normal(), colour);
			renderGroup_.addExtraSolidTriangle(secondPlane.vertex(0), secondPlane.vertex(1), secondPlane.vertex(2), secondPlane.normal(), colour);
		}
		else
		{
			// Draw quad
			renderGroup_.addExtraSolidTriangle(firstPlane.vertex(0), firstPlane.vertex(1), firstPlane.vertex(2), firstPlane.normal(), colour);
			renderGroup_.addExtraSolidTriangle(firstPlane.vertex(0), firstPlane.vertex(2), firstPlane.vertex(3), firstPlane.normal(), colour);
			renderGroup_.addExtraSolidTriangle(secondPlane.vertex(0), secondPlane.vertex(1), secondPlane.vertex(2), secondPlane.normal(), colour);
			renderGroup_.addExtraSolidTriangle(secondPlane.vertex(0), secondPlane.vertex(2), secondPlane.vertex(3), secondPlane.normal(), colour);
		}
	}

		// Pores Widget // ATEN2 TODO
// 			if ((!gui.poresWidget->isVisible()) || (gui.poresWidget->ui.PoreTabWidget->currentIndex() != 2)) continue;
// 			ps = &PoresWidget::partitioningScheme();
// 			mat = aten.currentModelOrFrame()->cell()->axes();
}
