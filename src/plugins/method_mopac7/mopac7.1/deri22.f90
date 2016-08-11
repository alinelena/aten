      subroutine deri22(c, b, work, foc2, ab, minear, fci, w, diag, &
        scalar, ninear) 
!-----------------------------------------------
!   M o d u l e s 
!-----------------------------------------------
      USE vast_kind_param, ONLY:  double 
      USE molkst_C, only : nopen, numat, lm6, norbs, n2elec
      use permanent_arrays, only : nfirst, nlast, nw
      use meci_C, only : nmos, nbo, nelec
!***********************************************************************
!DECK MOPAC
!...Translated by Pacific-Sierra Research 77to90  4.4G  17:49:16  03/12/06  
!...Switches: -rl INDDO=2 INDIF=2 
!-----------------------------------------------
!   I n t e r f a c e   B l o c k s
!-----------------------------------------------
      use mxm_I 
      use mxmt_I 
      use ddot_I 
      use fockd2_I 
      use fock2_I 
      use fockd1_I 
      use fock1_I 
      use supdot_I 
      use dot_I 
      use mtxm_I 
      implicit none
!-----------------------------------------------
!   G l o b a l   P a r a m e t e r s
!-----------------------------------------------
!-----------------------------------------------
!   D u m m y   A r g u m e n t s
!-----------------------------------------------
      integer , intent(in) :: minear 
      integer , intent(in) :: ninear 
      real(double)  :: c(norbs,norbs) 
      real(double)  :: b(minear) 
      real(double)  :: work(norbs,norbs) 
      real(double)  :: foc2(norbs*norbs) 
      real(double)  :: ab(minear) 
      real(double)  :: fci(ninear) 
      real(double)  :: w(n2elec) 
      real(double) , intent(in) :: diag(*) 
      real(double) , intent(in) :: scalar(*) 
      real(double)  :: dp(norbs**2) 
      real(double)  :: dpa(norbs**2) 
!-----------------------------------------------
!   L o c a l   P a r a m e t e r s
!-----------------------------------------------
!-----------------------------------------------
!   L o c a l   V a r i a b l e s
!-----------------------------------------------
      integer :: i, l, j, nend, loop, ninit, n1, n2, icount
      real(double) :: wj(1) = 0.d0, wk(1) = 0.d0  ! Dummys - wj and wk are used by solids only. 
!-----------------------------------------------
!***********************************************************************
!  1) BUILD THE 2-ELECTRON FOCK MATRIX DEPENDING ON B AS FOLLOWS :
!     DP = C * SCALE*B * C' ...  DP DENSITY MATRIX 'DERIVATIVE',
!     FOC2 = 0.5 * TRACE ( DP * (2<J>-<K>) ) DONE IN FOCK2 & FOCK1.
!  2) HALF-TRANSFORM ONTO M.O. BASIS : DPT =  FOC2 * C
!     AND COMPUTE DIAGONAL BLOCKS ELEMENTS OF C' * FOC2, EXTRACTING
!     IN FCI ELEMENTS OVER C.I-ACTIVE M.O ONLY.
!  3) COMPUTE SUPERVECTOR AB = (DIAG + A) * B DEFINED BY THE MATRIX :
!     AB(I,J)= ( DIAG(I,J)*B(I,J)+DPT(I,J) )*SCALAR(I,J)  WITH I.GT.J,
!     DIAG(I,J)=(EIGS(I)-EIGS(J))/(O(J)-O(I)) >0, O OCCUPANCY NUMBERS,
!     EIGS EIGENVALUES OF FOCK OPERATOR WITH EIGENVECTORS C IN A.O.
!
!   INPUT
! C(NORBS,NORBS)   : M.O. EIGENVECTORS (COLUMNWISE).
! B(*)             : B SUPERVECTOR PACKED BY OFF-DIAGONAL BLOCKS, SCALED
! WORK(*)          : WORK AREA OF SIZE N*N.
! NORBS            : NUMBER OF M.O.S
! NELEC,NMOS       : LAST FROZEN CORE M.O. , C.I-ACTIVE BAND LENGTH.
!           IN COMMON
! DIAG,SCALAR AS DEFINED IN 'DERI0'.
!   OUTPUT
! FOC2(*)       : 2-ELECTRON FOCK MATRIX, PACKED CANONICAL.
! AB(*)         : ANTISYMMETRIC MATRIX PACKED IN SUPERVECTOR FORM WITH
!                 THE CONSECUTIVE FOLLOWING BLOCKS:
!              1) OPEN-CLOSED  I.E. B(IJ)=B(I,J) WITH I OPEN & J CLOSED
!                 AND I RUNNING FASTER THAN J,
!              2) VIRTUAL-CLOSED SAME RULE OF ORDERING,
!              3) VIRTUAL-OPEN   SAME RULE OF ORDERING.
! FCI(*)        : FOCK DIAGONAL BLOCKS ELEMENTS OVER C.I-ACTIVE M.O.
!            FOC2 CAN BE EQUIVALENCED WITH WORK IN THE CALLING SEQUENCE.
!***********************************************************************
!
!  NOTE: NORBS AND NORD ARE THE SAME ADDRESS.  THE NAME NORBD IS NOT
!        USED HERE.
!
!
!     DERIVATIVE OF THE DENSITY MATRIX IN DP (PACKED,CANONICAL).
!     ----------------------------------------------------------
!     DP = C * B * C' .
!
!     STEP 0 : UNSCALE VECTOR B.
      b = b*scalar(:minear) 
