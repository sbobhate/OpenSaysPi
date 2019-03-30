#!/bin/bash

PIDFILE="/home/pi/Desktop/EC535Project/QR/QR.pid"

if [ -e "${PIDFILE}" ] && (ps -u $(whoami) -opid= |
                           grep -P "^\s*$(cat ${PIDFILE})$" &> /dev/null); then
  echo "Already running."
  exit 99
fi

nohup /home/pi/Desktop/EC535Project/QR/QR >> /home/pi/Desktop/EC535Project/QR/QR_log.txt &

echo $! > "${PIDFILE}"
chmod 644 "${PIDFILE}"