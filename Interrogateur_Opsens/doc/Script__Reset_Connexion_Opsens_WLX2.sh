echo "Arrêt du serveur dhcp (dncmasq)"
#sudo service isc-dhcp-server stop
sudo service dnsmasq stop


#echo "Arrêt de eth0"
#sudo ifdown eth0  

#sudo ifdown eth0
#echo "Arrêt de eth0"
#sleep 5


#echo "Redémarrage d'eth0"
#sudo ifdown eth0 && sudo ifup eth0 && sudo service isc-dhcp-server stop

#echo "Arrêt du serveur dhcp"
#sudo service isc-dhcp-server stop


#process_dhcp="dhclient"
process_dhcp="dnsmasq"
list1=$(pidof $process_dhcp)
#echo $list1

for p in $list1
do
  	
  echo "Killing $p..."
  sudo kill -9 $p
done


#process_dhcp="/usr/sbin/dhcpd"
process_dhcp="/usr/sbin/dnsmasq"
list2=$(pidof -x $process_dhcp)
#echo $list2 

for p in $list2
do	
  echo "Killing $p..."
  sudo kill -9 $p
done


#echo "Modification de la configuration eth0 (mode Internet IP: 137.121.77.72)"
sudo cp /etc/network/interfaces.back /etc/network/interfaces


echo "Démarrage du serveur dhcp (dhcpcd)"
sudo service dhcpcd start

#echo "Redémarrage d'eth0"
#sudo ifup eth0




