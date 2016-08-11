      subroutine matou1(a, b, ncx, nr, ndim, iflag) 
!-----------------------------------------------
!   M o d u l e s 
!-----------------------------------------------
      USE vast_kind_param, ONLY:  double 
      use molkst_C, only : numat, norbs, nalpha, nclose, keywrd
      use permanent_arrays, only : nfirst, nlast, nat
      use chanel_C, only : iw
      use elemts_C, only : elemnt
      USE symmetry_C, only :  jndex, namo
!***********************************************************************
!DECK MOPAC
!...Translated by Pacific-Sierra Research 77to90  4.4G  10:47:24  03/09/06  
!...Switches: -rl INDDO=2 INDIF=2 
      implicit none
!-----------------------------------------------
!   D u m m y   A r g u m e n t s
!-----------------------------------------------
      integer , intent(in) :: ncx 
      integer , intent(inout) :: nr 
      integer , intent(in) :: ndim 
      integer , intent(in) :: iflag 
      real(double) , intent(in) :: a(nr,nr) 
      real(double) , intent(in) :: b(ndim) 

!-----------------------------------------------
!   L o c a l   V a r i a b l e s
!-----------------------------------------------
      integer , dimension(norbs + 3* numat) :: natom 
      integer :: nc, nsave, nfix, i, jlo, jhi, l, j, ka, kc, kb, la, lc, lb 
      logical :: allprt 
      character , dimension(9) :: atorbs*2 
      character , dimension(norbs + 3* numat) :: itext*2, jtext*2 
      character, dimension(3) :: xyz*2 

      save atorbs, xyz 
!-----------------------------------------------
!**********************************************************************
!
!      MATOUT PRINTS A SQUARE MATRIX OF EIGENVECTORS AND EIGENVALUES
!
!    ON INPUT A CONTAINS THE MATRIX TO BE PRINTED.
!             B CONTAINS THE EIGENVALUES.
!             NC NUMBER OF MOLECULAR ORBITALS TO BE PRINTED.
!             NR IS THE SIZE OF THE SQUARE ARRAY TO BE PRINTED.
!             NDIM IS THE ACTUAL SIZE OF THE SQUARE ARRAY "A".
!             NFIRST AND NLAST CONTAIN ATOM ORBITAL COUNTERS.
!             NAT = ARRAY OF ATOMIC NUMBERS OF ATOMS.
!
!
!     OUTPUT TYPE (ROW LABELING)
!       IFLAG=1 : ORBITALS
!       IFLAG=2 : ORBITALS + SYMMETRY-DESIGNATORS
!       IFLAG=3 : ATOMS
!       IFLAG=4 : NUMBERS ONLY
!       IFLAG=5 : VIBRATIONS + SYMMETRY-DESIGNATIONS
!
!
!***********************************************************************
      data xyz/ ' x', ' y', ' z'/  
      data atorbs/ 'S ', 'Px', 'Py', 'Pz', 'x2', 'xz', 'z2', 'yz', 'xz'/  
!      -------------------------------------------------
      allprt = index(keywrd,'ALLVEC') /= 0 
      if (iflag>2 .and. iflag/=5) go to 50 
      nc = ncx 
      if (.not.allprt) then 
        nsave = ncx 
        nfix = max(nalpha,nclose) 
        if (iflag==2 .and. nc>16) nc = nfix + 7 
        nc = min0(nsave,nc) 
      endif 
      if (numat == 0) go to 50 
      if (nlast(numat) /= nr) go to 50 
      do i = 1, numat 
        jlo = nfirst(i) 
        jhi = nlast(i) 
        l = nat(i) 
        if (iflag <= 2) then 
          itext(jlo:jhi) = atorbs(:jhi-jlo+1) 
          jtext(jlo:jhi) = elemnt(l) 
          natom(jlo:jhi) = i 
        else 
          jhi = 3*(i - 1) 
          itext(1+jhi:3+jhi) = xyz 
          jtext(1+jhi:3+jhi) = elemnt(l) 
          natom(1+jhi:3+jhi) = i 
        endif 
      end do 
      go to 70 
   50 continue 
      nr = abs(nr) 
      if (iflag == 3) then 
        do i = 1, nr 
          itext(i) = '  ' 
          jtext(i) = elemnt(nat(i)) 
          natom(i) = i 
        end do 
      else 
        do i = 1, nr 
          itext(i) = '  ' 
          jtext(i) = '  ' 
          natom(i) = i 
        end do 
      endif 
   70 continue 
      ka = 1 
      kc = 8 
      if (.not.allprt) then 
        if (iflag==2 .and. norbs>16) ka = nfix - 8 
        ka = max0(1,ka) 
        if (iflag==2 .and. norbs>16) kc = ka + 7 
      endif 
   90 continue 
      kb = min0(kc,nc) 
      write (iw, 130) (i,i=ka,kb) 
      if (iflag==2 .or. iflag==5) write (iw, 180) (jndex(i),namo(i),i=ka,kb) 
      if (b(1) /= 0.D0) then 
        if (iflag == 5) then 
          write (iw, 150) (b(i),i=ka,kb) 
        else 
          write (iw, 140) (b(i),i=ka,kb) 
        endif 
      endif 
      write (iw, 160) 
      la = 1 
      lc = 40 
  100 continue 
      lb = min0(lc,nr) 
      do i = la, lb 
        if (itext(i) == ' S') write (iw, 160) 
        write (iw, 170) itext(i), jtext(i), natom(i), (a(i,j),j=ka,kb) 
      end do 
      if (lb == nr) go to 120 
      la = lc + 1 
      lc = lc + 40 
      go to 100 
  120 continue 
      if (kb == nc) return  
      ka = kc + 1 
      kc = kc + 8 
      go to 90 
  130 format(/,/,2x,' Root No.',i5,9i8) 
  140 format(/,10x,10f8.3) 
  150 format(/,10x,10f8.1) 
  160 format('  ') 
  170 format(' ',2(1x,a2),i3,f8.4,10f8.4) 
  180 format(/,12x,10(i3,1x,a4)) 
      end subroutine matou1 
