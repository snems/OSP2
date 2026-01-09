#!/bin/bash
RET="$(cppcheck -D__GNUC__ -q -j32 --enable=warning,performance,portability --std=c89 --suppressions-list=./tools/cppcheck_suppress.txt --error-exitcode=1 ./code)"
exit $RET
