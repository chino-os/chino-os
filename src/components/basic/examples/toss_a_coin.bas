100 REM TOSS A COIN
110 C=INT(RND(1)*2)
120 IF C=0 THEN GOTO 180
130 PRINT "TAILS"
140 PRINT "TRY AGAIN?";
150 INPUT T$
160 IF T$="Y" THEN GOTO 110
170 END
180 PRINT "HEADS"
190 GOTO 140