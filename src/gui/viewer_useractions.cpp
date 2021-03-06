/*
	*** User Actions Rendering
	*** src/gui/viewer_useractions.cpp
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

#include "main/aten.h"
#include "gui/viewer.hui"
#include "model/model.h"
#include "model/fragment.h"
#include "base/sysfunc.h"
#include "gui/mainwindow.h"
#include <QPainter>

// Render active mode embellishments
void Viewer::renderActiveModes(Model* currentModel)
{
	// Variables
	GLfloat colour[4];
	int i, skip, n;
	double dx, halfw;

	prefs.copyColour(prefs.currentForegroundColour(), colour);
	glColor4fv(colour);
	switch (atenWindow_->activeMode())
	{
		case (UserAction::NoAction):
			break;
		// Only selection mode where we draw a selection box
		case (UserAction::SelectAction):

			glBegin(GL_LINE_LOOP);
			glVertex2d(rMouseDown_.x, contextHeight_-rMouseDown_.y);
			glVertex2d(rMouseDown_.x, contextHeight_-rMouseLast_.y);
			glVertex2d(rMouseLast_.x, contextHeight_-rMouseLast_.y);
			glVertex2d(rMouseLast_.x, contextHeight_-rMouseDown_.y);
			glEnd();
			break;
		default:
			break;
	}

// 	// Passive mode embellishments
// 	if (currentModel != NULL) switch (atenWindow_->selectedMode())
// 	{
// 		// Draw on distance ruler for drawing modes //ATEN2 TODO
// 		case (UserAction::DrawAtomsAction):
// 			// Get pixel 'length' in Angstrom terms at current draw depth
// 			r = currentModel->screenToModel(contextWidth_/2+10, contextHeight_/2, atenWindow_->currentDrawDepth());
// 			r -= currentModel->screenToModel(contextWidth_/2, contextHeight_/2, atenWindow_->currentDrawDepth());
// 			dx = 10.0 / r.magnitude();
// 			
// 			halfw = contextWidth_ / 2.0;
// 			i = int( halfw / dx);
// 			skip = 1;
// 			while ( (i/skip) > 5)
// 			{
// 				skip += (skip == 1 ? 4 : 5);
// 			}
// 			for (n = -i; n <= i; n ++)
// 			{
// 				if ((n%skip) != 0) continue;
// 				painter.drawLine(halfw + n*dx, 20, halfw + n*dx, 10);
// 				painter.drawLine(halfw + n*dx, contextHeight_-20, halfw + n*dx, contextHeight_-10);
// 				if (n != i)
// 				{
// 					painter.drawLine(halfw + (n+0.5*skip)*dx, contextHeight_-15, halfw + (n+0.5*skip)*dx, contextHeight_-10);
// 					painter.drawLine(halfw + (n+0.5*skip)*dx, contextHeight_-15, halfw + (n+0.5*skip)*dx, contextHeight_-10);
// 				}
// 			}
// 			painter.drawLine(halfw - i*dx, 10, halfw + i*dx, 10);
// 			painter.drawLine(halfw - i*dx, contextHeight_-10, halfw + i*dx, contextHeight_-10);
// 			for (n = -i; n <= i; n++)
// 			{
// 				if ((n%skip) != 0) continue;
// 				painter.drawText(halfw + n*dx - (n < 0 ? 8 : 3), contextHeight_, QString::number(n));
// 			}
// 			break;
// 		default:
// 			break;
// 	}
}

// Render addition elements related to selected/active UserActions
void Viewer::renderUserActions(Model* source)
{
	Matrix A;
	Atom* j, tempj;
	Vec4<GLfloat> colour_i, colour_j;
	Prefs::DrawStyle style_i, style_j;
	Vec3<double> pos, v;
	Bond::BondType bt = Bond::Single;
	double radius_i, radius_j;
	QString text;

	// Draw on the selection highlights (for atoms in the canvas' pickedAtoms list)
	prefs.copyColour(prefs.currentForegroundColour(), colour_i);
	glColor4f(colour_i.x, colour_i.y, colour_i.z, colour_i.w);
	glEnable(GL_DEPTH_TEST);
	for (RefListItem<Atom,int>* ri = atenWindow_->pickedAtoms(); ri != NULL; ri = ri->next)
	{
		// Get Atom pointer
		Atom* i = ri->item;

		// Get radius information
		style_i = (source->drawStyle() == Prefs::OwnStyle ? i->style() : source->drawStyle());
		radius_i = prefs.atomStyleRadius(style_i);
		if (style_i == Prefs::ScaledStyle) radius_i *= ElementMap::atomicRadius(i->element());

		// Create matrix
		A.createTranslation(i->r());
		A.applyScaling(radius_i, radius_i, radius_i);

		// Draw arrow indicator primitive
		renderGroup_.addTriangles(primitives_[primitiveSet_].pickedAtom(), A, colour_i);
	}

	// Grab the clicked atom
	Atom* clickedAtom = atenWindow_->clickedAtom();

	// Active user actions
	switch (atenWindow_->activeMode())
	{
		// Draw on bond and new atom for chain drawing (if mode is active)
		case (UserAction::DrawAtomsAction):
			if (clickedAtom == NULL) break;
			pos = clickedAtom->r();
			style_i = (source->drawStyle() == Prefs::OwnStyle ? clickedAtom->style() : source->drawStyle());
			if (style_i == Prefs::TubeStyle) radius_i = 0.0;
			else if (style_i == Prefs::ScaledStyle) radius_i = source->styleRadius(Prefs::ScaledStyle, clickedAtom->element()) - primitives_[primitiveSet_].scaledAtomAdjustment(clickedAtom->element());
			else radius_i = source->styleRadius(style_i, clickedAtom->element()) - primitives_[primitiveSet_].sphereAtomAdjustment();

			// We need to project a point from the mouse position onto the canvas plane, unless the mouse is over an existing atom in which case we snap to its position instead
			j = source->atomOnScreen(rMouseLast_.x, contextHeight_ - rMouseLast_.y);
			if (j == NULL)
			{
				j = &tempj;
				v = source->screenToModel(rMouseLast_.x, rMouseLast_.y, atenWindow_->currentDrawDepth());
				j->r() = v;
				style_j = (source->drawStyle() == Prefs::OwnStyle ? Prefs::LineStyle : source->drawStyle());
				j->setStyle(style_j);
			}
			else
			{
				v = j->r();
				style_j = (source->drawStyle() == Prefs::OwnStyle ? j->style() : source->drawStyle());
			}
			if (style_j == Prefs::TubeStyle) radius_j = 0.0;
			else if (style_j == Prefs::ScaledStyle) radius_j = source->styleRadius(Prefs::ScaledStyle, j->element()) - primitives_[primitiveSet_].scaledAtomAdjustment(atenWindow_->currentBuildElement());
			else radius_j = source->styleRadius(style_j, j->element()) - primitives_[primitiveSet_].sphereAtomAdjustment();
			v -= pos;

			// Select colour
			if (clickedAtom->isPositionFixed()) prefs.copyColour(Prefs::FixedAtomColour, colour_i);
			else switch (source->colourScheme())
			{
				case (Prefs::ElementScheme):
					ElementMap::copyColour(clickedAtom->element(), colour_i);
					break;
				case (Prefs::ChargeScheme):
					prefs.colourScale[0].colour(clickedAtom->charge(), colour_i);
					break;
				case (Prefs::VelocityScheme):
					prefs.colourScale[1].colour(clickedAtom->v().magnitude(), colour_i);
					break;
				case (Prefs::ForceScheme):
					prefs.colourScale[2].colour(clickedAtom->f().magnitude(), colour_i);
					break;
				case (Prefs::BondsScheme):
					prefs.colourScale[3].colour(clickedAtom->nBonds(), colour_i);
					break;
				case (Prefs::OwnScheme):
					clickedAtom->copyColour(colour_i);
					break;
				default:
					break;
			}
			ElementMap::copyColour(atenWindow_->currentBuildElement(), colour_j);
			
			// Construct transformation matrix to centre on original (first) atom
			A.createTranslation(pos);
			
			// Render new (temporary) bond
			renderGroup_.createBond(primitives_[primitiveSet_], A, v, clickedAtom, style_i, colour_i, radius_i, j, style_j, colour_j, radius_j, bt, prefs.selectionScale(), NULL);
			
			// Draw text showing distance
			renderGroup_.addOverlayText(QString("r = %1 %2").arg(v.magnitude()).arg(QChar(0x212b)), v+pos, 0.2);
			break;
		// Draw on selection bond for bond deletion click-drag
		case (UserAction::DrawDeleteAction):
			if (clickedAtom == NULL) break;
			pos = clickedAtom->r();
			style_i = (source->drawStyle() == Prefs::OwnStyle ? clickedAtom->style() : source->drawStyle());
			if (style_i == Prefs::TubeStyle) radius_i = 0.0;
			else if (style_i == Prefs::ScaledStyle) radius_i = source->styleRadius(Prefs::ScaledStyle, clickedAtom->element()) - primitives_[primitiveSet_].scaledAtomAdjustment(clickedAtom->element());
			else radius_i = source->styleRadius(style_i, clickedAtom->element()) - primitives_[primitiveSet_].sphereAtomAdjustment();

			// If the mouse isn't currently over an atom, don't draw anything
			// We need to project a point from the mouse position onto the canvas plane, unless the mouse is over an existing atom in which case we snap to its position instead
			j = source->atomOnScreen(rMouseLast_.x, contextHeight_ - rMouseLast_.y);
			if (j && (j != clickedAtom))
			{
				Bond* b = clickedAtom->findBond(j);
				if (!b) break;

				colour_i.set(0.0, 0.0, 0.0, 1.0);
				v = j->r();
				style_j = (source->drawStyle() == Prefs::OwnStyle ? j->style() : source->drawStyle());
				if (style_j == Prefs::TubeStyle) radius_j = 0.0;
				else if (style_j == Prefs::ScaledStyle) radius_j = source->styleRadius(Prefs::ScaledStyle, j->element()) - primitives_[primitiveSet_].scaledAtomAdjustment(atenWindow_->currentBuildElement());
				else radius_j = source->styleRadius(style_j, j->element()) - primitives_[primitiveSet_].sphereAtomAdjustment();
				v -= pos;
				
				// Construct transformation matrix to centre on original (first) atom
				A.createTranslation(pos);
				
				// Render new (temporary) bond
				renderGroup_.createSelectedBond(primitives_[primitiveSet_], A, v, clickedAtom, style_i, colour_i, radius_i, j, style_j, colour_i, radius_j, b->type(), 1.0, NULL);
				
				// Draw text indicating deletion
				renderGroup_.addOverlayText("DEL", v, 0.2);
			}
			break;
		default:
			break;
	}
	
	// Selected user mode actions
	switch (atenWindow_->selectedMode())
	{
		// Draw on fragment (as long as mode is selected)
		case (UserAction::DrawFragmentsAction):
			if (!aten_->currentFragment()) break;
			j = source->atomOnScreen(rMouseLast_.x, contextHeight_ - rMouseLast_.y);
			if ((clickedAtom != NULL) || (j != NULL))
			{
				// Atom is now fragment anchor point - make sure we select a non-null atom i or j
				if (clickedAtom != NULL) j = clickedAtom;
				pos = j->r();
				Model* m = aten_->currentFragment()->anchoredModel(j, keyModifier(Prefs::ShiftKey), aten_->fragmentBondId());
				A.createTranslation(pos);

				// Did we find a valid anchor point?
				if (m != NULL) renderGroup_.createAtomsAndBonds(primitives_[primitiveSet_], m, A);
				else
				{
					prefs.copyColour(prefs.currentForegroundColour(), colour_i);
					renderGroup_.addLines(primitives_[primitiveSet_].crossedCube(), A, colour_i, true);
				}
			}
			else
			{
				// No atom under the mouse pointer, so draw on at the prefs drawing depth in its current orientation
				// Get drawing point origin, translate to it, and render the stored model
				if (atenWindow_->activeMode() == UserAction::DrawFragmentsAction) pos = source->screenToModel(rMouseDown_.x, rMouseDown_.y, prefs.drawDepth());
				else pos = source->screenToModel(rMouseLast_.x, rMouseLast_.y, prefs.drawDepth());
				A.createTranslation(pos);
				renderGroup_.createAtomsAndBonds(primitives_[primitiveSet_], aten_->currentFragment()->orientedModel(), A);
			}
			break;
		default:
			break;
	}
}
