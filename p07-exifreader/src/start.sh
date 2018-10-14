#!/bin/bash

chown -R root:root /app
chmod -R 555 /app

mkdir /app/temp
chmod -R 777 /app/temp

runuser -l ctf -c 'cd /app; /usr/local/bin/gunicorn -k tornado -w 4 --bind 0.0.0.0:8000 server:application' &

(crontab -l ; echo "* * * * * /bin/bash -c '/sanity.sh 127.0.0.1 8000 10'") | crontab
service cron start

tail -f /dev/null
