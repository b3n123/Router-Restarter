#define TIME_1 20000
#include <UIPEthernet.h>
#include <TelnetClient.h>
#include <ArduinoOTA.h>
#include <avr/wdt.h>
//declaration
EthernetClient client;
telnetClient tc(client); 
unsigned long next;
unsigned long next2;

IPAddress mikrotikRouterIp (192, 168, 100, 1);
int ledPin=13;
int RS=12;

void setup()
{
	/*OTA setting*/
	wdt_enable(WDTO_8S);
	const char* ota_hostname = "Restarter01";
	Serial.begin(115200);
	pinMode(ledPin,OUTPUT);
	Serial.print("setup ");
	uint8_t mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
	uint8_t dhcp=0;
	while(dhcp==0)
	{
		if(Ethernet.linkStatus()!=1)
		{
			
			eth_reset();
		}
		
		if(!Ethernet.begin(mac/*,IPAddress(192,168,100,246)*/)) //Configure IP address via DHCP
		{
			Serial.print("No IP obtained");
		}
		else
		{
			dhcp=1;
		}
	}
	
	Serial.print("Local IP: ");
	Serial.println(Ethernet.localIP());
	Serial.print("Subnet Mask: ");
	Serial.println(Ethernet.subnetMask());
	Serial.print("Gateway IP: ");
	Serial.println(Ethernet.gatewayIP());
	Serial.print("dnsServerIP: ");
	Serial.println(Ethernet.dnsServerIP());
	
	next=0;
	next2=0;
	/*******************************************/
	/*Pub IP*/
	/*******************************************/
	//GetExternalIP();
	 digitalWrite(ledPin,LOW);
}

void loop(){
	 // check for updates
	if((signed long)(millis()-next2)>0)
	{
		wdt_reset();//watchdog heartbeat
		next2=millis() + 3000;
		//Serial.println(Ethernet.linkStatus());
		if(Ethernet.linkStatus()!=1)
		{
			
			eth_reset();
		}
		
	}
	if((signed long)(millis()-next)>0)
	{
		Ethernet.maintain();
		next=millis() + TIME_1;
		digitalWrite(ledPin,HIGH);
		resetRouter();
	
	}
}
void eth_reset()
{
	
	
		pinMode(RS,OUTPUT);
		digitalWrite(RS,LOW);
		delay(100);
		digitalWrite(RS,HIGH);
		pinMode(RS,INPUT);
		
	
}
char* GetExternalIP()
{
	char *r;
  EthernetClient client;
  int timeout = millis() + 5000;
  while (Ethernet.linkStatus() != 1) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return 0;
      }
	}
 
  if (!client.connect("api.ipify.org", 80)) {
    Serial.println("Failed to connect with 'api.ipify.org' !");
  }
  else {
	  
    timeout = millis() + 5000;
	while (Ethernet.linkStatus() != 1) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return 0;
      }
	}
    client.println("GET / HTTP/1.1");
	client.println("Host: api.ipify.org");
	client.println("Connection: close");
	client.println();
   wdt_reset();
	while (client.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return 0;
      }
    }
    int size;
	uint8_t offset=167;
	
	
    while ((size = client.available()) > 0) {
      uint8_t* msg = (uint8_t*)malloc(size);
      size = client.read(msg, size);
	  size=size-offset;
	  
	  if(size<20)
	  {
		  if(size<10)return 0;
		  r= malloc(sizeof (char) * (size+1));
		  for(int i=0;i<size;i++)
		{
		  r[i]=*(msg+offset+i);
		}
		r[size]='\0';
	  }
      free(msg);
	  //Serial.println("");
    }
	
  }
  return r;
}
char* GetRouterIP()
{
	char *r;
	if((tc.login(mikrotikRouterIp, "root", "admin",23))){        //tc.login(mikrotikRouterIp, "admin", "", 1234) if you want to specify a port different than 23
			r=tc.sendCommandCustom("ifconfig");
			
			 
		}
		  else{
			Serial.println("login failed");
			return 0;
		  }
		  
		  return r;
}
void resetRouter()
{
	char *pub_ip_1;
	char *pub_ip_2;
	bool end=false;
	uint8_t index=0;
	if(pub_ip_1=GetExternalIP())
	{
		if(pub_ip_2=GetRouterIP())
		{
			Serial.println(pub_ip_1);
			Serial.println(pub_ip_2);
			digitalWrite(ledPin,LOW);
			if(strcmp(pub_ip_1,pub_ip_2))
			{
				tc.sendCommand("reset");
			}
		}
		else
		{
			next=next-(TIME_1+5000);
		}
	}
	else
	{
		next=next-(TIME_1+5000);
	}
	
	tc.disconnect();
	free(pub_ip_2);
	free(pub_ip_1);
}
// starts OTA server
