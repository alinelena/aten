      MODULE charmo_I   
      INTERFACE
!...Generated by Pacific-Sierra Research 77to90  4.4G  10:47:03  03/09/06  
      REAL(KIND(0.0D0)) FUNCTION charmo (VECTS, NTYPE, JORB, IOPER, R, NVECS, FIRST) 
      USE vast_kind_param,ONLY: DOUBLE
      INTEGER, INTENT(IN) :: NVECS
      REAL(DOUBLE), DIMENSION(NVECS,NVECS), INTENT(IN) :: VECTS 
      INTEGER, DIMENSION(*), INTENT(IN) :: NTYPE 
      INTEGER, INTENT(IN) :: JORB 
      INTEGER, INTENT(IN) :: IOPER 
      REAL(DOUBLE), DIMENSION(3,3), INTENT(IN) :: R 
      LOGICAL, intent(inout) :: FIRST 
      END FUNCTION  
      END INTERFACE 
      END MODULE 
