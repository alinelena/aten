      MODULE drcout_I   
      INTERFACE
!...Generated by Pacific-Sierra Research 77to90  4.4G  10:47:10  03/09/06  
      SUBROUTINE drcout (XYZ3, GEO3, VEL3, NVAR, TIME, ESCF3, EKIN3, ETOT3&
        , XTOT3, ILOOP, CHARGE, FRACT, TEXT1, TEXT2, II, JLOOP) 
      USE vast_kind_param,ONLY: DOUBLE 
      INTEGER, INTENT(IN) :: NVAR 
      REAL(DOUBLE), DIMENSION(3,NVAR), INTENT(IN) :: XYZ3 
      REAL(DOUBLE), DIMENSION(3,*), INTENT(IN) :: GEO3 
      REAL(DOUBLE), DIMENSION(3,NVAR), INTENT(IN) :: VEL3       
      REAL(DOUBLE), INTENT(IN) :: TIME 
      REAL(DOUBLE), DIMENSION(3), INTENT(IN) :: ESCF3 
      REAL(DOUBLE), DIMENSION(3), INTENT(IN) :: EKIN3 
      REAL(DOUBLE), DIMENSION(3), INTENT(IN) :: ETOT3 
      REAL(DOUBLE), DIMENSION(3), INTENT(IN) :: XTOT3 
      INTEGER, INTENT(IN) :: ILOOP 
      REAL(DOUBLE), DIMENSION(*), INTENT(IN) :: CHARGE 
      REAL(DOUBLE), INTENT(IN) :: FRACT 
      CHARACTER (LEN = 3), INTENT(IN) :: TEXT1 
      CHARACTER (LEN = 2), INTENT(IN) :: TEXT2 
      INTEGER, INTENT(IN) :: II 
      INTEGER, INTENT(INOUT) :: JLOOP 
      END SUBROUTINE  
      END INTERFACE 
      END MODULE 
