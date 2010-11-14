/*
	*** Rendering Primitive
	*** src/render/primitive.cpp
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

#include "render/primitive.h"
#include "base/messenger.h"
#include "base/constants.h"
#include "classes/prefs.h"
#include <stdio.h>
#include <math.h>

// Constructor
Primitive::Primitive()
{
	vertices_ = NULL;
	normals_ = NULL;
	centroids_ = NULL;
	maxVertices_ = 0;
	nDefinedVertices_ = 0;
	type_ = GL_TRIANGLES;
	verticesPerType_ = 3;
	nDefinedTypes_ = 0;
}

// Destructor
Primitive::~Primitive()
{
	clear();
}

// Clear existing data
void Primitive::clear()
{
    // OPTIMIZE - Combine vertex and normal data into single interleaved array
	if (vertices_ != NULL) delete[] vertices_;
	vertices_ = NULL;
	if (normals_ != NULL) delete[] normals_;
	normals_ = NULL;
	if (centroids_ != NULL) delete[] centroids_;
	centroids_ = NULL;
}

// Forget all data, leaving arrays intact
void Primitive::forgetAll()
{
	nDefinedVertices_ = 0;
}

// Create empty data arrays, setting type specified
void Primitive::createEmpty(GLenum type, int ntype)
{
	// Clear old data, if any
	clear();
	type_ = type;
	nType_ = ntype;
	if (type_ == GL_LINES) verticesPerType_ = 2;
	else verticesPerType_ = 3;
	maxVertices_ = nType_*verticesPerType_;
	nDefinedVertices_ = 0;
	nDefinedTypes_ = 0;
	vertices_ = new GLfloat[maxVertices_*3];
	normals_ = new GLfloat[maxVertices_*3];
	centroids_ = new GLfloat[nType_*3];
	for (int n=0; n<nType_*3; ++n) centroids_[n] = 0.0f;
}

// Define next vertex and normal
void Primitive::addVertexAndNormal(GLfloat x, GLfloat y, GLfloat z, GLfloat nx, GLfloat ny, GLfloat nz, bool calcCentroid)
{
	if (nDefinedVertices_ == maxVertices_) printf("Internal Error: Vertex limit for primitive reached.\n");
	// Define next vertex in primitive
	vertices_[nDefinedVertices_*3] = x;
	vertices_[nDefinedVertices_*3+1] = y;
	vertices_[nDefinedVertices_*3+2] = z;
	// Define normal
	normals_[nDefinedVertices_*3] = nx;
	normals_[nDefinedVertices_*3+1] = ny;
	normals_[nDefinedVertices_*3+2] = nz;
	// Accumulate centroid
	if (calcCentroid)
	{
		centroids_[nDefinedTypes_*3] += x;
		centroids_[nDefinedTypes_*3+1] += y;
		centroids_[nDefinedTypes_*3+2] += z;
	}
	// Increase vertex counter
	++nDefinedVertices_;
	// Increase type counter
	if ((nDefinedVertices_%verticesPerType_) == 0)
	{
		// Finalise centroid 
		if (calcCentroid)
		{
			centroids_[nDefinedTypes_*3] /= verticesPerType_;
			centroids_[nDefinedTypes_*3+1] /= verticesPerType_;
			centroids_[nDefinedTypes_*3+2] /= verticesPerType_;
		}
		++nDefinedTypes_;
	}
}

// Create vertices of sphere with specified radius and quality
void Primitive::createSphere(double radius, int nstacks, int nslices)
{
	msg.enter("Primitive::createSphere");
	int i, j, count;
	double stack0, stack1, z0, zr0, z1, zr1, slice0, slice1, x0, y0, x1, y1;

	// Clear existing data first (if it exists)
	createEmpty(GL_TRIANGLES, nstacks*nslices*2);

	count = 0;
	for (i = 0; i < nstacks; i++)
	{
		stack0 = PI * (-0.5 + (double) i / nstacks);
		z0  = sin(stack0);
		zr0 = cos(stack0);

		stack1 = PI * (-0.5 + (double) (i+1) / nstacks);
		z1 = sin(stack1);
		zr1 = cos(stack1);

		for (j = 0; j < nslices; j++)
		{
			slice0 = 2 * PI * (double) j / nslices;
			x0 = cos(slice0);
			y0 = sin(slice0);

			slice1 = 2 * PI * (double) (j+1) / nslices;
			x1 = cos(slice1);
			y1 = sin(slice1);

			// First triangle - {x0,y0,z0},{x0,y0,z1},{x1,y1,z0}
			addVertexAndNormal(x0 * zr0 * radius, y0 * zr0 * radius, z0 * radius, x0 * zr0, y0 * zr0, z0);
			addVertexAndNormal(x0 * zr1 * radius, y0 * zr1 * radius, z1 * radius, x0 * zr1, y0 * zr1, z1);
			addVertexAndNormal(x1 * zr0 * radius, y1 * zr0 * radius, z0 * radius, x1 * zr0, y1 * zr0, z0);

			// Second triangle - {x0,y0,z0},{x0,y0,z1},{x1,y1,z0}
			addVertexAndNormal(x0 * zr1 * radius, y0 * zr1 * radius, z1 * radius, x0 * zr1, y0 * zr1, z1);
			addVertexAndNormal(x1 * zr0 * radius, y1 * zr0 * radius, z0 * radius, x1 * zr0, y1 * zr0, z0);
			addVertexAndNormal(x1 * zr1 * radius, y1 * zr1 * radius, z1 * radius, x1 * zr1, y1 * zr1, z1);
		}
	}
	msg.exit("Primitive::createSphere");
}

// Create vertices of cylinder with specified radii, length, and quality
void Primitive::createCylinder(double startradius, double endradius, double length, int nstacks, int nslices)
{
	msg.enter("Primitive::createCylinder");
	int i, j, count;
	double stack0, stack1, z0, zr0, z1, zr1, slice0, slice1, x0, y0, x1, y1, deltaz, deltar;

	// Clear existing data first (if it exists)
	createEmpty(GL_TRIANGLES, nstacks*nslices*2);

	count = 0;
	deltaz = length / nstacks;
	deltar = (endradius-startradius) / nstacks;
	for (i = 0; i < nstacks; i++)
	{
		stack0 = PI * (-0.5 + (double) i / nstacks);
		z0  = i*deltaz;
		zr0 = startradius - i*deltar;

		stack1 = PI * (-0.5 + (double) (i+1) / nstacks);
		z1 = (i+1)*deltaz;
		zr1 = startradius - (i+1)*deltar;;

		for (j = 0; j < nslices; j++)
		{
			slice0 = 2 * PI * (double) j / nslices;
			x0 = cos(slice0);
			y0 = sin(slice0);

			slice1 = 2 * PI * (double) (j+1) / nslices;
			x1 = cos(slice1);
			y1 = sin(slice1);

			// First triangle - {x0,y0,z0},{x0,y0,z1},{x1,y1,z0}
			addVertexAndNormal(x0 * zr0, y0 * zr0, z0, x0 * zr0, y0 * zr0, 0.0);
			addVertexAndNormal(x0 * zr1, y0 * zr1, z1, x0 * zr1, y0 * zr1, 0.0);
			addVertexAndNormal(x1 * zr0, y1 * zr0, z0, x1 * zr0, y1 * zr0, 0.0);

			// Second triangle - {x0,y0,z0},{x0,y0,z1},{x1,y1,z0}
			addVertexAndNormal(x0 * zr1, y0 * zr1, z1, x0 * zr1, y0 * zr1, 0.0);
			addVertexAndNormal(x1 * zr0, y1 * zr0, z0, x1 * zr0, y1 * zr0, 0.0);
			addVertexAndNormal(x1 * zr1, y1 * zr1, z1, x1 * zr1, y1 * zr1, 0.0);
		}
	}
	msg.exit("Primitive::createCylinder");
}

// Create vertices of cross with specified width
void Primitive::createCross(double width, int naxes)
{
	int i,j,count, limit = max(1,naxes);
	// Clear existing data first (if it exists)
	clear();

	maxVertices_ = 2*limit;
	vertices_ = new GLfloat[maxVertices_*3];
	normals_ = new GLfloat[maxVertices_*3];
	type_ = GL_LINES;
	count = 0;

	for (i=0; i<limit; ++i)
	{
		for (j=0; j<3; ++j)
		{
			vertices_[count] = (i == j ? width : 0.0);
			normals_[count++] = (j == 0 ? 1.0 : 0.0);
		}
		for (j=0; j<3; ++j)
		{
			vertices_[count] = (i == j ? -width : 0.0);
			normals_[count++] = (j == 0 ? 1.0 : 0.0);
		}
	}
}

// Return vertex array
GLfloat *Primitive::vertices()
{
	return vertices_;
}

// Return normal array
GLfloat *Primitive::normals()
{
	return normals_;
}

// Return centroids array
GLfloat *Primitive::centroids()
{
	return centroids_;
}

// Return number of vertices defined
int Primitive::nDefinedVertices()
{
	return nDefinedVertices_;
}

// Return number of primitive types defined
int Primitive::nDefinedTypes()
{
	return nDefinedTypes_;
}

// Return whether all arrays are full
bool Primitive::full()
{
	return (nDefinedVertices_ == maxVertices_);
}

// Send to OpenGL (i.e. render)
void Primitive::sendToGL()
{
	if (nDefinedVertices_ == 0) return;
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices_);
	glNormalPointer(GL_FLOAT, 0, normals_);
	glDrawArrays(type_, 0, nDefinedVertices_);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

/*
// Primitive Info
*/

