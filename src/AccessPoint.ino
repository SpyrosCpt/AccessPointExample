#define NUM_NETWORKS 20

String AvailableNetworks[NUM_NETWORKS];
int numNetworks = 0;

void ScanForWifiNetworks(void)
{
  int n = 0;
  int i = 0;
  delay(200);
  n =  WiFi.scanNetworks();
  delay(200);
  if(n == 0) { if(DEBUG) Serial.println("No networks found"); }
  else
  {
    if(DEBUG) Serial.println();
    if(DEBUG) Serial.print("Found ");
    if(DEBUG) Serial.print(n);
    if(DEBUG) Serial.println(" Networks");

    for(i = 0; i < NUM_NETWORKS; i++) { AvailableNetworks[i] = WiFi.SSID(i); }
    if(DEBUG) Serial.println();
  }
  numNetworks = n;
}
/**
  * @brief  This sets up the ESP32 as an access point
  * @param  none
  * @retval gotAccessPointConnection: 1 - got connection, 0 - no connection got
  */
byte DoAccessPointSetup(void)
{
  byte GotDetails = 0;
  byte SSIDLength = 0;
  byte PasswordLength = 0;
  byte n = 0;
  byte CheckSumArr[EEPROM_SIZE];
  byte waitingForClientConnection = 0;
  
  AccessPointSetup();                      //setup access point
 
  GotDetails = ReadEEPROM(0);              //check if there is a SSID and Password (1= yes, 0 = no)

  ScanForWifiNetworks();
    
  if(GotDetails) AccessPointTimer = 30000; //30 seconds if there is a SSID and password
  else AccessPointTimer = 120000;          //120 seconds if there isn't (for 1st time connection)
  
  while(AccessPointTimer != 0)             //wait to see if a device connects
  {
     if(!AccessPointConnection) 
     { 
        if(DEBUG) Serial.print("\rWaiting for a connection.. "); 
        if(DEBUG) Serial.print(AccessPointTimer/1000);  
        if(DEBUG) Serial.print(" "); 
     }
     if(AccessPointConnection == 1) { gotAccessPointConnection = 1; break; } //got a connection
     server.handleClient();
  }
  if(AccessPointConnection)                //wait until the user finishes inputting wifi details
  {
    if(DEBUG) Serial.println("- Got Connection!");
    
    while(AccessPointConnection) server.handleClient(); //get wifi details via AP
    delay(1000);
  }
  else 
  { 
    if(DEBUG) Serial.println("No Connection!");  
    if(DEBUG)  Serial.println(); 
    server.stop();
    server.close();
    WiFi.softAPdisconnect();
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println();
    Serial.println("HTTP server stopped");
    Serial.println();
  }

  return gotAccessPointConnection;
}
void AccessPointSetup(void) 
{
  const char* AccessPointName = "ESP32_Access_Point"; //change to what ever you want the access point name to be;
  char macID[12];
  int i = 0;
  
  WiFi.softAP(AccessPointName);
  delay(1000);  
  WiFi.softAPConfig(local_ip, gateway, subnet);      //these are configured at the top of the main file (172.16.1.1)
  delay(1000);                                        //this delay is important, does not work without it, do not remove!
  
  server.on("/", handle_OnConnect);
  server.on("/action_page.php", handle_submit);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  if(DEBUG) Serial.println("Device has started in AP mode");
}

void handle_OnConnect()  //this gets called when a device connects
{
  server.send(200, "text/html", SendHTML(false)); 
  AccessPointConnection = 1;
}

void handle_submit()    //this gets called when a user presses "submit"
{
  String ssid = "";
  String pwd = "";
  byte n = 0;
  byte CheckSumArr[EEPROM_SIZE];

  ssid = server.arg(0); 
  pwd = server.arg(1);

  if(DEBUG) Serial.println();
  if(DEBUG) Serial.println();
  if(DEBUG) Serial.println("New WIFI details");
  if(DEBUG) Serial.print("SSID: ");
  if(DEBUG) Serial.println(ssid);
  if(DEBUG) Serial.print("PWD : ");
  if(DEBUG) Serial.println(pwd);
  
  server.send(200, "text/html", SendHTML(true)); 
  delay(1000);
  
  WIFI_SSID = ssid;
  WIFI_PASSWORD = pwd;
  
  Serial.println();
  if(DEBUG) Serial.println("Storing new Wifi details in EEPROM.. ");

  for(n = 0; n < EEPROM_SIZE; n++) CheckSumArr[n] = 0;  //clear the array

  CheckSumArr[0] = 1; 
  CheckSumArr[1] = ssid.length(); 

  for(n = 0; n < CheckSumArr[1]; n++) CheckSumArr[n+2] = ssid[n];
  
  CheckSumArr[22] = pwd.length(); 
  
  for(n = 0; n < CheckSumArr[22]; n++) CheckSumArr[n+23] = pwd[n];

  CheckSumArr[43] = Checksum(CheckSumArr, EEPROM_SIZE - 1);
  for(n = 0; n < EEPROM_SIZE; n++) WriteEEPROM(n, CheckSumArr[n]); 
  
  server.stop();
  server.close();
  WiFi.softAPdisconnect();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println();
  Serial.println("HTTP server stopped");
  Serial.println();
  AccessPointConnection=0;
}

void handle_NotFound()                //this gets called when something goes wring and a page isnt found
{
  server.send(404, "text/plain");
}

/**
  * @brief  This function prints out the html code needed for the webpage
  * @param  Submit: 1- user pressed submit, 0- not submitted
  * @retval ptr to HTML string
  */
String SendHTML(uint8_t submit)
{ 
  int i = 0;
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Wifi Setup</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1 style=""background-color:rgb(23,161,165);"">Wifi Setup</h1>\n"; //252,203,8
  
  if(submit==true) ptr+= "<h3>Your device will now connect with the new WIFI details.</h3\n>";
  else
  {  
     ptr +="<h3>Please enter SSID and Password</h3>\n";
     ptr +="<form action=""/action_page.php"">";
     ptr +="<label for=""ssid"">SSID:</label><br>";
     ptr += "<select id=""\"ssid\""" name=""\"ssid\">";
     for(i = 0 ; i < numNetworks; i++) 
     {
        ptr += "<option value=""\"";
        ptr += AvailableNetworks[i]; //"\"HOGWARTS [2Ghz]\"";//AvailableNetworks[i];
        ptr += """\">"; 
        ptr += AvailableNetworks[i];
        ptr += "</option>";
     }
     ptr += "</select><br><br>";
     ptr +="<label for=""pwd"">Password:</label><br>";
     ptr +="<input type=""password"" id=""pwd"" name=""pwd""><br><br>";
     ptr +="<input type=""submit"" value=""Connect"">";
     ptr +="<input type=""reset"">";
     ptr +="</form>"; 
   }
   
   ptr +="</body>\n";
   ptr +="</html>\n";
   
   return ptr;
}
