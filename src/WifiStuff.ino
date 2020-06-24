
/*WIFI STATUS RETURNS*/
/*  
 *   WL_CONNECTED:       3                                                                  
     WL_NO_SHIELD:       255                                                                
     WL_IDLE_STATUS:     0                                                                
     WL_NO_SSID_AVAIL:   1                                                              
     WL_SCAN_COMPLETED:  2                                                             
     WL_CONNECT_FAILED:  4                                                             
     WL_CONNECTION_LOST: 5                                                            
     WL_DISCONNECTED:    6 
 */
 
/**
  * @brief  This function sets up the wifi using the details provided via bluetooth or the details stored in EEPROM
  * @param  temp - whether details have been received via bluetooth (0 - not received, 1 - received via bluetooth)
  * @retval none
  */
void DoWifiSetup(byte temp)
{
  byte GotWifi = 0;
  byte len1=0;
  byte SSIDLength = 0;
  byte PasswordLength = 0;
  byte n = 0;
  byte ReconnectAttempts = 5; //30 times (5 * 6 = 30)
  byte EEPROM_Status = 0;
  
  if(!temp) 
  { 
    GotWifi = ReadEEPROM(0);      //check if there is a SSID and Password
    EEPROM_Status = CheckEEPROM(); //Ensure that EEPROM is not corrupt
    
    if((GotWifi == 1) && (EEPROM_Status == 1))              //found details inside EEPROM and they are not corrupt
    {
      WIFI_SSID = "";
      WIFI_PASSWORD = "";
      if(DEBUG) Serial.println("WIFI Details Found!");
     
      SSIDLength     = ReadEEPROM(1);   //get ssid length
      PasswordLength = ReadEEPROM(22);  //get password length

      for(n = 2; n < SSIDLength+2; n++)       WIFI_SSID += (char)ReadEEPROM(n);          //Get the SSID
      for(n = 23; n < PasswordLength+23; n++) WIFI_PASSWORD += (char)ReadEEPROM(n);      //Get the passowrd
  
      if(DEBUG) Serial.print("SSID: ");
      if(DEBUG) Serial.println(WIFI_SSID);
      if(DEBUG) Serial.print("Password: ");
      if(DEBUG) Serial.println(WIFI_PASSWORD); 
      if(DEBUG) Serial.println();          
    }
    else                      //No valid details found
    {
      if(!GotWifi) if(DEBUG) { if(DEBUG) Serial.println("No WIFI details found!"); }
      if(!EEPROM_Status)  { if(DEBUG) Serial.print("EEPROM Corrupt!"); } //Serial.println((char)ReadEEPROM(0));
    }
  }
  
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());  //start the wifi and try to connect using the details 
  
  if(DEBUG) Serial.print("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED)                 //wait for a connection to be established
  {
    if(DEBUG) Serial.print(".");
    wifiCount++;
    if(wifiCount > 5) { WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str()); wifiCount = 0; ReconnectAttempts--; } 

    if(ReconnectAttempts == 0) //try 30 times, if we fail we need a restart
    {
      if(DEBUG) Serial.println();
      if(DEBUG) Serial.println();
      if(DEBUG) Serial.println("Wifi Connection Permanent Fail"); 
      if(DEBUG) Serial.println("Please check the following: ");
      if(DEBUG) Serial.println("1. Wifi is on");
      if(DEBUG) Serial.print("2. SSID is correct: ");
      if(DEBUG) Serial.println(WIFI_SSID);
      if(DEBUG) Serial.print("3. Password is correct: ");  
      if(DEBUG) Serial.println(WIFI_PASSWORD);   
      while(1) {  }
    }
    delay(1000);
  }
  if(DEBUG) Serial.println();
  if(DEBUG) Serial.println();
 
  if(DEBUG) Serial.print("Connected with IP: ");
  if(DEBUG) Serial.println(WiFi.localIP());
  if(DEBUG) Serial.print("RSSI Strength: ");
  if(DEBUG) Serial.print(WiFi.RSSI());
  SignalStrength(WiFi.RSSI());
  WifiConnectionStatus = 1;
}

/**
  * @brief  This function checks the current status of the wifi
  * @param  wifiReconnectCount - how many times we have tried to reconnect
  * @retval wifi status (1 - all good, 0- not connected)
  */
byte CheckWifi(byte wifiReconnectCount)
{
  if ((WiFi.status() != WL_CONNECTED) || (WifiConnectionStatus == 0))                            //check the wifi is connected
  {
    if(WifiConnectionStatus == 1) WifiConnectionStatus = 0;
    if(DEBUG) Serial.println("\nWi-Fi Connection Lost");  
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());       //connection lost try to reconnect
    if(DEBUG) Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
      if(DEBUG) Serial.print(".");
      wifiCount++;
      if(wifiCount > 5) { WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str()); wifiCount = 0; }
      wifiReconnectCount++;

      if(wifiReconnectCount > 30)                               //After 30 attempts disable trying to connect for 30 seconds
      {
        if(DEBUG) Serial.println();
        if(DEBUG) Serial.println("Disable Wifi connect for 30s");
        wifiReconnectCount = 0; 
        WifiConnectionStatus = 0;
        return 0;
      }
      delay(1000);
    }
     WifiConnectionStatus = 1;
     if(DEBUG) Serial.println("Connected to Wi-Fi!");
  }  
  return 1;
}

void SignalStrength(long str)
{ 
  if(str > -50)                           { if(DEBUG) Serial.println(" - EXCELLENT!"); }
  else if( (str <= -50) && (str > -60) )  { if(DEBUG) Serial.println(" - GOOD!"); }
  else if( (str <= -60) && (str > -70) )  { if(DEBUG) Serial.println(" - FAIR!"); }
  else if( str <= -70)                    { if(DEBUG) Serial.println(" - WEAK!"); }
  else { if(DEBUG) Serial.println(" - SIGNAL ERR!"); }
  if(DEBUG) Serial.println();
}
