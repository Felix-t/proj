echo "Arrêt du serveur dhcp"
sudo service isc-dhcp-server stop

#sudo ifdown eth0
#echo "Arrêt de eth0"
#sleep 5

echo "Modification de la configuration eth0 (mode local IP: 10.0.0.1)"
sudo cp /etc/network/interfaces.opsens /etc/network/interfaces

echo "Redémarrage d'eth0"
sudo ifdown eth0 && sudo ifup eth0

echo "Redémarrage du serveur dhcp"
sudo cp /var/lib/dhcp/dhcpd.leases.opsens /var/lib/dhcp/dhcpd.leases

sudo service isc-dhcp-server start


