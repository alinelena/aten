      subroutine dernvo() 
!-----------------------------------------------
!   M o d u l e s 
!-----------------------------------------------
      USE vast_kind_param, ONLY:  double 
      USE molkst_C, only : numcal, numat, nopen, nclose, norbs, gnorm, &
      & fract, keywrd, mpack, moperr, lm6
      use meci_C, only : nbo, nmos
      use permanent_arrays, only : eigs, dxyz
      use chanel_C, only : iw 
!***********************************************************************
!DECK MOPAC
!...Translated by Pacific-Sierra Research 77to90  4.4G  14:24:19  03/15/06  
!...Switches: -rl INDDO=2 INDIF=2 
!-----------------------------------------------
!   I n t e r f a c e   B l o c k s
!-----------------------------------------------
      use deri0_I 
      use deri1_I 
      use deri2_I 
      implicit none 
      real(double), dimension(:), allocatable  :: scalar, diag, fmooff
!-----------------------------------------------
!   L o c a l   V a r i a b l e s
!-----------------------------------------------
      integer ::  minear, ninear, nvax, icalcn, i, ilast, ifirst, j, k, l, ll 
      real(double), dimension(nmos*norbs + 1) :: fmoon 
      real(double), dimension(3*numat) :: dxyzr 
      real(double), dimension(nmos*norbs*20) :: eigbb 
      real(double) :: sum, throld, sumx, sumy, sumz 
      logical :: debug, dcar, large, relaxd 
      character :: blank*60 

      save  debug, dcar, large,  minear, ninear, nvax, icalcn, &
      & diag, scalar, fmooff
