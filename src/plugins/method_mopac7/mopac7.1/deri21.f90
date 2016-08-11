      subroutine deri21(a, nvar_nvo, minear, ifirst, vnert, pnert, b, ncut) 
!-----------------------------------------------
!   M o d u l e s 
!-----------------------------------------------
      USE vast_kind_param, ONLY:  double 
!...Translated by Pacific-Sierra Research 77to90  4.4G  10:47:06  03/09/06  
!...Switches: -rl INDDO=2 INDIF=2 
!-----------------------------------------------
!   I n t e r f a c e   B l o c k s
!-----------------------------------------------
      use mtxmc_I 
      use rsp_I 
      use mxm_I 
      implicit none
!-----------------------------------------------
!   D u m m y   A r g u m e n t s
!-----------------------------------------------
      integer  :: nvar_nvo 
      integer  :: minear 
      integer , intent(in) :: ifirst 
      integer  :: ncut 
      real(double)  :: a(minear,nvar_nvo) 
      real(double)  :: vnert(nvar_nvo) 
      real(double)  :: pnert(nvar_nvo) 
      real(double)  :: b(minear,nvar_nvo) 
!-----------------------------------------------
!   L o c a l   V a r i a b l e s
!-----------------------------------------------
      integer :: i, l
      real(double), dimension(4) :: work 
      real(double) :: cutoff, sum2, sum 
!-----------------------------------------------
!***********************************************************************
!
!     LEAST-SQUARE ANALYSIS OF A SET OF nvar_nvo POINTS {A} :
!
!     PRODUCE A SUBSET OF NCUT ORTHONORMALIZED VECTORS B, OPTIMUM IN A
!     LEAST-SQUARE SENSE WITH RESPECT TO THE INITIAL SPACE {A}.
!     EACH NEW HIERARCHIZED VECTOR B EXTRACTS A MAXIMUM PERCENTAGE FROM
!     THE REMAINING DISPERSION OF THE SET {A} OUT OF THE PREVIOUS
!     {B} SUBSPACE.
!   INPUT
!     A(MINEAR,nvar_nvo): ORIGINAL SET {A}.
!     ifirst        : MAXIMUM ALLOWED SIZE OF THE BASIS B.
!   OUTPUT
!     VNERT(nvar_nvo)   : LOWEST EIGENVECTOR OF A'* A.
!     PNERT(nvar_nvo)     : SQUARE ROOT OF THE ASSOCIATED EIGENVALUES
!                     IN DECREASING ORDER.
!     B(MINEAR,NCUT): OPTIMUM ORTHONORMALIZED SUBSET {B}.
!
!***********************************************************************
!
!     VNERT = A' * A
      cutoff = 0.85D0 
      sum2 = 0.D0 
      call mtxmc (a, nvar_nvo, a, minear, work) 
      work(:nvar_nvo*(nvar_nvo+1)/2) = -work(:nvar_nvo*(nvar_nvo+1)/2) 
!     DIAGONALIZE IN DECREASING ORDER OF EIGENVALUES
      if (abs(work(1))<1.D-28 .and. nvar_nvo==1) then 
        pnert(1) = sqrt((-work(1))) 
        work(1) = 1.D15 
        vnert(1) = 1.D0 
        ncut = 1 
        go to 50 
      else 
        call rsp (work, nvar_nvo, 1, pnert, vnert) 
!     FIND NCUT ACCORDING TO CUTOFF, BUILD WORK = VNERT * (PNERT)**-0.5
        sum = 0.D0 
        do i = 1, nvar_nvo 
          sum = sum - pnert(i) 
        end do 
        l = 1 
        do i = 1, ifirst 
          sum2 = sum2 - pnert(i)/sum 
          pnert(i) = sqrt((-pnert(i))) 
          if (nvar_nvo > 0) then 
            work(l:nvar_nvo-1+l) = vnert(l:nvar_nvo-1+l)/pnert(i) 
            l = nvar_nvo + l 
          endif 
          if (sum2 < cutoff) cycle  
          ncut = i 
          go to 50 
        end do 
        ncut = ifirst 
!     ORTHONORMALIZED BASIS
!     B(MINEAR,NCUT) = A(MINEAR,nvar_nvo)*WORK(nvar_nvo,NCUT)
      endif 
   50 continue 
      call mxm (a, minear, work, nvar_nvo, b, ncut) 
      return  
      end subroutine deri21 
