while true; do
	read -p "Is the accelerometer still ?" yn
	case $yn in

		[YyoO]* ) 
			gcc -c -o obj/accelerometer.o src/accelerometer.c -Iinc  -pthread
			gcc -c -o obj/cfg.o src/cfg.c -Iinc -pthread
			gcc -c -o tests/obj/test.o tests/acc_tests.c -Iinc -lm
			gcc -o test tests/obj/test.o obj/accelerometer.o obj/cfg.o -pthread -lm -lbcm2835 -lconfig
			sudo ./test; break;;
		[Nn]* ) exit;;
		* ) echo "Please answer yes(y/o) or no(n).";;
	esac
done
