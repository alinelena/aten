      MODULE fockd1_I   
      INTERFACE
!...Generated by Pacific-Sierra Research 77to90  4.4G  12:41:19  03/10/06  
      SUBROUTINE fockd1 (F, PTOT, PA, W) 
      USE vast_kind_param,ONLY: DOUBLE 
      use molkst_C, only : lm6
      REAL(DOUBLE), DIMENSION(*), INTENT(INOUT) :: F 
      REAL(DOUBLE), DIMENSION(*), INTENT(IN) :: PTOT 
      REAL(DOUBLE), DIMENSION(*), INTENT(IN) :: PA 
      REAL(DOUBLE), DIMENSION(LM6,LM6), INTENT(IN) :: W 
      END SUBROUTINE  
      END INTERFACE 
      END MODULE 
