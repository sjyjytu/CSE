#!/bin/sh
java -jar ./jywatcher.jar 172.20.0.3,172.20.0.4,172.20.0.5 /jy ${NAME} /etc/hosts &
/usr/local/bin/npm start