#include <stdio.h>
#include <stdlib.h>

#include <io.h>

const char *last_error = NULL;

void error(const char *error_msg) { last_error = error_msg; }
