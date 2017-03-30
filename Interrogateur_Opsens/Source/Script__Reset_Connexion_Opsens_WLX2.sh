echo "Arrêt du serveur dhcp"
sudo service isc-dhcp-server stop
#sudo ifdown eth0  

#sudo ifdown eth0
#echo "Arrêt de eth0"
#sleep 5

echo "Modification de la configuration eth0 (mode Internet IP: 137.121.77.72)"
sudo cp /etc/network/interfaces.back /etc/network/interfaces

echo "Redémarrage d'eth0"
#sudo ifdown eth0 && sudo ifup eth0 && sudo service isc-dhcp-server stop

echo "Arrêt du serveur dhcp"
#sudo service isc-dhcp-server stop

process_dhcp="dhclient"
list1=$(pidof $process_dhcp)
echo $list1

for p in $list1
do
  	
  echo "Killing $p..."
  sudo kill -9 $p
done


process_dhcp="/usr/sbin/dhcpd"
list2=$(pidof -x $process_dhcp)
echo $list2 

for p in $list2
do	
  echo "Killing $p..."
  sudo kill -9 $p
done

