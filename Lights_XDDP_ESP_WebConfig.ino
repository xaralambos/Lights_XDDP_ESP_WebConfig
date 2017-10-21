/* 
  ESP_WebConfig 

  Copyright (c) 2015 John Lassen. All rights reserved.
  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Latest version: 1.1.3  - 2015-07-20
  Changed the loading of the Javascript and CCS Files, so that they will successively loaded and that only one request goes to the ESP.

  -----------------------------------------------------------------------------------------------
  History

  Version: 1.1.2  - 2015-07-17
  Added URLDECODE for some input-fields (SSID, PASSWORD...)

  Version  1.1.1 - 2015-07-12
  First initial version to the public



  
  */


#define ACCESS_POINT_NAME  "xambisLights"        
#define ACCESS_POINT_PASSWORD  "12345678" 
const char* www_username = "admin"; //web login username
const char* www_password = "secpass"; //web login password
#define AdminTimeOut 3000  // Defines the Time in Seconds, when the Admin-Mode will be diabled
#define LightInPin 13
#define LightOutPin 12

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include "helpers.h"
#include "global.h"
/*
Include the HTML, STYLE and Script "Pages"
*/
#include <XDDP.h>
#include "Page_Root.h"
#include "Page_Admin.h"
#include "Page_Script.js.h"
#include "Page_Style.css.h"
#include "Page_NTPsettings.h"
#include "Page_Information.h"
#include "Page_General.h"
#include "PAGE_NetworkConfiguration.h"
#include "example.h"
#include "myOTA.h"


IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);



void setup ( void ) {

  Serial.begin(115200);
  
  pinMode(LightInPin, OUTPUT);
  pinMode(LightOutPin, OUTPUT);
  digitalWrite(LightInPin, HIGH);
  digitalWrite(LightOutPin, HIGH);

  pinMode(buttonIn, INPUT_PULLUP);
  pinMode(buttonOut, INPUT_PULLUP);
  attachInterrupt(buttonIn, interuptButtonIn, CHANGE);
  attachInterrupt(buttonOut, interuptButtonOut, CHANGE);
  
	EEPROM.begin(512);
	
	delay(500);
	Serial.println("\nStarting ES8266");
  
	if (!ReadConfig())
	{
		// DEFAULT CONFIG
		config.ssid = "xambis";
		config.password = "xambisss";
		config.dhcp = false;
		config.IP[0] = 192;config.IP[1] = 168;config.IP[2] = 0;config.IP[3] = 201;
		config.Netmask[0] = 255;config.Netmask[1] = 255;config.Netmask[2] = 255;config.Netmask[3] = 0;
		config.Gateway[0] = 192;config.Gateway[1] = 168;config.Gateway[2] = 0;config.Gateway[3] = 1;
		config.ntpServerName = "0.de.pool.ntp.org";
		config.Update_Time_Via_NTP_Every =  0;
		config.timezone = -10;
		config.daylight = true;
		config.DeviceName = "Not Named";
		config.AutoTurnOff = false;
		config.AutoTurnOn = false;
		config.TurnOffHour = 0;
		config.TurnOffMinute = 0;
		config.TurnOnHour = 0;
		config.TurnOnMinute = 0;
		WriteConfig();
		Serial.println("General config applied");
	}

	
//	if (AdminEnabled)
//	{
		WiFi.mode(WIFI_AP_STA);
		
    Serial.print("Setting soft-AP configuration ... ");
    Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

    WiFi.softAP( ACCESS_POINT_NAME , ACCESS_POINT_PASSWORD, 4);

    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());

    
