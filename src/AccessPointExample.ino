#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <WebServer.h>

#define EEPROM_SIZE         44
#define ARRAY_SIZE          256
#define DEBUG               1

WiFiClient wifiClient;

/* Put IP Address details */
IPAddress local_ip(172,16,1,1);
IPAddress gateway(172,16,1,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);

String WIFI_SSID = " ";
String WIFI_PASSWORD= " ";
volatile int WifiTimer = 0;
volatile long AccessPointTimer = 0;
volatile byte waitingForClientConnection = 0;
volatile byte AccessPointConnection = 0;
byte gotAccessPointConnection = 0;
hw_timer_t * timer = NULL;
byte WifiConnectionStatus = 0;
byte wifiCount = 0;

/*************************PROTOTYPES***************************************/
void DoWifiSetup(byte temp);
byte CheckWifi(byte wifiReconnectCount);
byte Checksum(volatile byte *Data, byte Length);
byte CheckEEPROM(void);
void SignalStrength(long str);
byte DoAccessPointSetup(void);
void AccessPointSetup(void);
void handle_OnConnect();
void handle_NotFound();
String SendHTML(uint8_t submit);
void DoTimerStuff(void);
/**********************END OF PROTOTYPES*********************************/

/**
  * @brief  This interrupt fires every 1ms and counts down any active timers
  * @param  none      
  * @retval none
  */
void IRAM_ATTR onTimer() 
{
  if(WifiTimer) WifiTimer--;
  if(AccessPointTimer) AccessPointTimer--;
}

/**
  * @brief  This fuction calculates an 8-bit checksum
  * @param  The array of data to be used and the length 
  * @retval The calculated Checksum
  */
byte Checksum(volatile byte *Data, byte Length)
{
    byte count;
    byte Sum = 0;
    byte Sum1 = 0; 
    
    count = 0;
    do {
        Sum1 = Sum1 + Data[count];
        count++;
    } while (--Length);
    
    Sum = -Sum1;

    return (byte)(Sum & 0xFF);
}

/**
  * @brief  This function writes a byte to the ESP EEPROM
  *         EEPROM MEMORY MAP
  *         ------------------
  *         byte 0  - Valid Wifi details(1), No Valid Wifi details(0)
  *         byte 1  - Length of SSID
  *         byte 2  - 21 - SSID characters 
  *         byte 22 - Length of Password
  *         byte 23 - 42 - Password characters
  *         byte 43 - Checksum
  *         
  * @param  address     - the EEPROM address to write to   
  *         byteToWrite - the byte to write to the EEPROM
  * @retval none
  */
byte WriteEEPROM(byte address, byte byteToWrite)
{
  EEPROM.write(address, byteToWrite);
  EEPROM.commit(); 
}

/**
  * @brief  This function reads a byte from the ESP EEPROM
  * @param  address - the address to read from in EEPROM      
  * @retval byte    - the byte returned from the address
  */
byte ReadEEPROM(byte address)
{
  return EEPROM.read(address);
}

/**
  * @brief  This function adds all bytes in EEPROM + checksum and returns if the EEPROM is okay or corrupt
  * @param  none
  * @retval 0 = EEPROM corrupt, 1 = EEPROM okay
  */
byte CheckEEPROM(void)
{
  byte n = 0;
  byte sum = 0;
  
  for(n = 0; n < EEPROM_SIZE; n++) sum += ReadEEPROM(n);  
  
  if(sum == 0) return 1;

  return 0;
}

void DoTimerStuff(byte wifiReconnectCount)
{
  byte wifiStatus = 0;
  
  if(!WifiTimer) 
  {
    wifiCount = CheckWifi(wifiCount);
    wifiStatus = CheckWifi(wifiReconnectCount);   
    if(wifiStatus == 0) WifiTimer = 30;
    else WifiTimer = 5; 
  }
}
void setup() 
{  
  byte temp = 0;
  
  //SETUP TIMER
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000, true);      //1ms interrupt
  timerAlarmEnable(timer);
  
  Serial.begin(115200);                    //setup serial
  Serial1.begin(115200);                   //setup second serial
  
  EEPROM.begin(EEPROM_SIZE);               //initailize ESP eeprom

  delay(1000);
  temp = DoAccessPointSetup();             //setup access point 
  
  DoWifiSetup(temp);                       //Do wifi setup
}

void loop() 
{  
  static byte wifiReconnectCount = 0;

  DoTimerStuff(wifiReconnectCount);
}
