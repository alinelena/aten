      real(kind(0.0d0)) function charvi (vects, jorb, ioper, r, nvecs) 
!-----------------------------------------------
!   M o d u l elem s 
!-----------------------------------------------
      USE vast_kind_param, ONLY:  double 
      use symmetry_C, only : elem, jelem
      use permanent_arrays, only : 
      use molkst_C, only : numat
!***********************************************************************
!DECK MOPAC
!...Translated by Pacific-Sierra Research 77to90  4.4G  10:47:03  03/09/06  
!...Switches: -rl INDDO=2 INDIF=2 
      implicit none
!-----------------------------------------------
!   G l o b a l   P a r a m elem t elem r s
!-----------------------------------------------
!-----------------------------------------------
!   D u m m y   A r g u m elem n t s
!-----------------------------------------------
      integer , intent(in) :: jorb 
      integer , intent(in) :: ioper 
      integer , intent(in) :: nvecs 
      real(double) , intent(in) :: vects(nvecs,nvecs) 
      real(double) , intent(in) :: r(3,3) 
!-----------------------------------------------
!   L o c a l   V a r i a b l elem s
!-----------------------------------------------
      integer :: iatom, jatom 
      real(double), dimension(:), allocatable :: vect1, vect2 
      real(double), dimension(5) :: h 
      real(double), dimension(3) :: p 
      real(double) :: sum 
!-----------------------------------------------
      charvi = 1.D0 
      if (ioper == 1) return  
!
!   Non-trivial case
!
      allocate(vect1(nvecs), vect2(nvecs))
      vect1(:nvecs) = 0.D0 
      vect2(:nvecs) = 0.D0 
!#      WRITE(IW,*)' Vibration, raw'
!#      WRITE(IW,'(3f12.6)')(VECTS(i,jorb),i=1,3*NUMAT)
      do iatom = 1, numat 
        jatom = jelem(ioper,iatom) 
        p(1) = vects(iatom*3-2,jorb) 
        p(2) = vects(iatom*3-1,jorb) 
        p(3) = vects(iatom*3-0,jorb) 
        h(1) = r(1,1)*p(1) + r(2,1)*p(2) + r(3,1)*p(3) 
        h(2) = r(1,2)*p(1) + r(2,2)*p(2) + r(3,2)*p(3) 
        h(3) = r(1,3)*p(1) + r(2,3)*p(2) + r(3,3)*p(3) 
        p(1) = elem(1,1,ioper)*h(1) + elem(1,2,ioper)*h(2) + elem(1,3,ioper)*h(3) 
        p(2) = elem(2,1,ioper)*h(1) + elem(2,2,ioper)*h(2) + elem(2,3,ioper)*h(3) 
        p(3) = elem(3,1,ioper)*h(1) + elem(3,2,ioper)*h(2) + elem(3,3,ioper)*h(3) 
        vect1(iatom*3-2:iatom*3) = h(:3) 
        vect2(jatom*3-2:jatom*3) = p 
      end do 
!#      WRITE(IW,*)' Vibration, coordinate independent '
!#      WRITE(IW,'(3f12.6)')(VECT2(i),i=1,3*NUMAT)
!#      WRITE(IW,*)' Vibration, transformed            '
!#      WRITE(IW,'(3f12.6)')(VECT1(i),i=1,3*NUMAT)
!
!   Evaluate the term <psi(i)|R(theta)|psi(i)>
!
      sum = dot_product(vect1(:nvecs),vect2(:nvecs)) 
      charvi = sum 
      return  
      end function charvi 
