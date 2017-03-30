echo "Arrêt du serveur dhcp (dhcpcd)"
#sudo service isc-dhcp-server stop

sudo service dhcpcd stop

#sudo service dnsmasq stop

#sudo ifdown eth0
#echo "Arrêt de eth0"
#sleep 5

echo "Modification de la configuration eth0 (mode local IP: 10.0.0.1)"
sudo cp /etc/network/interfaces.opsens /etc/network/interfaces

echo "Redémarrage d'eth0"
sudo sudo ifdown eth0 && sudo ifup eth0

echo "Démarrage du serveur dhcp (dnsmasq)"
#sudo cp /var/lib/dhcp/dhcpd.leases.opsens /var/lib/dhcp/dhcpd.leases
sudo cp /var/lib/misc/dnsmasq.leases.opsens /var/lib/misc/dnsmasq.leases

#sudo service isc-dhcp-server start
sudo service dnsmasq start

