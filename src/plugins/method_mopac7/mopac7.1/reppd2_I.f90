      MODULE reppd2_I   
      INTERFACE
!...Generated by Pacific-Sierra Research 77to90  4.4G  12:41:19  03/10/06  
      SUBROUTINE reppd2 (NI, NJ, R, RI, REP, CORE) 
      USE vast_kind_param,ONLY: DOUBLE 
      integer, INTENT(IN) :: NI 
      integer, INTENT(IN) :: NJ 
      real(DOUBLE) :: R 
      real(DOUBLE), DIMENSION(22), INTENT(IN) :: RI 
      real(DOUBLE), DIMENSION(491), INTENT(OUT) :: REP 
      real(DOUBLE), DIMENSION(10,2), INTENT(OUT) :: CORE 
      END SUBROUTINE  
      END INTERFACE 
      END MODULE 