//	}
//	else
//	{
//		WiFi.mode(WIFI_STA);
//	}

	ConfigureWifi();

  runOTA();
  

  server.on ( "/api/", [](){ 
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication(); 
    processRequest(true); //isAPI = true
  });

  server.on ( "/api/others/", [](){   
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    requestForOthers();  
  });

  server.on ( "/api/content", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication(); 
    send_functional_content_html();  
  });

  server.on ( "/api/myFunctions.js", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication(); 
    send_myFunctions_js();  
  });

  server.on ( "/api/mystyle.css", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication(); 
    Serial.println("/api/mystyle.css"); 
    server.send ( 200, "text/html", PAGE_mystyle_css ); 
  });

  server.on ( "/othersMicroajax.js", []() { 
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    Serial.println("othersMicroajax.js"); 
    send_othersMicroajax();
  });
  
  server.on ( "/api/dodiscover", [](){    XDDP_discover(); server.send ( 200, "text/plain", "dodiscover" ); });
  
  server.on ( "/", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication(); 
    processRequest(false); //isAPI = false
  });
  server.on ( "/admin/filldynamicdata", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication(); 
    Serial.println("filldynamicdata");
    filldynamicdata();
  });
  
  server.on ( "/favicon.ico",   []() { 
    Serial.println("favicon.ico"); 
    server.send ( 200, "text/html", "" );   
    });

  server.on ( "/admin.html", []() { 
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    Serial.println("admin.html"); 
    server.send ( 200, "text/html", PAGE_AdminMainPage );   
    });
    
  server.on ( "/config.html", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    send_network_configuration_html(); 
  });
  
  server.on ( "/info.html", []() { 
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    Serial.println("info.html"); 
    server.send ( 200, "text/html", PAGE_Information );   
    });
    
  server.on ( "/ntp.html", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    send_NTP_configuration_html();  
  });
  
  server.on ( "/general.html", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    send_general_html();  
  });
  
  //server.on ( "/example.html", []() { server.send ( 200, "text/html", PAGE_EXAMPLE );  } );
  
  server.on ( "/style.css", []() { 
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    Serial.println("style.css"); 
    server.send ( 200, "text/css", PAGE_Style_css );  
  });
    
  server.on ( "/homepage_style.css", []() { 
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    Serial.println("homepage_style.css"); 
    server.send ( 200, "text/plain", PAGE_homepage_css );  
  });
  
  server.on ( "/microajax.js", []() { 
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    Serial.println("microajax.js"); 
    server.send ( 200, "text/plain", PAGE_microajax_js );  
  });
  
  server.on ( "/admin/values", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    send_network_configuration_values_html(); 
  });
  
  server.on ( "/admin/connectionstate", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    send_connection_state_values_html(); 
  });
  
  server.on ( "/admin/infovalues", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    send_information_values_html(); 
  });
  
  server.on ( "/admin/ntpvalues", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    send_NTP_configuration_values_html();
  });
  
  server.on ( "/admin/generalvalues", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    send_general_configuration_values_html();
  });
  
  server.on ( "/admin/devicename", [](){
    send_devicename_value_html();
  });
    
 

	server.onNotFound ( []() { Serial.println("Page Not Found"); server.send ( 400, "text/html", "Page not Found" );   }  );
	server.begin();
	Serial.println( "HTTP server started" );
	tkSecond.attach(1,Second_Tick);
	UDPNTPClient.begin(2390);  // Port for NTP receive

  XDDP_addFile("/api/", "text");
  XDDP_addFile("/api/content", "html");
  XDDP_addFile("/api/myFunctions.js", "js");
  XDDP_addFile("/api/mystyle.css", "css");
  XDDP_begin("XDDP Home Lights .01");
  
}

 
void loop ( void ) {
  
//	if (AdminEnabled)
//	{
//		if (AdminTimeOutCounter > AdminTimeOut)
//		{
//			 AdminEnabled = false;
//			 Serial.println("Admin Mode disabled!");
//			 WiFi.mode(WIFI_STA);
//		}
//	}
 
	if (config.Update_Time_Via_NTP_Every  > 0 )
	{
		if (cNTP_Update > 5 && firstStart)
		{
			NTPRefresh();
			cNTP_Update =0;
			firstStart = false;
		}
		else if ( cNTP_Update > (config.Update_Time_Via_NTP_Every * 60) )
		{

			NTPRefresh();
			cNTP_Update =0;
		}
	}

	if(DateTime.minute != Minute_Old)
	{
		 Minute_Old = DateTime.minute;
		 if (config.AutoTurnOn)
		 {
			 if (DateTime.hour == config.TurnOnHour && DateTime.minute == config.TurnOnMinute)
			 {
				  Serial.println("SwitchON");
			 }
		 }


		 Minute_Old = DateTime.minute;
		 if (config.AutoTurnOff)
		 {
			 if (DateTime.hour == config.TurnOffHour && DateTime.minute == config.TurnOffMinute)
			 {
				  Serial.println("SwitchOff");
			 }
		 }
	}
	server.handleClient();
  

	/*
	*    Your Code here
	*/
  ArduinoOTA.handle();
  XDDP_handle();
  

	if (Refresh)  
	{
		Refresh = false;
		///Serial.println("Refreshing...");
		 //Serial.printf("FreeMem:%d %d:%d:%d %d.%d.%d \n",ESP.getFreeHeap() , DateTime.hour,DateTime.minute, DateTime.second, DateTime.year, DateTime.month, DateTime.day);
	}



 


}

