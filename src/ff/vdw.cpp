/*
	*** van der Waals energy / force calculation
	*** src/ff/vdw.cpp
	Copyright T. Youngs 2007,2008

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

#include "classes/pattern.h"
#include "classes/forcefield.h"
#include "model/model.h"
#include "templates/vector3.h"
#include "classes/energystore.h"
#include "base/prefs.h"
#include <math.h>

// Calculate energy for specified interaction
double VdwEnergy(VdwFunctions::VdwFunction type, double rij, ForcefieldParams paramsi, ForcefieldParams paramsj, double scale, int i, int j)
{
	static double U, epsilon, sigma, sigmar2, sigmar6, r6, ar12, br6, pwr, a, b, c, d, forcek, eq, expo;
	switch (type)
	{
		case (VdwFunctions::None):
			msg.print( "Warning: No function is specified for vdW energy %i-%i.\n", i, j);
			U = 0.0;
			break;
		case (VdwFunctions::InversePower):
			// U = epsilon * (r / rij) ** n
			epsilon = sqrt( paramsi.data[VdwFunctions::InversePowerEpsilon] * paramsj.data[VdwFunctions::InversePowerEpsilon] );
			sigma = ( paramsi.data[VdwFunctions::InversePowerR] + paramsj.data[VdwFunctions::InversePowerR] ) * 0.5 * scale;
			pwr = ( paramsi.data[VdwFunctions::InversePowerN] + paramsj.data[VdwFunctions::InversePowerN] ) * 0.5;
			U = epsilon * pow(sigma / rij, pwr);
			break;
		case (VdwFunctions::Lj):
			// U = 4 * epsilon * [ (s/rij)**12 - n *(s/rij)**6 ]
			epsilon = sqrt( paramsi.data[VdwFunctions::LjEpsilon] * paramsj.data[VdwFunctions::LjEpsilon] );
			sigma = ( paramsi.data[VdwFunctions::LjSigma] + paramsj.data[VdwFunctions::LjSigma] ) * 0.5 * scale;
			a = ( paramsi.data[VdwFunctions::LjN] + paramsj.data[VdwFunctions::LjN] ) * 0.5;
			sigmar2 = sigma / rij;
			sigmar2 *= sigmar2;
			sigmar6 = sigmar2 * sigmar2 * sigmar2;
			U = 4.0 * epsilon * (sigmar6*sigmar6 - a * sigmar6);
			break;
		case (VdwFunctions::LjAB):
			// U = A/r**12 - B/r**6
			a = sqrt( paramsi.data[VdwFunctions::LjA] * paramsj.data[VdwFunctions::LjA] );
			b = sqrt( paramsi.data[VdwFunctions::LjB] * paramsj.data[VdwFunctions::LjB] );
			r6 = rij * rij * rij;
			r6 *= r6;
			ar12 = a / (r6 * r6);
			br6 = b / r6;
			U = ar12 - br6;
			break;
		case (VdwFunctions::Buckingham):
			// U = A * exp(-rij/B) - C/(rij**6)
			a = sqrt( paramsi.data[VdwFunctions::BuckinghamA] * paramsj.data[VdwFunctions::BuckinghamA] );
			b = sqrt( paramsi.data[VdwFunctions::BuckinghamB] * paramsj.data[VdwFunctions::BuckinghamB] );
			c = sqrt( paramsi.data[VdwFunctions::BuckinghamC] * paramsj.data[VdwFunctions::BuckinghamC] );
			r6 = rij * rij * rij;
			r6 *= r6;
			U = a * exp(-rij/b) - c/r6;
			break;
		case (VdwFunctions::Morse):
			// U = E0 * ( (1 - exp( -k(rij - r0) ) )**2 - 1)
			d = sqrt( paramsi.data[VdwFunctions::MorseD] * paramsj.data[VdwFunctions::MorseD]);
			forcek = sqrt( paramsi.data[VdwFunctions::MorseK] * paramsj.data[VdwFunctions::MorseK] );
			eq = 0.5 * (paramsi.data[VdwFunctions::MorseEq] + paramsj.data[VdwFunctions::MorseEq]) * scale;
			rij -= eq;
			expo = 1.0 - exp( -forcek * rij );
			U = d * ( expo*expo - 1.0);
			break;
	}
	return U;
}

// Calculate forces for specified interaction (return force on atom i)
Vec3<double> VdwForces(VdwFunctions::VdwFunction type, Vec3<double> vecij, double rij, ForcefieldParams paramsi, ForcefieldParams paramsj, double scale, int i, int j)
{
	static double du_dr, epsilon, sigma, sigmar2, sigmar6, r2, r6, ar12, br6, a, b, c, d, pwr, forcek, expo, eq;
	static Vec3<double> fi;
	switch (type)
	{
		case (VdwFunctions::None):
			msg.print( "Warning: No function is specified for vdW forces %i-%i.\n", i, j);
			du_dr = 0.0;
			break;
		case (VdwFunctions::InversePower):
			// dU/dr = -n * epsilon * (sigma / r)**(n-1) * (sigma/ r**2)
			epsilon = sqrt( paramsi.data[VdwFunctions::InversePowerEpsilon] * paramsj.data[VdwFunctions::InversePowerEpsilon] );
			sigma = ( paramsi.data[VdwFunctions::InversePowerR] + paramsj.data[VdwFunctions::InversePowerR] ) * 0.5 * scale;
			pwr = ( paramsi.data[VdwFunctions::InversePowerN] + paramsj.data[VdwFunctions::InversePowerN] ) * 0.5;
			du_dr = -pwr * epsilon * pow(sigma / rij, (pwr - 1.0)) * (sigma / (rij*rij));
			break;
		case (VdwFunctions::Lj):
			// dU/dr = 48 * epsilon * ( sigma/r**13 - 0.5 * sigma/r**7)
			epsilon = sqrt( paramsi.data[VdwFunctions::LjEpsilon] * paramsj.data[VdwFunctions::LjEpsilon] );
			sigma = ( paramsi.data[VdwFunctions::LjSigma] + paramsj.data[VdwFunctions::LjSigma] ) * 0.5 * scale;
			a = ( paramsi.data[VdwFunctions::LjN] + paramsj.data[VdwFunctions::LjN] ) * 0.5;
			sigmar2 = (sigma / rij);
			sigmar2 *= sigmar2;
			sigmar6 = sigmar2 * sigmar2 * sigmar2;
			du_dr = 48.0 * epsilon * sigmar6 * (sigmar6 - 0.5*a) / rij;
			break;
		case (VdwFunctions::LjAB):
			// dU/dr = 6*(B/r**7) - 12*(A/r**13)
			a = sqrt( paramsi.data[VdwFunctions::LjA] * paramsj.data[VdwFunctions::LjA] );
			b = sqrt( paramsi.data[VdwFunctions::LjB] * paramsj.data[VdwFunctions::LjB] );
			r6 = rij * rij * rij;
			r6 *= r6;
			ar12 = a / (r6 * r6);
			br6 = b / r6;
			du_dr = (6.0 * br6 - 12.0 * ar12) / rij;
			break;
		case (VdwFunctions::Buckingham):
			// dU/dr = A * exp(-rij/B) / B + 6.0 * C/(rij**7)
			a = sqrt( paramsi.data[VdwFunctions::BuckinghamA] * paramsj.data[VdwFunctions::BuckinghamA] );
			b = sqrt( paramsi.data[VdwFunctions::BuckinghamB] * paramsj.data[VdwFunctions::BuckinghamB] );
			c = sqrt( paramsi.data[VdwFunctions::BuckinghamC] * paramsj.data[VdwFunctions::BuckinghamC] );
			r2 = rij * rij;
			r6 = r2 * r2 * r2;
			du_dr = a * b * exp(-rij/b) / b + 6.0 * c/(r6 * rij);
			break;
		case (VdwFunctions::Morse):
			// dU/dr = 2.0 * k * E0 * (1 - exp( -k(rij - r0) ) ) * exp( -k*(rij - r0) )
			d = sqrt( paramsi.data[VdwFunctions::MorseD] * paramsj.data[VdwFunctions::MorseD] );
			forcek = sqrt(paramsi.data[VdwFunctions::MorseK] * paramsj.data[VdwFunctions::MorseK] );
			eq = 0.5 * (paramsi.data[VdwFunctions::MorseEq] + paramsj.data[VdwFunctions::MorseEq] ) * scale;
			expo = exp( -forcek * (rij - eq) );
			du_dr = 2.0 * forcek * d * (1.0 - expo) * expo;
			break;
	}
	// Calculate final forces (vecij contains dx, dy, dz between target atoms)
	return -(vecij * du_dr / rij);
}

// Intrapattern VDW energy
void Pattern::vdwIntraPatternEnergy(Model *srcmodel, Energy *estore, int lonemolecule)
{
	// Calculate the internal VDW contributions with coordinates from *xcfg
	// Consider only the intrapattern interactions between atoms in individual molecules within the pattern.
	msg.enter("Pattern::vdwIntraPatternEnergy");
	static int n,aoff,m1,i,j, start1, finish1, con;
	static Vec3<double> mim_i;
	static double U, rij, energy_inter, energy_intra, cutoff, vrs;
	PatternAtom *pai, *paj;
	PatternBound *pb;
	cutoff = prefs.vdwCutoff();
	vrs = prefs.vdwScale();
	Atom **modelatoms = srcmodel->atomArray();
	Cell *cell = srcmodel->cell();
	energy_inter = 0.0;
	energy_intra = 0.0;
	start1 = (lonemolecule == -1 ? 0 : lonemolecule);
	finish1 = (lonemolecule == -1 ? nMolecules_ : lonemolecule+1);
	aoff = startAtom_ + start1*nAtoms_;
	for (m1=start1; m1<finish1; m1++)
	{
		// Calculate energies of atom pairs that are unbound or separated by more than two bonds
		// I.E. bound interactions up to and including angles are excluded. Torsions are scaled by the scale matrix.
		i = -1;
		for (pai = atoms_.first(); pai != atoms_.last(); pai = pai->next)
		{
			i++;
			j = i;
			for (paj = pai->next; paj != NULL; paj = paj->next)
			{
				j++;
				con = conMatrix_[i][j];
				if ((con > 2) || (con == 0))
				{
					mim_i = cell->mimd(modelatoms[i+aoff]->r(), modelatoms[j+aoff]->r());
					rij = mim_i.magnitude();
					if (rij > cutoff) continue;
					// Check for conflicting VDW types
					if (atoms_[i]->data()->vdwForm() != atoms_[j]->data()->vdwForm())
					{
						msg.print( "Conflicting vdW types for atoms %i (%s) and %i (%s).\n", i, VdwFunctions::VdwFunctions[atoms_[i]->data()->vdwForm()].name, j, VdwFunctions::VdwFunctions[atoms_[j]->data()->vdwForm()].name);
						continue;
					}
					// Calculate the enrgy contribution
					U = VdwEnergy(atoms_[i]->data()->vdwForm(), rij, atoms_[i]->data()->params(), atoms_[j]->data()->params(), vrs, i, j);
					con == 0 ? energy_inter += U : energy_intra += (con == 3 ? U * vdwScaleMatrix_[i][j] : U);
				}
			}
		}
		aoff += nAtoms_;
	}
	// Add totals into Energy
	estore->add(Energy::VdwIntraEnergy,energy_intra,id_);
	estore->add(Energy::VdwInterEnergy,energy_inter,id_,id_);
	msg.exit("Pattern::vdwIntraPatternEnergy");
}

// Interpattern VDW energy
void Pattern::vdwInterPatternEnergy(Model *srcmodel, Pattern *otherPattern, Energy *estore, int molId)
{
	// Calculate the VDW contribution to the energy from interactions between molecules of this pattern and the one supplied
	msg.enter("Pattern::vdwInterPatternEnergy");
	static int n1,n2,i,j,aoff1,aoff2,m1,m2,finish1,start1,start2,finish2;
	static Vec3<double> mim_i;
	static double rij, energy_inter, cutoff, vrs, U;
	PatternAtom *pai, *paj;
	cutoff = prefs.vdwCutoff();
	vrs = prefs.vdwScale();
	Atom **modelatoms = srcmodel->atomArray();
	Cell *cell = srcmodel->cell();
	energy_inter = 0.0;
	// Outer loop over molecules in *this* pattern
	// When we are considering the same node with itself, calculate for "m1=1,T-1 m2=2,T"
	if (molId == -1)
	{
		start1 = 0;
		finish1 = (this == otherPattern ? nMolecules_ - 1 : nMolecules_);
	}
	else
	{
		start1 = molId;
		finish1 = molId + 1;
	}
	aoff1 = startAtom_ + start1 * nAtoms_;
	for (m1=start1; m1<finish1; m1++)
	{
		// Inner loop - over *all* molecules in 'otherPattern'
		if (this == otherPattern)
		{
			// Same pattern - if a specific molecule was given then we loop over all molecules.
			// If not, loop over m1+1 to nMolecules_.
			if (molId == -1)
			{
				start2 = m1 + 1;
				finish2 = nMolecules_;
			}
			else
			{
				start2 = 0;
				finish2 = nMolecules_;
			}
		}
		else
		{
			// Simple - go over all molecules in the dissimilar pattern
			start2 = 0;
			finish2 = otherPattern->nMolecules_;
		}

		//if (m1 == 0) printf("IPE - finish1 = %i, start2 = %i, finish2 = %i\n",finish1,start2,finish2);
		aoff2 = otherPattern->startAtom_ + start2*otherPattern->nAtoms_;
		//printf("  VDWINTER2 %i %i %i\n",start2,finish2,aoff2);
		for (m2=start2; m2<finish2; m2++)
		{
			if ((this == otherPattern) && (molId == m2)) { aoff2 += nAtoms_; continue; }
			//printf("      m1/m2=%i/%i  aoff1/aoff2=%i/%i \n",m1,m2,aoff1,aoff2);
			i = -1;
			for (pai = atoms_.first(); pai != NULL; pai = pai->next)
			{
				i++;
				j = -1;
				for (paj = otherPattern->atoms_.first(); paj != NULL; paj = paj->next)
				{
					j++;

					mim_i = cell->mimd(modelatoms[i+aoff1]->r(), modelatoms[j+aoff2]->r());
					rij = mim_i.magnitude();
					if (rij > cutoff) continue;
					// Check for conflicting VDW types
					if (atoms_[i]->data()->vdwForm() != otherPattern->atoms_[j]->data()->vdwForm())
					{
						msg.print( "Conflicting vdW types for atoms %i (%s) and %i (%s).\n", i, VdwFunctions::VdwFunctions[atoms_[i]->data()->vdwForm()].name, j, VdwFunctions::VdwFunctions[otherPattern->atoms_[j]->data()->vdwForm()].name);
						continue;
					}
					// Calculate the enrgy contribution
					U = VdwEnergy(atoms_[i]->data()->vdwForm(), rij, atoms_[i]->data()->params(), otherPattern->atoms_[j]->data()->params(), vrs, i, j);
					energy_inter += U;;
				}
			}
			aoff2 += otherPattern->nAtoms_;
		}
		aoff1 += nAtoms_;
	}
	estore->add(Energy::VdwInterEnergy,energy_inter,id_,otherPattern->id_);
	msg.exit("Pattern::vdwInterPatternEnergy");
}

// Intrapattern VDW forces
void Pattern::vdwIntraPatternForces(Model *srcmodel)
{
	// Calculate the internal VDW contributions with coordinates from *xcfg
	// Consider only the intrapattern interactions between atoms in individual molecules within the pattern.
	// 'aoff' stores the atom number offset (molecule offset) but is *only* used for lookups in the coordinate
	// arrays since assuming the pattern definition is correct then the sigmas/epsilons in molecule 0 represent
	// those of all molecules.
	msg.enter("Pattern::vdwIntraPatternForces");
	static int n,i,j,aoff,m1,con;
	static Vec3<double> mim_i, f_i, tempf;
	static double cutoff, vrs, rij;
	PatternAtom *pai, *paj;
	PatternBound *pb;
	cutoff = prefs.vdwCutoff();
	vrs = prefs.vdwScale();
	Atom **modelatoms = srcmodel->atomArray();
	Cell *cell = srcmodel->cell();
	aoff = startAtom_;
	for (m1=0; m1<nMolecules_; m1++)
	{
		// Add contributions from atom pairs that are unbound or separated by more than three bonds
		i = -1;
		for (pai = atoms_.first(); pai != atoms_.last(); pai = pai->next)
		{
			i++;
			// Store temporary forces to avoid unnecessary array lookups
			f_i = modelatoms[i+aoff]->f();
			j = i;
			for (paj = pai->next; paj != NULL; paj = paj->next)
			{
				j++;
				con = conMatrix_[i][j];
				if ((con > 2) || (con == 0))
				{
					mim_i = cell->mimd(modelatoms[i+aoff]->r(), modelatoms[j+aoff]->r());
					rij = mim_i.magnitude();
					if (rij > cutoff) continue;
					// Check for conflicting VDW types
					if (atoms_[i]->data()->vdwForm() != atoms_[j]->data()->vdwForm())
					{
						msg.print( "Conflicting vdW types for atoms %i (%s) and %i (%s).\n", i, VdwFunctions::VdwFunctions[atoms_[i]->data()->vdwForm()].name, j, VdwFunctions::VdwFunctions[atoms_[j]->data()->vdwForm()].name);
						continue;
					}
					// Calculate force contribution
					tempf = VdwForces(atoms_[i]->data()->vdwForm(), mim_i, rij, pai->data()->params(), paj->data()->params(), vrs, i, j);
					if (con == 3) tempf *= vdwScaleMatrix_[i][j];
					f_i -= tempf;
					modelatoms[j+aoff]->f() += tempf;
				}
			}
			// Put the temporary forces back into the main array
			modelatoms[i+aoff]->f() = f_i;
		}
		aoff += nAtoms_;
	}
	msg.exit("Pattern::vdwIntraPatternForces");
}

// Interpattern VDW forces
void Pattern::vdwInterPatternForces(Model *srcmodel, Pattern *otherPattern)
{
	// Calculate the VDW forces from interactions between different molecules
	// of this pnode and the one supplied
	msg.enter("Pattern::vdwInterPatternForces");
	static int n1,n2,i,j,aoff1,aoff2,m1,m2,start,finish;
	static Vec3<double> mim_i, f_i, tempf;
	static double rij, factor, cutoff, vrs;
	PatternAtom *pai, *paj;
	cutoff = prefs.vdwCutoff();
	vrs = prefs.vdwScale();
	Atom **modelatoms = srcmodel->atomArray();
	Cell *cell = srcmodel->cell();
	aoff1 = startAtom_;
	//printf("Pattern IDs are %i (this) and %i\n",id_, otherPattern->id_);

	// TODO Move loops so that we can load temporary forces for i then calculate all other forces on it in one go.
	 // When we are considering the same node with itself, calculate for "m1=1,T-1 m2=2,T"
        this == otherPattern ? finish = nMolecules_ - 1 : finish = nMolecules_;
	for (m1=0; m1<finish; m1++)
	{
		this == otherPattern ? start = m1 + 1 : start = 0;
		aoff2 = otherPattern->startAtom_ + start*nAtoms_;
		for (m2=start; m2<otherPattern->nMolecules_; m2++)
		{
			i = -1;
			for (pai = atoms_.first(); pai != NULL; pai = pai->next)
			{
				i++;
				f_i = modelatoms[i+aoff1]->f();
				j = -1;
				for (paj = otherPattern->atoms_.first(); paj != NULL; paj = paj->next)
				{
					j++;
					mim_i = cell->mimd(modelatoms[i+aoff1]->r(), modelatoms[j+aoff2]->r());
					rij = mim_i.magnitude();
					if (rij > cutoff) continue;
					// Check for conflicting VDW types
					if (atoms_[i]->data()->vdwForm() != otherPattern->atoms_[j]->data()->vdwForm())
					{
						msg.print( "Conflicting vdW types for atoms %i (%s) and %i (%s).\n", i, VdwFunctions::VdwFunctions[atoms_[i]->data()->vdwForm()].name, j, VdwFunctions::VdwFunctions[otherPattern->atoms_[j]->data()->vdwForm()].name);
						continue;
					}
					// Calculate force contribution
					tempf = VdwForces(atoms_[i]->data()->vdwForm(), mim_i, rij, pai->data()->params(), paj->data()->params(), vrs, i, j);
					f_i -= tempf;
					modelatoms[j+aoff2]->f() += tempf;
				}
				// Store temporary force array back into main force array
				modelatoms[i+aoff1]->f() = f_i;
			}
			aoff2 += otherPattern->nAtoms_;
		}
		aoff1 += nAtoms_;
	}
	msg.exit("Pattern::vdwInterPatternForces");
}

/*
// VDW Long Range Correction to Energy
// Frenkel and Smit, Academic Press, 1996, p32	    // Eq in this edition is wrong
//
//               /inf
// U(lr) = 0.5 * |     dr 4 * pi * r**2 * rho(r) * u(r)
//               /rcut
//
// Assume p(r) is equal to the (bulk) number density at r > rcut.
*/
void Pattern::vdwCorrectEnergy(Cell *cell, Energy *estore)
{
	// Calculate the long-range correction to the VDW energy
	msg.enter("Pattern::vdwCorrectEnergy");
	static int i, j;
	static Pattern *p1, *p2;
	static double energy, rho, cutoff, du_dr, sigma, epsilon, sigmar3, sigmar9, volume, vrs;
	PatternAtom *pai, *paj;
	static ForcefieldParams paramsi, paramsj;
	cutoff = prefs.vdwCutoff();
	vrs = prefs.vdwScale();
	// The way the patterns are stored does not give direct access to the number of different
	// atom types used *or* the number densities of each. So, assume each atom in the pattern 
	// molecule is a unique VDW type and that the number density is nMolecules_/cellvolume
	volume = cell->volume();
	energy = 0.0;
	for (p1 = this; p1 != NULL; p1 = p1->next)
	{
		for (p2 = this; p2 != NULL; p2 = p2->next)
		{
			rho = (p1->nMolecules_ * p2->nMolecules_) /volume;
			i = -1;
			for (pai = p1->atoms_.first(); pai != NULL; pai = pai->next)
			{
				i++;
				paramsi = pai->data()->params();
				j = -1;
				for (paj = p2->atoms_.first(); paj != NULL; paj = paj->next)
				{
					j++;
					// Check for conflicting VDW types
					if (p1->atoms_[i]->data()->vdwForm() != p2->atoms_[j]->data()->vdwForm())
					{
						msg.print( "Conflicting vdW types for atoms %i (%s) and %i (%s).\n", i, VdwFunctions::VdwFunctions[p1->atoms_[i]->data()->vdwForm()].name, j, VdwFunctions::VdwFunctions[p2->atoms_[j]->data()->vdwForm()].name);
						continue;
					}
					paramsj = paj->data()->params();
					switch (p2->atoms_[j]->data()->vdwForm())
					{
						case (VdwFunctions::None):
							msg.print( "Warning: No function is specified for vdW energy correction %i-%i.\n", i, j);
							du_dr = 0.0;
							break;
						case (VdwFunctions::Lj):
							// U = 4/3 * eps * sigma**3 * ( 1/3 * (s/r)**9 - (s/r)**3
							epsilon = sqrt(paramsi.data[VdwFunctions::LjEpsilon] * paramsj.data[VdwFunctions::LjEpsilon]);
							sigma = 0.5 * (paramsi.data[VdwFunctions::LjSigma] + paramsj.data[VdwFunctions::LjSigma]) * vrs;
							sigmar9 = sigma / cutoff;
							sigmar3 = sigmar9 * sigmar9 * sigmar9;
							sigmar9 = sigmar3 * sigmar3 * sigmar3;
							du_dr = (4.0/3.0) * epsilon * ( sigmar9/3.0 - sigmar3 );
							du_dr *= (sigma * sigma * sigma);
							break;
						default:
							msg.print("VDW tail correction not implemented for %s VDW interactions.\n", VdwFunctions::VdwFunctions[p1->atoms_[i]->data()->vdwForm()].name);
							break;
					}
					energy += 2.0 * PI * rho * du_dr;
				}
			}
		}
	}
	estore->add(Energy::VdwTailEnergy,energy,-1);
	msg.exit("Pattern::vdwCorrectEnergy");
}
