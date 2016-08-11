      real(kind(0.0d0)) function babbcd (iocca1, ioccb1, iocca2, ioccb2, nmos, &
        xy) 
!-----------------------------------------------
!   M o d u l e s 
!-----------------------------------------------
      USE vast_kind_param, ONLY:  double 
!***********************************************************************
!DECK MOPAC
!...Translated by Pacific-Sierra Research 77to90  4.4G  10:47:01  03/09/06  
!...Switches: -rl INDDO=2 INDIF=2 
      implicit none
!-----------------------------------------------
!   D u m m y   A r g u m e n t s
!-----------------------------------------------
      integer , intent(in) :: nmos 
      integer , intent(in) :: iocca1(nmos) 
      integer , intent(in) :: ioccb1(nmos) 
      integer , intent(in) :: iocca2(nmos) 
      integer , intent(in) :: ioccb2(nmos) 
      real(double) , intent(in) :: xy(nmos,nmos,nmos,nmos) 
!-----------------------------------------------
!   L o c a l   P a r a m e t e r s
!-----------------------------------------------
!-----------------------------------------------
!   L o c a l   V a r i a b l e s
!-----------------------------------------------
      integer :: ij, i, j, k, l 
      real(double) :: one 
!-----------------------------------------------
!**********************************************************************
!
! BABBCD EVALUATES THE C.I. MATRIX ELEMENT FOR TWO MICROSTATES DIFFERING
!       BY TWO BETA MOS. ONE MICROSTATE HAS BETA ELECTRONS IN
!       M.O.S PSI(I) AND PSI(J) FOR WHICH THE OTHER MICROSTATE HAS
!       ELECTRONS IN PSI(K) AND PSI(L)
!
!**********************************************************************
      ij = 0 
      do i = 1, nmos 
        if (ioccb1(i) >= ioccb2(i)) cycle  
        exit  
      end do 
      do j = i + 1, nmos 
        if (ioccb1(j) < ioccb2(j)) exit  
        ij = ij + iocca2(j) + ioccb2(j) 
      end do 
      ij = ij + iocca2(j) 
      do k = 1, nmos 
        if (ioccb1(k) <= ioccb2(k)) cycle  
        exit  
      end do 
      do l = k + 1, nmos 
        if (ioccb1(l) > ioccb2(l)) exit  
        ij = ij + iocca1(l) + ioccb1(l) 
      end do 
      ij = ij + iocca1(l) 
      if ((ij/2)*2 == ij) then 
        one = 1.D0 
      else 
        one = -1.D0 
      endif 
      babbcd = (xy(i,k,j,l)-xy(i,l,j,k))*one 
      return  
      end function babbcd 
