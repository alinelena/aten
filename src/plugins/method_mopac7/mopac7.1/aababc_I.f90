      MODULE aababc_I   
      INTERFACE
!...Generated by Pacific-Sierra Research 77to90  4.4G  10:46:58  03/09/06  
      REAL(KIND(0.0D0)) FUNCTION aababc (IOCCA1, IOCCB1, IOCCA2, NMOS, XY) 
      USE vast_kind_param,ONLY: DOUBLE 
      INTEGER, INTENT(IN) :: NMOS 
      INTEGER, DIMENSION(NMOS), INTENT(IN) :: IOCCA1 
      INTEGER, DIMENSION(NMOS), INTENT(IN) :: IOCCB1 
      INTEGER, DIMENSION(NMOS), INTENT(IN) :: IOCCA2 
      REAL(DOUBLE), DIMENSION(NMOS,NMOS,NMOS,NMOS), INTENT(IN) :: XY 
      END FUNCTION  
      END INTERFACE 
      END MODULE 
