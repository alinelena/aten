/*
	*** Basic atom
	*** src/classes/atom.h
	Copyright T. Youngs 2007

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

#ifndef H_ATOM_H
#define H_ATOM_H

#include "classes/atomtype.h"
#include "templates/vector3.h"
#include "templates/vector4.h"
#include "templates/reflist.h"
#include "base/constants.h"
#ifdef IS_MAC
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif	

// Atom drawing styles
enum draw_style { DS_STICK, DS_TUBE, DS_SPHERE, DS_SCALED, DS_INDIVIDUAL, DS_NITEMS };
inline draw_style operator++(draw_style &style,int)
{
	if (style == DS_INDIVIDUAL) return style = DS_STICK;
	else return style = (draw_style)(style + 1);
}
draw_style DS_from_text(const char*);
const char *text_from_DS(draw_style);
const char **get_DS_strings();

// Atom labels
enum atom_label { AL_ID=1, AL_ELEMENT=2, AL_FFTYPE=4, AL_FFEQUIV=8, AL_CHARGE=16, AL_NITEMS=5 };
atom_label AL_from_text(const char*);

// Hydrogen-add geometries
enum hadd_geom { HG_LINEAR, HG_PLANAR, HG_TETRAHEDRAL };

// Data items in atom structure
enum atom_data { AD_ALL=0, AD_R=1, AD_F=2, AD_V=4, AD_Q=8, AD_FIXFREE=16, AD_Z = 32 };

// Forward declarations
class model;
class bond;
class ffatom;

// Base Atom Data
class atom
{
	public:
	// Constructor / Destructor
	atom();	
	~atom();
	// List pointers
	atom *prev,*next;
	// Get next selected atom in list
	atom *get_next_selected();
	// Add bound neighbours to reflist specified
	void add_bound_to_reflist(reflist<atom>*);
	// Reset all data items in structure
	void reset();
	// Copy atom data from supplied atom
	void copy(atom*);
	// Copy style data (no q, r, f, or v) from supplied atom
	void copy_style(atom*);
	// Print out all info about the atom
	void print();
	// One-line atom summary
	void print_summary();

	/*
	// Coordinates
	*/
	protected:
	vec3<double> rr;
	public:
	vec3<double> &r() { return rr; }

	/*
	// Forces
	*/
	protected:
	vec3<double> ff;
	public:
	vec3<double> &f() { return ff; }

	/*
	// Velocities
	*/
	protected:
	vec3<double> vv;
	public:
	vec3<double> &v() { return vv; }

	/*
	// Character
	*/
	protected:
	// Atomic charge
	double q;
	// Element number
	short int el;
	// Oxidation state (used by typing routines)
	short int os;
	// Forcefield atom type
	ffatom *fftype;
	// Whether the assigned forcefield type is fixed
	bool fftypefixed;
	// Chemical environment of atom
	atom_env env;

	public:
	// Sets the atom charge
	void set_charge(double d) { q = d; }
	// Return the atom charge
	double get_charge() { return q; }
	// Set the element type of the atom
	void set_element(short int newel) { el = newel; }
	// Return the element of the atom
	short int get_element() { return el; }
	// Check element against the supplied value
	bool is_element(short int n) { return (n == el ? TRUE : FALSE); }
	// Check oxidation state against supplied value
	bool is_os(short int n) { return (n == os ? TRUE : FALSE); }
	// Return the oxidation state of the atom
	short int get_os() { return os; }
	// Set the forcefield type of the atom
	void set_fftype(ffatom *ffa) { fftype = ffa; }
	// Return the forcefield type of the atom
	ffatom *get_fftype() { return fftype; }
	// Set the fixed status of the assigned atom type
	void set_fftypefixed(bool b) { fftypefixed = b; }
	// Return the fixed status of the assigned atom type
	bool get_fftypefixed() { return fftypefixed; }
	// Check the ff type of the atom against the supplied value
	bool fftype_is(ffatom *type) { return (fftype == type ? TRUE : FALSE); }
	// Set the environment of the atom
	void set_env(atom_env ae) { env = ae; }
	// Return the environment of the atom
	atom_env get_env() { return env; }
	// Check the environment of the atom against the supplied value
	bool is_env(atom_env ae) { return (env == ae ? TRUE : FALSE); }
	// Whether the atom will be moved in minimisations etc.
	bool fixed;

	/*
	// Bonds / Bonding
	*/
	protected:
	// Bond list for atom
	reflist<bond> bonds;

	public:
	// Return the number of bonds to the atom
	int get_nbonds() { return bonds.size(); }
	// Return the current bond list
	refitem<bond> *get_bonds() { return bonds.first(); }
	// Check the number of bonds against the supplied value
	bool is_nbonds(int n) { return (bonds.size() == n ? TRUE : FALSE); }
	// Accept the specified bond to the atom's local reference list
	void accept_bond(bond *b) { bonds.add(b,0,0); }
	// Delete the specified bond from the atom's local reference list
	void detach_bond(bond*);
	// Return the total bond order of the atom
	int total_bond_order();
	// Return the number of bonds of specified type to the atom
	int count_bonds(bond_type);
	// Calculate the bond order between this atom and the specified atom
	double get_bond_order(atom*);
	// Calculates the geometry of the atom's bound environment
	atom_geom get_geometry(model*);
	// Returns bond pointer between this and atom 'j' (if it exists)
	bond *find_bond(atom*);
	// Determine bond plane
	vec3<double> find_bond_plane(atom*, bond*, const vec3<double>&);

	/*
	// Selection
	*/
	private:
	// Selection flag
	bool selected;	
	// Hidden flag
	bool hidden;

	public:
	// Sets the selected flag of the atom
	void set_selected(bool b) { selected = b; }
	// Returns the current selection state of the atom
	bool is_selected() { return selected; }
	// Sets the hidden flag of the atom
	void set_hidden(bool b) { hidden = b; }
	// Return whether the atom is hidden
	bool is_hidden() { return hidden; }

	/*
	// Misc
	*/
	private:
	// ID number of atom
	short int id;

	public:
	// Temporary integer variable
	short int tempi;
	// Sets the atom id
	void set_id(short int number) { id = number; }
	// Decreases the id of the atom by 1
	void decrease_id() { id --; }
	// Return the id of the atom
	short int get_id() { return id; }

	/*
	// Rendering Coordinates
	*/
	private:
	// 2D coordinates (screen) and 2D depth
	vec3<double> rr_screen;
	// Screen radius for selection
	double screenrad;
	// World (GL) coordinates, transformed by camera and rotation matrices
	vec3<double> rr_world;

	public:
	// World (GL Transformed) coordinates
	vec3<double> &worldr() { return rr_world; }
	// Screen (two-dimensional) coordinates
	vec3<double> &screenr() { return rr_screen; }
	void set_screen_radius(double a) { screenrad = a; }
	double get_screen_radius() { return screenrad; }

	/*
	// Rendering
	*/
	protected:
	// How to draw this atom (and its associated bonds)
	draw_style style;
	// Bitvector for atom labelling
	short int labels;

	public:
	// Sets the drawing style of the atom
	void set_style(draw_style ds) { style = ds; }
	// Returns the drawing style of the atom
	draw_style get_style() { return style; }
	// Returns TRUE id the atom has at least one label specified
	bool has_labels() { return (labels == 0 ? FALSE : TRUE); }
	// Set label bitvector to specified value
	void set_labels(int l) { labels = l; }
	// Returns the label bitmask of the atom
	int get_labels() { return labels; }
	// Set the bit for the specified label (if it is not set already)
	void add_label(atom_label al) { if (!(labels&al)) labels += al; }
	// Unsets the bit for the specified label (if it is not unset already)
	void remove_label(atom_label al) { if (labels&al) labels -= al; }
	// Clear all labels from the atom
	void clear_labels() { labels = 0; }
};

#endif