!-----------------------------------------------
!**********************************************************************
!
!    IMPLEMENTATION OF ANALYTICAL FORMULATION FOR OPEN SHELL OR CI,
!                      VARIABLES FINITE DIFFERENCE METHODS,
!                      STATISTICAL ESTIMATE OF THE ERRORS,
!                   BY D. LIOTARD
!                      LABORATOIRE DE CHIMIE STRUCTURALE
!                      UNIVERSITE DE PAU ET DES PAYS DE L'ADOUR
!                      AVENUE DE L'UNIVERSITE, 64000, PAU (FRANCE)
!
!
!   MODIFIED BY JJPS TO CONFORM TO MOPAC CONVENTIONS
!   (NOTE BY JJPS:  PROF. LIOTARD'S TECHNIQUE WORKS.  IF THIS
!   IMPLEMENTATION DOES NOT WORK, THE REASON IS A FAULT INTRODUCED
!   BY JJPS, AND DOES NOT REFLECT ON PROF. LIOTARD'S ABILITY)
!
!
!    AS THE WAVE FUNCTION IS NOT VARIATIONALLY OPTIMIZED, I.E.
!    HALF-ELECTRON OR CI, THE DERIVATIVES OF THE 1 AND 2-ELECTRON
!    INTEGRALS IN A.O. BASIS ARE EVALUATED IN CARTESIAN COORDINATES
!    BY A 1 OR 2 POINTS FINITE DIFFERENCE FORMULA AND STORED.
!    THUS ONE GETS THE NON-RELAXED (I.E. FROZEN ELECTRONIC CLOUD)
!    CONTRIBUTION TO THE FOCK EIGENVALUES AND 2-ELECTRON INTEGRALS IN
!    AN M.O. BASIS.  THE NON-RELAXED GRADIENT COMES FROM THE
!    NON-RELAXED C.I. MATRIX DERIVATIVE (SUBROUTINE DERI1).
!    THE DERIVATIVES OF THE M.O. COEFFICIENTS ARE THEN WORKED OUT
!    ITERATIVELY (OK FOR BOTH CLOSED SHELLS AND HALF-ELECTRON CASES)
!    AND STORED. THUS ONE GETS THE ELECTRONIC RELAXATION CONTRIBUTION TO
!    THE FOCK EIGENVALUES AND 2-ELECTRON INTEGRALS IN M.O. BASIS.
!    FINALLY THE RELAXATION CONTRIBUTION TO THE C.I. MATRIX DERIVATIVE
!    GIVES THE RELAXATION CONTRIBUTION TO THE GRADIENT (ROUTINE DERI2).
!
!
!        COORD  HOLDS THE CARTESIAN COORDINATES.
!    INPUT
!        DXYZ   NOT DEFINED.
!    EXIT
!        DXYZ   DERIVATIVES OF ENERGY W.R.T CARTESIAN COORDINATES,
!               IN KCAL/MOL/ANGSTROM (3 * NUMAT OF THESE)
!
!**********************************************************************
      data icalcn/ 0/  
!        ACTUAL SIZES FOR C.I. GRADIENT CALCULATION.
        nbo(1) = nclose 
        nbo(2) = nopen - nclose 
        nbo(3) = norbs - nopen 
        minear = nbo(2)*nbo(1) + nbo(3)*nopen 
        ninear = (nmos*(nmos + 1))/2 + 1        
        if(allocated(scalar)) deallocate(scalar)
        allocate(scalar(minear))
        if(allocated(diag)) deallocate(diag)
        allocate(diag(minear))
        if(allocated(fmooff)) deallocate(fmooff)
        allocate(fmooff(2*minear))
!
!     SELECT THE REQUIRED OPTION AND READ KEYWORDS
!     --------------------------------------------
!
      if (icalcn /= numcal) then 
        icalcn = numcal 
        debug = index(keywrd,'DERNVO') /= 0 
        large = index(keywrd,'LARGE') /= 0 
        dcar = index(keywrd,'FORC') + index(keywrd,'PREC') /= 0 
        nvax = 3*numat         
      endif 
      dxyzr(:nvax) = 0.D0 
!        SCALING ROW FACTORS TO SPEED CV OF RELAXATION PROCEDURE.
      call deri0 (eigs, norbs, scalar, diag, fract, nbo) 
!
!   BECAUSE DERI2 IS CPU INTENSIVE, AND THE CONTRIBUTION TO THE
!   DERIVATIVE DUE TO RELAXATION OF THE ELECTRON CLOUD IS RELATIVELY
!   INSENSITIVE TO CHANGES IN GEOMETRY, WHERE POSSIBLE ONLY CALCULATE
!   THE DERIVATIVE EVERY 2 CALLS TO DERNVO
!
      eigbb = 0.D0 
      sum = 0.D0 
      if (gnorm<1.D0 .or. dcar) then 
        dxyzr(:nvax) = 0.D0 
        relaxd = .FALSE. 
      endif 
      do i = 1, nvax 
        sum = sum + abs(dxyzr(i)) 
      end do 
      relaxd = sum > 1.D-7 
!
!  IF DXYZR CONTAINS DATA, USE IT AND FLUSH AFTER USE.
!
      ilast = 0 
   50 continue 
      ifirst = ilast + 1 
      ilast = min(nvax,ilast + 1) 
      j = 1 - minear 
      k = 1 - ninear 
      do i = ifirst, ilast 
        k = k + ninear 
        j = j + minear 
!
!        NON-RELAXED CONTRIBUTION (FROZEN ELECTRONIC CLOUD) IN DXYZ
!        AND NON-RELAXED FOCK MATRICES IN FMOOFF AND FMOON.
!   CONTENTS OF F-MO-OFF: OPEN-CLOSED, VIRTUAL-CLOSED, AND VIRTUAL-OPEN
!   CONTENTS OF F-MO-ON:  CLOSED-CLOSED, OPEN-OPEN AND VIRTUAL-VIRTUAL
!   OVER M.O. INDICES
!
        if (lm6 /= 0) then 
          l = 45*lm6 + 1 
        else 
          l = 3*mpack 
        endif 
        call deri1 (i, dxyz(i), fmooff(j), minear, fmoon(k), scalar) 
      end do 
      if (debug) then 
        if (ifirst==1 .and. large) then 
          write (iw, *) ' CONTENTS OF FMOOFF ' 
          write (iw, *) ' OPEN-CLOSED' 
          write (iw, '(7X,I3,5I12)') (j,j=nclose + 1,nopen) 
          do i = 1, nclose 
            write (iw, '(I3,6F12.6)') i, (fmooff(j),j=(i - 1)*nbo(2) + 1,i*nbo(&
              2)) 
          end do 
!
!
          write (iw, *) ' VIRTUAL-CLOSED' 
          k = nclose*nbo(2) 
          write (iw, '(7X,I3,5I12)') (j,j=nopen + 1,min(nopen + 6,norbs)) 
          do i = 1, nclose 
            write (iw, '(I3,6F12.6)') i, (fmooff(j+k),j=(i - 1)*nbo(3) + 1,min(&
              6 + (i - 1)*nbo(3),i*nbo(3))) 
          end do 
          k = nclose*nbo(2) + nbo(3)*nclose 
!
!
          write (iw, *) ' VIRTUAL-OPEN' 
          write (iw, '(7X,I3,4I12)') (j,j=nclose + 1,nopen) 
          do i = 1, min(6,nbo(3)) 
            write (iw, '(I3,6F12.6)') i + nopen, (fmooff(j+k),j=(i - 1)*nbo(2)&
               + 1,min((i - 1)*nbo(2)+6,i*nbo(2))) 
          end do 
          write (iw, *) ' CONTENTS OF FMOON (ACTIVE-SPACE -- ACTIVE SPACE)' 
          k = (nmos*(nmos - 1))/2 
          ll = 1 
          blank = ' ' 
          do i = 1, nmos 
            l = ll + nmos - i - 1 
            write (iw, '(A,5F12.6)') blank(:12*i), (fmoon(j),j=ll,l), fmoon(k+i&
              ) 
            ll = l + 1 
          end do 
        endif 
      endif 
!        COMPUTE THE ELECTRONIC RELAXATION CONTRIBUTION.
!
!   DERNVO PROVIDES THE FOLLOWING SCRATCH AREAS TO DERI2: EIGB, WORK2,
!          WORK3, CBETA.  THESE ARE DIMENSIONED ON ENTRY TO DERI2
!          WHICH IS WHY THEY ARE NOT DECLARED THERE.  THEY ARE NOT USED
!          AT ALL IN DERNVO.
!
!
!  The following function was chosen as a guide to THROLD.  It is NOT
!  intended to be hard-and-fast.
!
!     throld = max(0.001D0,min(thref,gnorm**3*0.00002D0)) 
      throld = 0.00001D0 
      if (.not.relaxd) call deri2 (minear, fmooff, fmoon, eigbb &
        , ninear, ilast - ifirst + 1, dxyzr(ifirst) &
        , throld,  diag, scalar) 
      if (moperr) return  
      if (ilast < nvax) go to 50 
      if (debug) then 
        sumx = 0.D0 
        sumy = 0.D0 
        sumz = 0.D0 
        do i = 1, numat 
          sumx = sumx + dxyz(i*3-2) 
          sumy = sumy + dxyz(i*3-1) 
          sumz = sumz + dxyz(i*3) 
        end do 
        write (iw, *) ' CARTESIAN DERIVATIVES DUE TO FROZEN CORE' 
        write (iw, '('' ATOM    X           Y           Z'')') 
        do i = 1, numat 
          write (iw, '(I4,3F12.7)') i, dxyz(i*3-2), dxyz(i*3-1), dxyz(i*3) 
        end do 
        write (iw, '(/10X,''RESIDUAL ERROR'')') 
        write (iw, '(4X,3F12.7)') sumx, sumy, sumz 
        write (iw, *) 
        sumx = 0.D0 
        sumy = 0.D0 
        sumz = 0.D0 
        do i = 1, numat 
          sumx = sumx + dxyzr(i*3-2) 
          sumy = sumy + dxyzr(i*3-1) 
          sumz = sumz + dxyzr(i*3) 
        end do 
        write (iw, *) ' CARTESIAN DERIVATIVES DUE TO RELAXING CORE' 
        write (iw, '('' ATOM    X           Y           Z'')') 
        do i = 1, numat 
          write (iw, '(I4,3F12.7)') i, dxyzr(i*3-2), dxyzr(i*3-1), dxyzr(i*3) 
        end do 
        write (iw, '(/10X,''RESIDUAL ERROR'')') 
        write (iw, '(4X,3F12.7)') sumx, sumy, sumz 
        write (iw, *) 
      endif 
      dxyz(:nvax) = dxyz(:nvax) + dxyzr(:nvax) 
      if (relaxd) then 
        dxyzr(:nvax) = 0.D0 
      endif 
      sumx = 0.D0 
      sumy = 0.D0 
      sumz = 0.D0 
      do i = 1, numat 
        sumx = sumx + dxyz(i*3-2) 
        sumy = sumy + dxyz(i*3-1) 
        sumz = sumz + dxyz(i*3) 
      end do 
      sum = max(1.D-10,abs(sumx) + abs(sumy) + abs(sumz)) 
      if (debug) then 
        write (iw, *) 'CARTESIAN DERIVATIVES FROM ANALYTICAL C.I. CALCULATION' 
        write (iw, '('' ATOM    X           Y           Z'')') 
        do i = 1, numat 
          write (iw, '(I4,3F12.7)') i, dxyz(i*3-2), dxyz(i*3-1), dxyz(i*3) 
        end do 
        write (iw, '(/10X,''RESIDUAL ERROR'')') 
        write (iw, '(4X,3F12.7)') sumx, sumy, sumz 
        write (iw, *) 
      endif 
      return  
      end subroutine dernvo 
