#!/bin/bash

rm -r /var/www/html/
cp -r /app/ /var/www/html/

chown -R root:root /var/www/html/
chmod -R 555 /var/www/html/

chmod -R 777 /var/www/html/storage
chmod -R 777 /var/www/html/bootstrap/cache

a2dissite 000-default.conf
a2ensite assignment.conf
a2enmod rewrite
service apache2 start

(crontab -l ; echo "* * * * * /bin/bash -c '/sanity.sh 127.0.0.1 80 10'") | crontab
service cron start

tail -f /var/log/apache2/access.log
