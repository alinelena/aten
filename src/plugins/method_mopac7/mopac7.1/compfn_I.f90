      MODULE compfn_I   
      INTERFACE
!...Generated by Pacific-Sierra Research 77to90  4.4G  12:18:48  03/10/06  
      SUBROUTINE compfn (XPARAM, INT, ESCF, FULSCF, GRAD, LGRAD, W, H, HQ, WJ&
        , WK) 
      USE vast_kind_param,ONLY: DOUBLE 
      REAL(DOUBLE), DIMENSION(*), INTENT(IN) :: XPARAM 
      LOGICAL, INTENT(IN) :: INT 
      REAL(DOUBLE), INTENT(INOUT) :: ESCF 
      LOGICAL :: FULSCF 
      REAL(DOUBLE), DIMENSION(*), INTENT(IN) :: GRAD 
      LOGICAL, INTENT(IN) :: LGRAD 
      REAL(DOUBLE), DIMENSION(*) :: W, H, HQ, WJ, WK
      END SUBROUTINE  
      END INTERFACE 
      END MODULE 
