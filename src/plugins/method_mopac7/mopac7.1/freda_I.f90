      MODULE freda_I   
      INTERFACE
!...Generated by Pacific-Sierra Research 77to90  4.4G  11:04:14  03/09/06  
      SUBROUTINE freda (L, D, A, E) 
      USE vast_kind_param,ONLY: DOUBLE 
      integer, INTENT(IN) :: L 
      real(DOUBLE), DIMENSION(L), INTENT(IN) :: D 
      real(DOUBLE), DIMENSION(*), INTENT(INOUT) :: A 
      real(DOUBLE), DIMENSION(L), INTENT(IN) :: E 
      END SUBROUTINE  
      END INTERFACE 
      END MODULE 
