  	/  �   k820309    v          9.0         ƴ�F                                                                                                       
       SDEintegration.f90 SDEINTEGRATION          SDE_INIT SDE_FINAL SDE_CALLONCLASSIFYFAST SDE_BROWNIANPATH_GENERATE SDE_BROWNIANPATH_DEALLOCATEROWS SDE_BROWNIANPATH_DEALLOCATETREE SDE_DRIFTDIFFUSION SDE_MILSTEIN_2DTERMS SDE_EXPLICIT_MILSTEIN SDE_EXPLICIT_EULERMARUYAMA SDE_CHECK_CRITERIA SDE_ADAPTIVE_TIME_STEPPING PATHSIZE MAX_TREE_ROW TREE_ROW BRANCH BROWNIANPATHTYPE BROWNIAN_PATHS                                         
                                               
                        @                      
                     @                        'X           #C    #MTYPE    #DATA    #SLISTLEN    #DLISTLEN 	   #ELISTLEN 
   #DLIST    #ELIST    #ESTOICHLIST    #SLIST    #V             �                                        
  p      p        p                       �                            0                  �                            4                  �                            8                  �                       	     <                  �                       
     @                  �                               D           p      p        p                       �                               \           p      p        p                       �                               t      	     p      p        p                       �                               �      
     p      p        p                       �                               �           p      p        p              #     @                                       	    #NORMAL_RAND%REAL    #NORMAL_RAND%PACK    #NORMAL_RAND%COUNT    #NORMAL_RAND%SQRT    #NORMAL_RAND%LOG    #NORMAL_RAND%MAX    #NORMAL_RAND%FLOOR    #NORMAL_RAND%MIN    #NORMAL_RAND%UNPACK    #NRGN    #MASK                             @           REAL                                      PACK                                      COUNT                                      SQRT                                      LOG                                      MAX                                      FLOOR                                      MIN                                      UNPACK                                          
           &                             
                                               &                                                                                                                                                X           &                       #RXNDATATYPE    #     @                                           #DERIVATIVES_PROPENSITIES%REAL     #DERIVATIVES_PROPENSITIES%SIZE !   #MASK "   #X #   #DER_ANEW $   #VRATIO %             @              @           REAL           @                    !     SIZE       
  @                      "                     &                             
   @                     #           
          &                             
  @                     $           
           &           &                             
   @                     %     
  #     @                          &                  #SDE_INIT%SQRT '   #SDE_INIT%REAL (   #M )   #N *   #NEWMODEL +             @                    '     SQRT           @             @      (     REAL       
   @                      )             
   @                      *             
   @                      +       #     @                          ,                  #SDE_FINAL%ALLOCATED -   #NEWMODEL .             @                    -     ALLOCATED       
   @                      .       #     @                          /                  #SDE_CALLONCLASSIFYFAST%ALL 0   #SDE_CALLONCLASSIFYFAST%ANY 1   #SDE_CALLONCLASSIFYFAST%SQRT 2   #SDE_CALLONCLASSIFYFAST%ALLOCATED 3   #SDE_CALLONCLASSIFYFAST%MAX 4   #SDE_CALLONCLASSIFYFAST%COUNT 5   #SDE_CALLONCLASSIFYFAST%REAL 6   #M 7   #FASTINDEX 8   #FASTINVERSEINDEX 9   #BEFOREFASTINDEX :   #BEFOREFASTINVERSEINDEX ;   #BEFOREFAST <   #AFTERFAST =             @                    0     ALL           @                    1     ANY           @                    2     SQRT           @                    3     ALLOCATED           @                    4     MAX           @                    5     COUNT           @             @      6     REAL       
   @                      7             
   @                      8                     &                             
   @                      9                     &                             
   @                      :                     &                             
   @                      ;                     &                             
  @@                      <                     &                             
  @@                      =                     &                       #     @                         >                  #SDE_BROWNIANPATH_GENERATE%SQRT ?   #SDE_BROWNIANPATH_GENERATE%ALLOCATED @   #SDE_BROWNIANPATH_GENERATE%COUNT A   #SDE_BROWNIANPATH_GENERATE%REAL B   #FAST C   #CURRENT_ROW D   #FINAL_ROW E   #RESET F             @                    ?     SQRT           @                    @     ALLOCATED           @                    A     COUNT           @             @      B     REAL       
  @@                      C                     &                             
   @                      D             
   @                      E             
   @                      F       #     @                          G                   #STARTROW H   #ENDROW I         
   @                      H             
   @                      I       #     @                         J                   #SDE_BROWNIANPATH_DEALLOCATETREE%ALLOCATED K             @                    K     ALLOCATED #     @                          L                  #SDE_DRIFTDIFFUSION%SQRT M   #FAST N   #FAST_INDEX O   #A P   #DELTAT Q   #DW R   #RXNEM S             @                    M     SQRT       
   A                      N                     &                             
   A                      O                     &                             
   A                     P           
          &                             
   @                     Q     
        
   A                     R           
          &                             D  A                     S           
           &                       #     @                          T                  #SDE_MILSTEIN_2DTERMS%SIZE U   #SDE_MILSTEIN_2DTERMS%SQRT V   #SDE_MILSTEIN_2DTERMS%SUM W   #SDE_MILSTEIN_2DTERMS%COUNT X   #SDE_MILSTEIN_2DTERMS%REAL Y   #FAST Z   #FASTSPECIES [   #FAST_INDEX \   #A ]   #DER_A ^   #DELTAT _   #DW `             @                    U     SIZE           @                    V     SQRT           @                    W     SUM           @                    X     COUNT           @             @      Y     REAL       
  @@                      Z                     &                             
   A                      [                      &                             
   @                      \                     &                             
   @                     ]           
          &                             
   @                     ^           
          &           &                             
  @@                     _     
        
   @                     `           
          &                       #     @                          a                  #SDE_EXPLICIT_MILSTEIN%REAL b   #FAST c   #FASTSPECIES d   #N e   #RXNEM f   #DXFAST g             @             @      b     REAL       
   @                      c            7         &                             
   A                      d            8         &                             
   @                      e             
   @                     f           
 9         &                             D  A                     g           
 :          &                       #     @                          h                  #SDE_EXPLICIT_EULERMARUYAMA%REAL i   #FAST j   #M k   #RXNEM l   #DXFAST m             @             @      i     REAL       
   @                      j            4         &                             
   @                      k             
   @                     l           
 5         &                             D  @                     m           
 6          &                       #     @                          n               
   #SDE_CHECK_CRITERIA%MATMUL o   #SDE_CHECK_CRITERIA%TRANSPOSE p   #SDE_CHECK_CRITERIA%ABS q   #SDE_CHECK_CRITERIA%MAXVAL r   #SDE_CHECK_CRITERIA%SQRT s   #SDE_CHECK_CRITERIA%SUM t   #SDE_CHECK_CRITERIA%COUNT u   #SDE_CHECK_CRITERIA%REAL v   #FAST w   #FASTSPECIES x   #X y   #DELTAT z   #DW {   #A |   #DER_A }   #VDIM ~   #TOL    #ERROROK �             @                    o     MATMUL           @                    p     TRANSPOSE           @                    q     ABS           @                    r     MAXVAL           @                    s     SQRT           @                    t     SUM           @                    u     COUNT           @             @      v     REAL       
  @@                      w            ?         &                             
   @                      x            @         &                             
   @                     y           
 ;         &                             
   @                     z     
        
   @                     {           
 <         &                             
   @                     |           
 =         &                             
   @                     }           
 >         &           &                             
  @@                     ~     
        
   @                          
        D  @                      �        #     @                          �                  #SDE_ADAPTIVE_TIME_STEPPING%MOD �   #ERROROK �   #TREE_ROW �   #BRANCH �             @                    �     MOD       
   @                      �             
D  @                      �              
D @@                      �               @                         �               @                         �               @ @                       �               @                         �                      @               @   �     '$            #ROW �          � $                     �                  
        &                            @ @!                      �        $            &                       #BROWNIANPATHTYPE �      �   *      fn#fn $   �   ]  b   uapp(SDEINTEGRATION    #  4   J  RANDOMGEN     W  4   J  GLOBALVARIABLES    �  4   J  RATELAWS ,   �  �       RXNDATATYPE+GLOBALVARIABLES .   z  p   a   RXNDATATYPE%C+GLOBALVARIABLES 2   �  8   a   RXNDATATYPE%MTYPE+GLOBALVARIABLES 1   "  8   a   RXNDATATYPE%DATA+GLOBALVARIABLES 5   Z  8   a   RXNDATATYPE%SLISTLEN+GLOBALVARIABLES 5   �  8   a   RXNDATATYPE%DLISTLEN+GLOBALVARIABLES 5   �  8   a   RXNDATATYPE%ELISTLEN+GLOBALVARIABLES 2     p   a   RXNDATATYPE%DLIST+GLOBALVARIABLES 2   r  p   a   RXNDATATYPE%ELIST+GLOBALVARIABLES 8   �  p   a   RXNDATATYPE%ESTOICHLIST+GLOBALVARIABLES 2   R  p   a   RXNDATATYPE%SLIST+GLOBALVARIABLES .   �  p   a   RXNDATATYPE%V+GLOBALVARIABLES &   2        NORMAL_RAND+RANDOMGEN +   I  1      NORMAL_RAND%REAL+RANDOMGEN +   z  1      NORMAL_RAND%PACK+RANDOMGEN ,   �  2      NORMAL_RAND%COUNT+RANDOMGEN +   �  1      NORMAL_RAND%SQRT+RANDOMGEN *   	  0      NORMAL_RAND%LOG+RANDOMGEN *   >	  0      NORMAL_RAND%MAX+RANDOMGEN ,   n	  2      NORMAL_RAND%FLOOR+RANDOMGEN *   �	  0      NORMAL_RAND%MIN+RANDOMGEN -   �	  3      NORMAL_RAND%UNPACK+RANDOMGEN +   
  `   a   NORMAL_RAND%NRGN+RANDOMGEN +   c
  `   a   NORMAL_RAND%MASK+RANDOMGEN "   �
  0       M+GLOBALVARIABLES "   �
  0       N+GLOBALVARIABLES (   #  m       RXNDATA+GLOBALVARIABLES 2   �  �       DERIVATIVES_PROPENSITIES+RATELAWS <   =  1      DERIVATIVES_PROPENSITIES%REAL+RATELAWS=REAL <   n  1      DERIVATIVES_PROPENSITIES%SIZE+RATELAWS=SIZE 7   �  `   e   DERIVATIVES_PROPENSITIES%MASK+RATELAWS 4   �  `   e   DERIVATIVES_PROPENSITIES%X+RATELAWS ;   _  p   e   DERIVATIVES_PROPENSITIES%DER_ANEW+RATELAWS 9   �  0   e   DERIVATIVES_PROPENSITIES%VRATIO+RATELAWS    �  ~       SDE_INIT    }  1      SDE_INIT%SQRT    �  1      SDE_INIT%REAL    �  0   a   SDE_INIT%M      0   a   SDE_INIT%N "   ?  0   a   SDE_INIT%NEWMODEL    o  c       SDE_FINAL $   �  6      SDE_FINAL%ALLOCATED #     0   a   SDE_FINAL%NEWMODEL '   8  �      SDE_CALLONCLASSIFYFAST +   �  0      SDE_CALLONCLASSIFYFAST%ALL +   
  0      SDE_CALLONCLASSIFYFAST%ANY ,   :  1      SDE_CALLONCLASSIFYFAST%SQRT 1   k  6      SDE_CALLONCLASSIFYFAST%ALLOCATED +   �  0      SDE_CALLONCLASSIFYFAST%MAX -   �  2      SDE_CALLONCLASSIFYFAST%COUNT ,     1      SDE_CALLONCLASSIFYFAST%REAL )   4  0   a   SDE_CALLONCLASSIFYFAST%M 1   d  `   a   SDE_CALLONCLASSIFYFAST%FASTINDEX 8   �  `   a   SDE_CALLONCLASSIFYFAST%FASTINVERSEINDEX 7   $  `   a   SDE_CALLONCLASSIFYFAST%BEFOREFASTINDEX >   �  `   a   SDE_CALLONCLASSIFYFAST%BEFOREFASTINVERSEINDEX 2   �  `   a   SDE_CALLONCLASSIFYFAST%BEFOREFAST 1   D  `   a   SDE_CALLONCLASSIFYFAST%AFTERFAST *   �        SDE_BROWNIANPATH_GENERATE /   �  1      SDE_BROWNIANPATH_GENERATE%SQRT 4   �  6      SDE_BROWNIANPATH_GENERATE%ALLOCATED 0     2      SDE_BROWNIANPATH_GENERATE%COUNT /   D  1      SDE_BROWNIANPATH_GENERATE%REAL /   u  `   a   SDE_BROWNIANPATH_GENERATE%FAST 6   �  0   a   SDE_BROWNIANPATH_GENERATE%CURRENT_ROW 4     0   a   SDE_BROWNIANPATH_GENERATE%FINAL_ROW 0   5  0   a   SDE_BROWNIANPATH_GENERATE%RESET 0   e  V       SDE_BROWNIANPATH_DEALLOCATEROWS 9   �  0   a   SDE_BROWNIANPATH_DEALLOCATEROWS%STARTROW 7   �  0   a   SDE_BROWNIANPATH_DEALLOCATEROWS%ENDROW 0     k       SDE_BROWNIANPATH_DEALLOCATETREE :   �  6      SDE_BROWNIANPATH_DEALLOCATETREE%ALLOCATED #   �  �       SDE_DRIFTDIFFUSION (   U  1      SDE_DRIFTDIFFUSION%SQRT (   �  `   a   SDE_DRIFTDIFFUSION%FAST .   �  `   a   SDE_DRIFTDIFFUSION%FAST_INDEX %   F  `   a   SDE_DRIFTDIFFUSION%A *   �  0   a   SDE_DRIFTDIFFUSION%DELTAT &   �  `   a   SDE_DRIFTDIFFUSION%DW )   6  `   a   SDE_DRIFTDIFFUSION%RXNEM %   �  (      SDE_MILSTEIN_2DTERMS *   �  1      SDE_MILSTEIN_2DTERMS%SIZE *   �  1      SDE_MILSTEIN_2DTERMS%SQRT )      0      SDE_MILSTEIN_2DTERMS%SUM +   P  2      SDE_MILSTEIN_2DTERMS%COUNT *   �  1      SDE_MILSTEIN_2DTERMS%REAL *   �  `   a   SDE_MILSTEIN_2DTERMS%FAST 1     `   a   SDE_MILSTEIN_2DTERMS%FASTSPECIES 0   s  `   a   SDE_MILSTEIN_2DTERMS%FAST_INDEX '   �  `   a   SDE_MILSTEIN_2DTERMS%A +   3   p   a   SDE_MILSTEIN_2DTERMS%DER_A ,   �   0   a   SDE_MILSTEIN_2DTERMS%DELTAT (   �   `   a   SDE_MILSTEIN_2DTERMS%DW &   3!  �       SDE_EXPLICIT_MILSTEIN +   �!  1      SDE_EXPLICIT_MILSTEIN%REAL +   �!  `   a   SDE_EXPLICIT_MILSTEIN%FAST 2   Y"  `   a   SDE_EXPLICIT_MILSTEIN%FASTSPECIES (   �"  0   a   SDE_EXPLICIT_MILSTEIN%N ,   �"  `   a   SDE_EXPLICIT_MILSTEIN%RXNEM -   I#  `   a   SDE_EXPLICIT_MILSTEIN%DXFAST +   �#  �       SDE_EXPLICIT_EULERMARUYAMA 0   2$  1      SDE_EXPLICIT_EULERMARUYAMA%REAL 0   c$  `   a   SDE_EXPLICIT_EULERMARUYAMA%FAST -   �$  0   a   SDE_EXPLICIT_EULERMARUYAMA%M 1   �$  `   a   SDE_EXPLICIT_EULERMARUYAMA%RXNEM 2   S%  `   a   SDE_EXPLICIT_EULERMARUYAMA%DXFAST #   �%  �      SDE_CHECK_CRITERIA *   G'  3      SDE_CHECK_CRITERIA%MATMUL -   z'  6      SDE_CHECK_CRITERIA%TRANSPOSE '   �'  0      SDE_CHECK_CRITERIA%ABS *   �'  3      SDE_CHECK_CRITERIA%MAXVAL (   (  1      SDE_CHECK_CRITERIA%SQRT '   D(  0      SDE_CHECK_CRITERIA%SUM )   t(  2      SDE_CHECK_CRITERIA%COUNT (   �(  1      SDE_CHECK_CRITERIA%REAL (   �(  `   a   SDE_CHECK_CRITERIA%FAST /   7)  `   a   SDE_CHECK_CRITERIA%FASTSPECIES %   �)  `   a   SDE_CHECK_CRITERIA%X *   �)  0   a   SDE_CHECK_CRITERIA%DELTAT &   '*  `   a   SDE_CHECK_CRITERIA%DW %   �*  `   a   SDE_CHECK_CRITERIA%A )   �*  p   a   SDE_CHECK_CRITERIA%DER_A (   W+  0   a   SDE_CHECK_CRITERIA%VDIM '   �+  0   a   SDE_CHECK_CRITERIA%TOL +   �+  0   a   SDE_CHECK_CRITERIA%ERROROK +   �+  �       SDE_ADAPTIVE_TIME_STEPPING /   n,  0      SDE_ADAPTIVE_TIME_STEPPING%MOD 3   �,  0   a   SDE_ADAPTIVE_TIME_STEPPING%ERROROK 4   �,  0   a   SDE_ADAPTIVE_TIME_STEPPING%TREE_ROW 2   �,  0   a   SDE_ADAPTIVE_TIME_STEPPING%BRANCH    .-  0       PATHSIZE    ^-  0       MAX_TREE_ROW    �-  0       TREE_ROW    �-  0       BRANCH !   �-  E       BROWNIANPATHTYPE %   3.  d   a   BROWNIANPATHTYPE%ROW    �.  r       BROWNIAN_PATHS 