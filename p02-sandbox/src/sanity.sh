#!/bin/bash

time_limit="${3}"

if /usr/bin/curl -sSf --max-time "${time_limit}" "${1}:${2}" --insecure 2>/dev/null >/dev/null; then
    true
else
    /usr/sbin/service apache2 restart
    false
fi
