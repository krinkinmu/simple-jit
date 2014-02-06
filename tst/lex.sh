#!/bin/bash

TESTER="`readlink -e $0`"
LEXER="`readlink -e $1`"
TESTS="`dirname $TESTER`/lex"

INPUTS=`ls $TESTS | grep .*\.input | sed -e 's/.input//'`

for TEST in $INPUTS
do
	RESULT=`$LEXER "$TESTS/$TEST.input"`
	EXPECTED=`cat "$TESTS/$TEST.output"`

	if [ "$RESULT" == "$EXPECTED" ]
	then
		echo "TEST $TEST PASSED"
	else
		echo "TEST $TEST FAILED"
		exit
	fi
done 
