#!/bin/bash

cd /app
make

chown -R root:root /app
chmod -R 555 /app

runuser -l ctf -c 'cd /app; ./server' &

(crontab -l ; echo "* * * * * /bin/bash -c '/sanity.sh 127.0.0.1 8000 10'") | crontab
service cron start

tail -f /dev/null
