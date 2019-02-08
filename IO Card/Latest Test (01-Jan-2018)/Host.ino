/*
    This sketch shows the Ethernet event usage

*/

#include <ETH.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <string.h>
#include <sstream>
#include "Adafruit_MCP23017.h"

IPAddress local_IP(192, 168, 1, 107); 
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
uint8_t datalen;
uint8_t incomingPacket[255];  // buffer for incoming packets
uint8_t replyPacket_config[] = "Configuration Recieved";
uint8_t replyPacket_inputfn[128];
std::string InputString[8];
std::stringstream InputStingStream[8]; 

Adafruit_MCP23017 mcp[8];
uint8_t mcp_pinconfig[8][16];
uint8_t pin_config=0;
int check;
uint8_t inputData[8][16];
uint8_t inputSingle=0;
static bool eth_connected = false;
void check_data(uint8_t incomingPacket[], uint8_t datalen);
void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      ETH.config(local_IP, gateway, subnet);
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void setup()
{
  for(int i=0; i<8; i++)
  {
    mcp[i].begin(i);
  }
  for(int i=0; i<8; i++)
  {
    for(int j=0; j<16; j++)
    {
      inputData[i][j]=0xAA;
    }
  }
  Serial.begin(115200);
  
//    if (!WiFi.config(local_IP, gateway, subnet)) {
//    Serial.println("STA Failed to configure");
//  }
  
  WiFi.onEvent(WiFiEvent);
  
//  if (!ETH.config(local_IP, gateway, subnet)) {
//    Serial.println("STA Failed to configure");
//  }
  
  ETH.begin();
  while(eth_connected = false)
  {}
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", ETH.localIP().toString().c_str(), localUdpPort);
}

unsigned long counter =0;

void loop()
{

int packetSize = Udp.parsePacket();

  if (packetSize)
  {
    Serial.println(packetSize);
    Serial.println(counter);
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    datalen = Udp.read(incomingPacket, 255);
    if (datalen > 0)
    {
      incomingPacket[datalen] = 0;
    }
    Serial.print("UDP packet contents:");
    for(int i=0; i<datalen; i++)
    {
      Serial.print(incomingPacket[i]);
      Serial.print(", ");
    }
    Serial.print("\n");
    check_data(incomingPacket, datalen);
    // send back a reply, to the IP address and port we got the packet from
    counter++;
    //Serial.print( "\nfree Heap : "); Serial.println( esp_get_free_heap_size());
  }
}

