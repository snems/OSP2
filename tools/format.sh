#!/bin/bash

declare -a FORMAT_PATHS=( \
"./code/cgame/*.c" \
"./code/cgame/*.h" \
"./code/game/*.c" \
"./code/game/*.h" \
"./code/ui/*.c" \
"./code/ui/*.h" \
"./code/q3_ui/*.c" \
"./code/q3_ui/*.h" \
)
DRY_RUN=0

ASTYLE="astyle --formatted --errors-to-stdout --options=./tools/astyle.conf"

if [[ $1 = "--dry" ]]; then
  ASTYLE=$ASTYLE" --dry-run"
  DRY_RUN=1
fi

NUMBER_ERRORS=0

astyle_job(){
  WILDCARD=$1
  for SOME_FILE in $WILDCARD; do

    # skip if no files
    if [[ "$SOME_FILE" = "$WILDCARD" ]]; then
      break
    fi

    # is formatted ?
    RESULT=`$ASTYLE $SOME_FILE | grep -i formatted | wc -l`

    # store and print error
    if [ "$RESULT" -ne 0 ]; then
      if [ "$DRY_RUN" -ne 0 ]; then
        echo File $SOME_FILE should be astyle formatted!
      fi
      NUMBER_ERRORS=$((NUMBER_ERRORS + 1))
    fi

  done
}


for CHECK_PATH in "${FORMAT_PATHS[@]}"
do
  astyle_job "$CHECK_PATH"
done


if [ "$NUMBER_ERRORS" -ne 0 ] ; then
  if [ "$DRY_RUN" -ne 0 ]; then
    echo Run ./format.sh from root directory to fix it
  else
    echo Number of formatted files: $NUMBER_ERRORS
  fi
fi

exit $NUMBER_ERRORS