// Constructor
PrimitiveInfo::PrimitiveInfo()
{
	// Private variables
	primitive_ = NULL;
	matrixTransformDefined_ = FALSE;

	// Public variables
	prev = NULL;
	next = NULL;
}

// Set primitive info data
void PrimitiveInfo::set(Primitive *prim, GLfloat *ambient, GLfloat *diffuse, Vec3<double> &coords)
{
	primitive_ = prim;
	localCoords_ = coords;
	for (int n=0; n<4; ++n)
	{
		ambient_[n] = ambient[n];
		diffuse_[n] = diffuse[n];
	}
}

// Set primitive info data, including local rotation
void PrimitiveInfo::set(Primitive *prim, GLfloat *ambient, GLfloat *diffuse, GLMatrix &transform)
{
	primitive_ = prim;
	localTransform_ = transform;
	matrixTransformDefined_ = TRUE;
	for (int n=0; n<4; ++n)
	{
		ambient_[n] = ambient[n];
		diffuse_[n] = diffuse[n];
	}
}

// Return pointer to primitive
Primitive *PrimitiveInfo::primitive()
{
	return primitive_;
}

// Return local coordinates of primitive
Vec3<double> &PrimitiveInfo::localCoords()
{
	return localCoords_;
}

// Return local transformation of primitive
GLMatrix &PrimitiveInfo::localTransform()
{
	return localTransform_;
}

