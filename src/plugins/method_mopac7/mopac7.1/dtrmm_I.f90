      MODULE dtrmm_I   
      INTERFACE
!...Generated by Pacific-Sierra Research 77to90  4.4G  22:48:56  03/08/06  
      SUBROUTINE dtrmm (SIDE, UPLO, TRANSA, DIAG, M, N, ALPHA, A, LDA, B, LDB) 
      USE vast_kind_param,ONLY: DOUBLE 
      character (LEN = *) :: SIDE 
      character (LEN = *) :: UPLO 
      character (LEN = *) :: TRANSA 
      character (LEN = *) :: DIAG 
      integer, INTENT(IN) :: M 
      integer, INTENT(IN) :: N, LDA, LDB
      real(DOUBLE), INTENT(IN) :: ALPHA 
      real(DOUBLE), DIMENSION(LDA,*), INTENT(IN) :: A 
      real(DOUBLE), DIMENSION(LDB,*), INTENT(INOUT) :: B 
      END SUBROUTINE  
      END INTERFACE 
      END MODULE 
