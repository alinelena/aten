      MODULE spcore_I   
      INTERFACE
!...Generated by Pacific-Sierra Research 77to90  4.4G  12:41:19  03/10/06  
      SUBROUTINE spcore (NI, NJ, R, CORE) 
      USE vast_kind_param,ONLY: DOUBLE 
      integer, INTENT(IN) :: NI 
      integer, INTENT(IN) :: NJ 
      real(DOUBLE), INTENT(IN) :: R 
      real(DOUBLE), DIMENSION(10,2), INTENT(OUT) :: CORE 
      END SUBROUTINE  
      END INTERFACE 
      END MODULE 
