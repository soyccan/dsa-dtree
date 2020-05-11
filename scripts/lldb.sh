#!/bin/bash

# disassemble next line
cmd="target stop-hook add
bt
di --pc
DONE"

while [ -n "$1" ] && [ "$1" != '--' ]; do
    cmd="$cmd"$'\n'"$1"
    shift
done

shift
lldb -s <(echo "$cmd") "$@"
