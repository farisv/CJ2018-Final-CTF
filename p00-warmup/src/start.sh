#!/bin/bash

rm /var/www/html/index.html
cp app/* /var/www/html/
chown -R root:root /var/www/html/
chmod -R 555 /var/www/html/

service apache2 start

(crontab -l ; echo "* * * * * /bin/bash -c '/sanity.sh 127.0.0.1 80 10'") | crontab
service cron start

tail -f /var/log/apache2/access.log
