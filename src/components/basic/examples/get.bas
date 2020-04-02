100 PRINT "Type max. 20 chars, please: ";:GOSUB 1000
200 PRINT: PRINT "You have written: ";B$
300 END
1000 REM simple GET-INPUT-Routine
1010 A$="": B$="": A=0: REM initialize variables
1020 GET A$: IF A$="" THEN 1020
1030 A=ASC(A$): IF (A<32 OR A>127) AND A<>13 THEN 1020
1040 REM B$=B$+A$: PRINT A$;: IF LEN(B$)<20 AND A$<>CHR$(13) THEN 1020
1045 B$=B$+A$: IF LEN(B$)<20 AND A$<>CHR$(13) THEN 1020
1050 RETURN
