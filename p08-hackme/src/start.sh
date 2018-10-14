#!/bin/bash

rm -r /usr/share/modsecurity-crs/	
cp /app/waf.conf /etc/modsecurity/waf.conf

chown -R root:root /webapp
chmod -R 555 /webapp

runuser -l ctf -c 'cd /webapp; /usr/local/bin/gunicorn -w 4 --bind 0.0.0.0:8000 hackme:app' &

a2dissite 000-default.conf
a2ensite hackme.conf
a2enmod proxy
a2enmod proxy_http
service apache2 start

(crontab -l ; echo "* * * * * /bin/bash -c '/sanity.sh 127.0.0.1 80 10'") | crontab
service cron start

tail -f /dev/null
