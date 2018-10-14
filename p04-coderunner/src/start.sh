#!/bin/bash

rm -r /usr/share/modsecurity-crs/
cp /app/waf.conf /etc/modsecurity/waf.conf

service apache2 start

(crontab -l ; echo "* * * * * /bin/bash -c '/sanity.sh 127.0.0.1 80 10'") | crontab
service cron start

tail -f /var/log/apache2/access.log
