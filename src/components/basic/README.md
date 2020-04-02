![BASIC](./basic.png)

A from-scratch BASIC interpreter with a focus on being easy to extend and port.

[![asciicast](https://asciinema.org/a/37018.png)](https://asciinema.org/a/37018)

# Introduction

This is a BASIC interpreter written from scratch in C. For the moment two architectures are supported, POSIX and AVR XMega via the avr-gcc toolchain.

Most of the common BASIC keywords are supported:

  * PRINT expression-list [ ; ]
  * IF relation-expression THEN statement
  * GOTO expression
  * INPUT variable-list
  * LET variable = expression
  * GOSUB expression
  * RETURN
  * FOR numeric\_variable '=' numeric\_expression TO numeric_expression [ STEP number ] 
  * CLEAR, NEW
  * LIST
  * RUN
  * END
  * DIM variable "(" expression ")"
  * ABS, AND, ATN, COS, EXP, INT, LOG, NOT, OR, RND, SGN, SIN, SQR, TAN
  * LEN, CHR$, MID$, LEFT$, RIGHT$, ASC 

# Extend

It should be easy to register a new BASIC function, as shown here with a `sleep` function for the XMEGA.

```C
register_function_1(basic_function_type_keyword, "SLEEP", do_sleep, kind_numeric);
...
int do_sleep(basic_type* delay, basic_type* rv)
{
  delay_ms(delay->value.number);
  
  rv->kind = kind_numeric;
  rv->value.number = 0;

  return 0;
}
```

Let's use that new keyword.

```REALbasic
10 INPUT "YOUR NAME?", NAME$
20 CLS
30 FOR I=1 TO LEN(NAME$)
40 PRINT MID$(NAME$,I,1); 
50 SLEEP(500)
60 NEXT
70 PRINT
```

# Port

It should be easy to port the interpreter to other architectures. As an example there is a port to an XMega 128A4U included using the [Batsocks breadmate board](http://www.batsocks.co.uk/products/BreadMate/XMega%20PDI%20AV.htm).

# Use

There is a simple REPL for the BASIC interpreter. You can use it in an interactive way, just as you would do on a 80's era computer.

```
 _               _
| |__   __ _ ___(_) ___
| '_ \ / _` / __| |/ __|
| |_) | (_| \__ \ | (__
|_.__/ \__,_|___/_|\___|
(c) 2015-2016 Johan Van den Brande
10 PRINT "HELLO"
20 GOTO 10
```

You can give it a BASIC file on the command line.

```
$> basic examples/diamond.bas
         *
        ***
       *****
      *******
     *********
    ***********
   *************
  ***************
 *****************
  ***************
   *************
    ***********
     *********
      *******
       *****
        ***
         *
```

You can also use the shebang operator to make standalone BASIC scripts

```
#!/usr/bin/env basic

10 RADIUS=10
20 FOR I=1 TO RADIUS-1
30 W=INT(RADIUS*SIN(180/RADIUS*I*3.1415/180))
40 PRINT SPC(RADIUS-W);:FOR J=1 TO 2*W:PRINT "*";:NEXT J:PRINT
50 NEXT I
```

```
$> ./examples/circle.bas
       ******
     **********
  ****************
 ******************
********************
 ******************
  ****************
     **********
       ******
```

It is easy to embed the interpreter into your own application.

```C
  basic_init(2048, 512); // memory size, stack size
  basic_register_io(putchar, getchar);
  basic_eval("10 PRINT \"HELLO\"");
  basic_eval("RUN"); 
  basic_destroy();  
```

On OSX/POSIX you can use the 'BASIC\_PATH' environment variable to set the folder used for loading and saving BASIC programs. The 'BASIC\_PATH' defaults to '.'.
BASIC programs are expected to end with '.bas'. You can use LOAD, SAVE, DELETE and DIR.

# Copyright

(c) 2015 - 2016 Johan Van den Brande
