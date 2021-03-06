/*
	*** Render Occurrence
	*** src/render/renderoccurrence.cpp
	Copyright T. Youngs 2013-2016

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

#include "render/renderoccurrence.h"
#include "render/primitive.h"
#include "math/matrix.h"

ATEN_USING_NAMESPACE

// Constructor
RenderOccurrence::RenderOccurrence(Primitive& targetPrimitive, int initialChunkSize) : ListItem<RenderOccurrence>(), primitive_(targetPrimitive)
{
	currentChunk_ = NULL;
	chunkSize_ = initialChunkSize;
}

/*
 * Primitive Data
 */

// Add chunk (with current chunkSize_)
RenderOccurrenceChunk* RenderOccurrence::addChunk()
{
	RenderOccurrenceChunk* newChunk = new RenderOccurrenceChunk(chunkSize_);
	chunks_.own(newChunk);

	return newChunk;
}

// Clear data (retaining arrays) adjusting chunkSize_ if necessary
void RenderOccurrence::clear(int newChunkSize)
{
	// If chunkSize_ has changed, delete all old arrays
	if (chunkSize_ != newChunkSize)
	{
		Messenger::print(Messenger::Verbose, "Setting new chunkSize to %i\n", newChunkSize);

		// Remove all old chunks
		chunks_.clear();
		chunkSize_ = newChunkSize;
		currentChunk_ = addChunk();
	}
	else
	{
		// Loop over chunks and clear data
		for (RenderOccurrenceChunk* chunk = chunks_.first(); chunk != NULL; chunk = chunk->next) chunk->clear();
	}
	currentChunk_ = chunks_.first();
}

// Return target primitive
Primitive& RenderOccurrence::primitive() const
{
	return primitive_;
}

// Add occurrence
void RenderOccurrence::addOccurrence(Matrix& transform, Vec4<GLfloat>& colour)
{
	// Check current chunk...
	if (currentChunk_ == NULL) currentChunk_ = addChunk();
	else if (currentChunk_->isFull())
	{
		if (currentChunk_->next) currentChunk_ = currentChunk_->next;
		else currentChunk_ = addChunk();
	}

	// Add to chunk...
	currentChunk_->setNextData(transform, colour);
}

// Add occurrence (with no colour)
void RenderOccurrence::addOccurrence(Matrix& transform)
{
	// Check current chunk...
	if (currentChunk_ == NULL) currentChunk_ = addChunk();
	else if (currentChunk_->isFull())
	{
		if (currentChunk_->next) currentChunk_ = currentChunk_->next;
		else currentChunk_ = addChunk();
	}

	// Add to chunk...
	currentChunk_->setNextData(transform);
}

/*
 * GL
 */

// Send to GL
void RenderOccurrence::sendToGL(Matrix& modelTransformationMatrix)
{
	// Setup primitive for rendering...
	QOpenGLFunctions* glFunctions = QOpenGLContext::currentContext()->functions();
	if (!primitive_.beginGL(glFunctions)) return;

	// Render based on instance type
	if (primitive_.instanceType() == PrimitiveInstance::VBOInstance)
	{
		// Loop over chunks
		for (RenderOccurrenceChunk* chunk = chunks_.first(); chunk != NULL; chunk = chunk->next)
		{
			// If there are no defined items in this chunk, may as well break early...
			if (chunk->nDefined() == 0) break;

			// Grab arrays from chunk...
			const Matrix* localTransforms = chunk->localTransform();

			// Loop over occurrences in chunk
			Matrix A;
			if (primitive_.colouredVertexData())
			{
				// Draw primitives
				for (int n=0; n<chunk->nDefined(); ++n)
				{
					A = modelTransformationMatrix * localTransforms[n];
					glLoadMatrixd(A.matrix());
					primitive_.sendVBO();
				}
			}
			else
			{
				// Grab colour arrays
				const GLfloat* r = chunk->red();
				const GLfloat* g = chunk->green();
				const GLfloat* b = chunk->blue();
				const GLfloat* a = chunk->alpha();

				// Draw primitives
				for (int n=0; n<chunk->nDefined(); ++n)
				{
					glColor4f(r[n], g[n], b[n], a[n]);
					A = modelTransformationMatrix * localTransforms[n];
					glLoadMatrixd(A.matrix());
					primitive_.sendVBO();
				}
			}
		}
	}
	else if (primitive_.instanceType() == PrimitiveInstance::ListInstance)
	{
		// Grab topmost instance
		PrimitiveInstance* instance = primitive_.lastInstance();
		GLuint listID = instance->listObject();

		// Loop over chunks
		for (RenderOccurrenceChunk* chunk = chunks_.first(); chunk != NULL; chunk = chunk->next)
		{
			// If there are no defined items in this chunk, may as well break early...
			if (chunk->nDefined() == 0) break;

			// Grab arrays from chunk...
			const Matrix* localTransforms = chunk->localTransform();

			// Loop over occurrences in chunk
			Matrix A;
			if (primitive_.colouredVertexData())
			{
				// Draw primitives
				for (int n=0; n<chunk->nDefined(); ++n)
				{
					A = modelTransformationMatrix * localTransforms[n];
					glLoadMatrixd(A.matrix());
					glCallList(listID);
				}
			}
			else
			{
				// Grab colour arrays
				const GLfloat* r = chunk->red();
				const GLfloat* g = chunk->green();
				const GLfloat* b = chunk->blue();
				const GLfloat* a = chunk->alpha();

				// Draw primitives
				for (int n=0; n<chunk->nDefined(); ++n)
				{
					glColor4f(r[n], g[n], b[n], a[n]);
					A = modelTransformationMatrix * localTransforms[n];
					glLoadMatrixd(A.matrix());
					glCallList(listID);
				}
			}
		}
	}
	else
	{
		// Loop over chunks
		for (RenderOccurrenceChunk* chunk = chunks_.first(); chunk != NULL; chunk = chunk->next)
		{
			// If there are no defined items in this chunk, may as well break early...
			if (chunk->nDefined() == 0) break;

			// Grab arrays from chunk...
			const Matrix* localTransforms = chunk->localTransform();

			// Loop over occurrences in chunk
			Matrix A;
			if (primitive_.colouredVertexData())
			{
				// Draw primitives
				for (int n=0; n<chunk->nDefined(); ++n)
				{
					A = modelTransformationMatrix * localTransforms[n];
					glLoadMatrixd(A.matrix());
					primitive_.sendToGL(QOpenGLContext::currentContext());
				}
			}
			else
			{
				// Grab colour arrays
				const GLfloat* r = chunk->red();
				const GLfloat* g = chunk->green();
				const GLfloat* b = chunk->blue();
				const GLfloat* a = chunk->alpha();

				// Draw primitives
				for (int n=0; n<chunk->nDefined(); ++n)
				{
					glColor4f(r[n], g[n], b[n], a[n]);
					A = modelTransformationMatrix * localTransforms[n];
					glLoadMatrixd(A.matrix());
					primitive_.sendToGL(QOpenGLContext::currentContext());
				}
			}
		}
	}

	// Done with primitive
	primitive_.finishGL(glFunctions);
}
