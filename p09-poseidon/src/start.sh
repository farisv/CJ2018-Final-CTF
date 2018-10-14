#!/bin/bash

cd /app

chown -R root:root /app
chmod -R 555 /app

socat TCP4-LISTEN:8000,reuseaddr,fork,su=ctf EXEC:"./poseidon" > /dev/null 2>&1 &

(crontab -l ; echo "* * * * * /bin/bash -c '/sanity.sh 127.0.0.1 8000 5'") | crontab
service cron start

tail -f /dev/null