!
!     STEP 1 : WORK = C * B    .  DP TEMPORARY ARRAY.
      l = 1 
      if (nbo(2)/=0 .and. nbo(1)/=0) then 
!        OPEN-CLOSED
        call mxm (c(1,nbo(1)+1), norbs, b(l), nbo(2), work, nbo(1)) 
!       CLOSED-OPEN
        call mxmt (c, norbs, b(l), nbo(1), work(1,nbo(1)+1), nbo(2)) 
        l = l + nbo(2)*nbo(1) 
      endif 
      if (nbo(3)/=0 .and. nbo(1)/=0) then 
!       VIRTUAL-CLOSED
        if (l > 1) then 
          call mxm (c(1,nopen+1), norbs, b(l), nbo(3), dp, nbo(1)) 
          icount = 0
        do j = 1, nbo(1)
          do i = 1, norbs
            icount = icount + 1
            work(i, j) = work(i, j) + dp(icount)
          end do
        end do
        else 
          call mxm (c(1,nopen+1), norbs, b(l), nbo(3), work, nbo(1)) 
        endif 
!       CLOSED-VIRTUAL
        call mxmt (c, norbs, b(l), nbo(1), work(1,nopen+1), nbo(3)) 
        l = l + nbo(3)*nbo(1) 
      endif 
      if (nbo(3)/=0 .and. nbo(2)/=0) then 
!       VIRTUAL-OPEN
        call mxm (c(1,nopen+1), norbs, b(l), nbo(3), dp, nbo(2)) 
         icount = 0
      do j = nbo(1) + 1, nbo(1) + nbo(2)
        do i = 1, norbs
          icount = icount + 1
          work(i, j) = work(i, j) + dp(icount)
        end do
      end do
!       OPEN-VIRTUAL
        call mxmt (c(1,nbo(1)+1), norbs, b(l), nbo(2), dp, nbo(3)) 
     icount = 0
      do j = nopen + 1, nopen + nbo(3)
        do i = 1, norbs
          icount = icount + 1
          work(i, j) = work(i, j) + dp(icount)
        end do
      end do
      endif 
!
!     STEP 2 : DP= WORK * C'   WITH DP PACKED,CANONICAL.
      l = 0 
      do i = 1, norbs 
        do j = 1, i 
          l = l + 1 
          dp(l) = ddot(norbs,work(i,1),norbs,c(j,1),norbs) 
        end do 
      end do 
!
!     2-ELECTRON FOCK MATRIX BUILD WITH THE DENSITY MATRIX DERIVATIVE.
!     ----------------------------------------------------------------
!     RETURNED IN FOC2 (PACKED CANONICAL).
      foc2 = 0.D0 
      dpa = 0.5D0*dp
      if (lm6 /= 0) then 
        call fockd2 (foc2, dp, dpa, w, lm6, wj, wk, numat, nfirst, nlast, nw) 
      else 
        call fock2 (foc2, dp, dpa, w, wj, wk, numat, nfirst, nlast) 
      endif 
      if (lm6 /= 0) then 
        call fockd1 (foc2, dp, dpa, w) 
      else 
        call fock1 (foc2, dp, dpa, dpa) 
      endif 
!
!     BUILD DP AND EXTRACT FCI.
!     --------------------------
!
!     DP(NORBS,NEND) = FOC2(NORBS,NORBS) * C(NORBS,NEND).
      nend = max(nopen,nelec + nmos) 
      l = 1 
      do i = 1, nend 
        call supdot (dp(l), foc2, c(1,i), norbs) 
        l = l + norbs 
      end do 
!     EXTRACT FCI
      l = 1 
      nend = 0 
      do loop = 1, 3 
        ninit = nend + 1 
        nend = nend + nbo(loop) 
        n1 = max(ninit,nelec + 1) 
        n2 = min(nend,nelec + nmos) 
        if (n2 < n1) cycle  
        do i = n1, n2 
          if (i <= ninit) cycle  
          call mxm (c(1,i), 1, dp(norbs*(ninit-1)+1), norbs, fci(l), i - ninit) 
          l = l + i - ninit 
        end do 
      end do 
      do i = nelec + 1, nelec + nmos 
        fci(l) = -dot(c(1,i),dp(norbs*(i-1)+1),norbs) 
        l = l + 1 
      end do 
!
!     NEW SUPERVECTOR AB = (DIAG + C'* FOC2 * C) * B , SCALED.
!     --------------------------------------------------------
!
!     PART 1 : AB(I,J) = (C' * DP)(I,J) DONE BY BLOCKS.
      l = 1 
      if (nbo(2)/=0 .and. nbo(1)/=0) then 
        call mtxm (c(1,nbo(1)+1), nbo(2), dp, norbs, ab(l), nbo(1)) 
        l = l + nbo(2)*nbo(1) 
      endif 
      if (nbo(3)/=0 .and. nbo(1)/=0) then 
        call mtxm (c(1,nopen+1), nbo(3), dp, norbs, ab(l), nbo(1)) 
        l = l + nbo(3)*nbo(1) 
      endif 
      if (nbo(3)/=0 .and. nbo(2)/=0) call mtxm (c(1,nopen+1), nbo(3), dp(norbs*&
        nbo(1)+1), norbs, ab(l), nbo(2)) 
!
!     PART 2 : AB = SCALE * (D * B + AB) AND RESCALE BASIS VECTOR B.
      ab = (diag(:minear)*b+ab)*scalar(:minear) 
      b = b/scalar(:minear) 
      return  
      end subroutine deri22 
