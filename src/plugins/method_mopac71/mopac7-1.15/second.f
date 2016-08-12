      FUNCTION SECOND()
      DOUBLE PRECISION SECOND
C******************************************************
C
C   SECOND, ON EXIT, CONTAINS THE NUMBER OF CPU SECONDS
C   SINCE THE START OF THE CALCULATION.
C
C******************************************************
      COMMON /OUTFIL/ WU
      INTEGER WU
      LOGICAL SETOK
      CHARACTER*1 X, GETNAM*80
      DIMENSION A(2)
      SAVE SETOK, SHUT
      DATA SETOK   /  .TRUE.    /, SHUT/0.D0/
C
C   IF YOU ARE NOT USING A VAX OR A UNIX COMPUTER, UNCOMMENT THE 
C   NEXT LINE
C     CPU=0.0
C
C   IF YOU ARE NOT USING A VAX OR A UNIX COMPUTER, REMOVE THE NEXT LINE
      Y=ETIME(A)
      CPU=A(1)
***********************************************************************
*
*   NOW TO SEE IF A FILE LOGICALLY CALLED SHUTDOWN EXISTS, IF IT DOES
*   THEN INCREMENT CPU TIME BY 1,000,000 SECONDS.
*
************************************************************************
      OPEN(UNIT=4, FILE=GETNAM('SHUTDOWN'),STATUS='UNKNOWN')
      READ(4,'(A)',END=10, ERR=10)X
*
*          FILE EXISTS, THEREFORE INCREMENT TIME
*
      SHUT=1.D6
      IF( SETOK) THEN
         WRITE(WU,'(///10X,''****   JOB STOPPED BY OPERATOR   ****'')')
         SETOK=.FALSE.
      ENDIF
   10 CONTINUE
      SECOND=CPU+SHUT
      CLOSE(4)
      RETURN
      END
      CHARACTER*80 FUNCTION GETNAM(NAMEIN)
C
C    THIS FUNCTION IS DESIGNED FOR USE ON A VMS AND ON A UNIX
C    SYSTEM.  IF YOUR SYSTEM IS VMS, COMMENT OUT THE LINE
C    "      CALL GETENV(NAMEIN, NAMEOUT)", FURTHER ON IN THIS FUNCTION.
C    IF YOUR SYSTEM IS UNIX, MAKE SURE THE LINE IS NOT
C    COMMENTED OUT.
C    ON A UNIX SYSTEM, GETENV WILL CONSULT THE ENVIRONMENT
C    FOR THE CURRENT ALIAS OF THE CHARACTER STRING CONTAINED IN
C    'NAMEIN'.  THE ALIAS, IF IT EXISTS, OR THE ORIGINAL NAME IN NAMEIN
C    WILL BE RETURNED.
C
      CHARACTER*(*) NAMEIN
      CHARACTER*(80) NAMEOUT
      NAMEOUT=' '
      CALL GETENV(NAMEIN, NAMEOUT)
      IF (NAMEOUT.EQ.'  ') NAMEOUT=NAMEIN
      GETNAM = NAMEOUT
      RETURN
      END