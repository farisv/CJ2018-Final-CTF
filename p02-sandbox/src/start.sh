#!/bin/bash

cd /app
gcc sandbox.c -o sandbox

rm -r /var/www/html/
cp -r /html /var/www/html
chown -R root:root /var/www/html/

chmod -R 777 /var/www/html/
chmod 555 /var/www/html/index.php

chown -R root:root /app
chmod -R 555 /app

service apache2 start

(crontab -l ; echo "* * * * * /bin/bash -c '/sanity.sh 127.0.0.1 80 10'") | crontab
service cron start

tail -f /var/log/apache2/access.log
