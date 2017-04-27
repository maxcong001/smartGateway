#include "RF24Mesh.h"  
#include <RF24.h>
#include <RF24Network.h>


RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);  
RF24Network network(radio);
RF24Mesh mesh(radio,network);

uint32_t displayTimer=0;

int main(int argc, char** argv) {
  
  // Set the nodeID to 0 for the master node
  mesh.setNodeID(4);
  // Connect to the mesh
  printf("start nodeID %d\n",mesh.getNodeID());
  mesh.begin();
  radio.printDetails();

while(1)
{
  
  // Call mesh.update to keep the network updated
  mesh.update();
    
  // Send the current millis() to the master node every second
  if(millis() - displayTimer >= 1000){
    displayTimer = millis();
    
    if(!mesh.write(&displayTimer,'M',sizeof(displayTimer))){
       
      // If a write fails, check connectivity to the mesh network
      if( ! mesh.checkConnection() ){
        // The address could be refreshed per a specified timeframe or only when sequential writes fail, etc.
		printf("Renewing Address\n");
        mesh.renewAddress(); 
	  }else{
        printf("Send fail, Test OK\n"); 
      }
    }else{
	  printf("Send OK: %u\n",displayTimer);
	}
  }
  delay(1);
  }
return 0;
}