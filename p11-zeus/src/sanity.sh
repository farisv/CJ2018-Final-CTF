#!/bin/bash

time_limit="${3}"

if timeout $time_limit nc -z ${1} ${2} 2>/dev/null >/dev/null; then
    true
else
    /usr/bin/socat TCP4-LISTEN:8000,reuseaddr,fork,su=ctf EXEC:"/app/zeus_sandbox /app/zeus" > /dev/null 2>&1 &
    return
fi
