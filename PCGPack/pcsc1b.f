      SUBROUTINEPCSC1B(IBLKN,IBTRM,IRTBM,IBTRMC,NBLCKB,C,IC,IJA
     *,A,V2,V1)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERIBLKN,IBTRM(*),IRTBM(*),IBTRMC(*),NBLCKB,C(*),
     *IC(*),IJA(*)
      DOUBLEPRECISIONA(*),V2(*),V1(*)
      INTEGERI,II,J,K,NBLACK,JJ,MI,MJ,KK,MM
      NBLACK=IBTRMC(NBLCKB+1)-1
      DO1I=1,IBLKN
      II=IC(I)
      KK=IBTRMC(II)-IBTRM(II)
      DO3K=IBTRM(II),IBTRM(II+1)-1
      MI=KK+K
CDIR$ IVDEP
CVDIR IGNORE RECRDEPS
      DO5J=IJA(K),IJA(K+1)-1
      MM=IRTBM(IJA(J))
      JJ=IC(MM)
      MJ=IBTRMC(JJ)+IJA(J)-IBTRM(MM)
      IF(MI.GT.NBLACK.AND.MJ.LE.NBLACK)THEN
      V1(MJ)=V1(MJ)-A(J)*V2(MI-NBLACK)
      ELSE
      IF(MI.LE.NBLACK.AND.MJ.GT.NBLACK)THEN
      V1(MI)=V1(MI)-A(J)*V2(MJ-NBLACK)
      ENDIF
      ENDIF
5     CONTINUE
3     CONTINUE
1     CONTINUE
      RETURN
      END