void check_data(uint8_t (incomingPacket)[], uint8_t datalen)
{
  if(incomingPacket[0]==0x7D && incomingPacket[1]==0xD7 && incomingPacket[datalen-2]==0xFF && incomingPacket[datalen-1]==0xFA)
  // Check headers and footers
  {
    switch(incomingPacket[2])
    {
      case 1: //Configure Function
        if(incomingPacket[3] == 0x01 && datalen==22)
        {
          check=1;
          int count=4;
          //MCP IC and Pin configuration. (i = IC number, j = pin data)
          for (int i=0; i<8; i++)
          {
            Serial.print(i);
            Serial.print(" pin:");
            for (int j=0; j<16; j++)
            {
              mcp_pinconfig[i][j]=((((uint16_t)incomingPacket[count+1]<<8) | incomingPacket[count])>>j)&0x01;
              // Above statement converts two bytes into 16-bit number, and stores value of each bit to mcp_pinconfig
              Serial.print(mcp_pinconfig[i][j]);
              if(mcp_pinconfig[i][j]==1)
              {
                mcp[i].pinMode(j,1);
                mcp[i].pullUp(j,1); 
              }
              else
              {
                mcp[i].pinMode(j,0);
              }
            }
            count = count+2; 
            Serial.print("\n");
          }
          pin_config=1;
          datalen=0;          
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          Udp.write(replyPacket_config, sizeof(replyPacket_config)-1);
          Udp.endPacket();
          break;
        }
        else
        {
          check=-1;
          for(int i=0; i<datalen; i++)
          {
            incomingPacket[i]=0;
            pin_config=0;
          }
        break;
        }
   
      case 2: //Input Function
      {
        if(pin_config==1 && incomingPacket[3]==0x02 && incomingPacket[4]==0x90 && datalen==7) // Read all input ports
        {
          for(int i=0; i<8; i++)
          {
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            uint8_t message1[]="MCP"; Udp.write(message1, sizeof(message1)-1);
            uint8_t message2=i; Udp.write(message2);
            uint8_t message3[]=" data:"; Udp.write(message3, sizeof(message3)-1);
            for(int j=0; j<16; j++)
            {
              if(mcp_pinconfig[i][j]==1)
                inputData[i][j]=mcp[i].digitalRead(j);
              Serial.print(inputData[i][j]);
              Udp.write(inputData[i][j]);
            }
            Serial.print("\n");
            Udp.endPacket();
          }
          
          check=1;
          datalen=0;
          break;
        }
        else if(pin_config==1 && incomingPacket[3]==0x03 && datalen==7) // Read inputPacket[4] input port
        {
          uint8_t pin = incomingPacket[4]%16;
          uint8_t ic = incomingPacket[4]/16; 
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          uint8_t message1[]="Port"; Udp.write(message1, sizeof(message1)-1);
          uint8_t message2=incomingPacket[4]; Udp.write(message2);
          uint8_t message3[]=" data:"; Udp.write(message3, sizeof(message3)-1);
          if(mcp_pinconfig[ic][pin]==1 && incomingPacket[4]>=0 && incomingPacket[4]<128)
            inputSingle=mcp[ic].digitalRead(pin);
          Udp.write(inputSingle);
          Udp.endPacket();
          check=2;
          datalen=0;
          break;
        }
        else
        {
          check=-1;
          for(int i=0; i<datalen; i++)
          {
            incomingPacket[i]=0;
            break;
          }
        }
      }
      case 3: //Output Function
      {
        if(pin_config==1 && incomingPacket[3]==0x04 && incomingPacket[4]==0x90 && datalen==7) // Write all output ports LOW
        {
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          //uint8_t message1[]="Writing all ports LOW"; Udp.write(message1, sizeof(message1)-1);
          for(int i=0; i<8; i++)
          {
            for(int j=0; j<16; j++)
            {
              if(mcp_pinconfig[i][j]==0)
              mcp[i].digitalWrite(j, LOW);
            }
          }
        }
        else if(pin_config==1 && incomingPacket[3]==0x04 && incomingPacket[4]==0x91 && datalen==7) // Write all output ports HIGH
        {
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          //uint8_t message1[]="Writing all ports HIGH"; Udp.write(message1, sizeof(message1)-1);
          for(int i=0; i<8; i++)
          {
            for(int j=0; j<16; j++)
            {
              if(mcp_pinconfig[i][j]==0)
              mcp[i].digitalWrite(j, HIGH);
            }
          }
        }
        else if(pin_config==1 && incomingPacket[3]==0x04 && (incomingPacket[5]==0 || incomingPacket[5]==1) && datalen==8) 
        // Write incomingPacket[4] output port to value incomingPacket[5]
        {
          uint8_t pin = incomingPacket[4]%16;
          uint8_t ic = incomingPacket[4]/16; 
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          uint8_t message1[]="Writing mcp "; Udp.write(message1, sizeof(message1)-1);
          uint8_t message2=ic; Udp.write(message2);
          uint8_t message3[]=" Port "; Udp.write(message3, sizeof(message3)-1);
          uint8_t message4=pin; Udp.write(message4);
          if(incomingPacket[5]==0)
          {
            uint8_t message5[]=" LOW"; Udp.write(message5, sizeof(message5)-1);
            mcp[ic].digitalWrite(pin, LOW);
          }
          else 
          {
            uint8_t message5[]=" HIGH"; Udp.write(message5, sizeof(message5)-1);
            mcp[ic].digitalWrite(pin, HIGH);
          }
        }
        else
        {
          check=-2;
          for(int i=0; i<datalen; i++)
          {
            incomingPacket[i]=0;
            break;
          }
        }     
        check=3;
        datalen=0;
        break;
      }     
      default: break;
    }
  }
  else 
  {
    for(int i=0; i<datalen; i++)
    {
      incomingPacket[i]=0;
    }
  }
}
