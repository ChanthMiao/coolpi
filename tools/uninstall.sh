#!/bin/bash

if [ $UID -ne 0 ];
then
    echo "Permission denied, please retry it as root."
fi

echo "stopping the coolpi..."
systemctl stop coolpi
echo "Stopped"
echo "Removing files..."
rm -rf /usr/local/coolpi
rm -rf /etc/coolpi
rm -f /lib/systemd/system/coolpi.service
systemctl daemon-reload
echo "Done"
echo "coolpi is already removed from your raspberry pi 3b+"