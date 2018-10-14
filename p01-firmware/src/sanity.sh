#!/bin/bash

time_limit="${3}"

if /usr/bin/curl -sSf --max-time "${time_limit}" "${1}:${2}" --insecure 2>/dev/null >/dev/null; then
    true
else
    /sbin/runuser -l ctf -c 'cd /app; ./server' &
    false
fi
