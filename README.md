##DOING
```cmake success```


depends on:
1. libevent
2. log4cplus


TODO list
1. /home/pi/study/github/smartGateway/src/server/RF24_server/RF24_server.cpp:103:13: error: within this context
     network.frame_queue.push(frame);
     add lock for frame_queue
     
###pin assignment

PIN 	NRF24L01 	 	RPI 	
1 	GND 	GND 
2 	VCC 	3.3V 	
3 	CE 	rpi-gpio22	
4 	CSN 	rpi-gpio8 
5 	SCK 	rpi-sckl 
6 	MOSI rpi-mosi 	
7 	MISO rpi-miso 	
8 	IRQ 	
