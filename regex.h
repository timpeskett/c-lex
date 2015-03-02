#ifndef REGEX_H
#define REGEX_H

#include <stdio.h>
#include <ctype.h>
#include "global.h"
#include "nfa.h"

void regexInit( void );

Dfa regexToDFA( char *inRegEx );

void regexDestroy( void );

#endif