// Return whether a local transformation is defined
bool PrimitiveInfo::matrixTransformDefined()
{
	return matrixTransformDefined_;
}

// Return ambient colour pointer
GLfloat *PrimitiveInfo::ambient()
{
	return ambient_;
}

// Return diffuse colour pointer
GLfloat *PrimitiveInfo::diffuse()
{
	return diffuse_;
}

/*
// Primitive Group
*/

// Constructor
PrimitiveGroup::PrimitiveGroup()
{
	primitives_ = NULL;
	nPrimitives_ = 0;

	clear();
}

PrimitiveGroup::~PrimitiveGroup()
{
	if (primitives_ != NULL) delete[] primitives_;
	nPrimitives_ = 0;
}

// Clear old primitives array and allocate new one
void PrimitiveGroup::clear()
{
	if (primitives_ != NULL) delete[] primitives_;
	nPrimitives_ = prefs.levelsOfDetail();
	primitives_ = new Primitive[nPrimitives_];
}

// Return primitive corresponding to level of detail specified
Primitive &PrimitiveGroup::primitive(int lod)
{
	// Clamp LOD to allowable range
	if (lod < 0) return primitives_[0];
	else if (lod >= nPrimitives_) return primitives_[nPrimitives_-1];
	else return primitives_[lod];
}

// Send to OpenGL (i.e. render) at specified level of detail
void PrimitiveGroup::sendToGL(int lod)
{
	// Clamp lod to allowable range for this PrimitiveGroup
	if (lod < 0) lod = 0;
	else if (lod >= nPrimitives_) lod = nPrimitives_-1;
	primitives_[lod].sendToGL();
}