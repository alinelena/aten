      MODULE xyzint_I   
      INTERFACE
!...Generated by Pacific-Sierra Research 77to90  4.4G  11:04:12  03/09/06  
      SUBROUTINE xyzint (XYZ, numat, NA, NB, NC, DEGREE, GEO) 
      USE vast_kind_param,ONLY: DOUBLE 
      REAL(DOUBLE), DIMENSION(3,NUMAT), INTENT(IN) :: XYZ  
      INTEGER, DIMENSION(NUMAT), INTENT(INOUT) :: NA 
      INTEGER, DIMENSION(NUMAT), INTENT(INOUT) :: NB 
      INTEGER, DIMENSION(NUMAT), INTENT(OUT) :: NC 
      REAL(DOUBLE) :: DEGREE 
      REAL(DOUBLE), DIMENSION(3,NUMAT), INTENT(OUT) :: GEO 
      END SUBROUTINE  
      END INTERFACE 
      END MODULE 
