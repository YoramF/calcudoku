calcu: calcu.o ../timer/libtimer.a
	gcc -s -o calcu.exe calcu.o -L../timer -ltimer

calcu.o: calcu.c calcu.h ../timer/timer.h
	gcc -c -O3 calcu.c

calcu_d: calcu_d.o ../timer/libtimer_d.a
	gcc -o calcu_debug.exe calcu_d.o -L../timer -ltimer_d

calcu_d.o: calcu.c calcu.h ../timer/timer.h
	gcc -c -g -o calcu_d.o calcu.c

clean:
	rm *.o