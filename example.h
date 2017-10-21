#ifndef PAGE_EXAMPLE_H
#define PAGE_EXAMPLE_H
//
//   The EXAMPLE PAGE
//
const char PAGE_example[] PROGMEM = R"=====(
  
  <head> 
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <link rel="stylesheet" type="text/css" href="homepage_style.css">
  <script src="/microajax.js"></script>
  <script src="/othersMicroajax.js"></script>
  <title>XAMBIS Web Server 1</title>     

  </head>
  <body>

    <div class="container"> 
      <header> 
        <h1>XAMBIS Web Server 1</h1> 
      </header>

      <div id="navMenu">
      <nav> 
        <ul> 
          <li><a href="/">XDDP Home Lights .01</a></li> 
          <li><a href="admin.html">Admin Page</a></li> 
        </ul> 
      </nav>
      </div>

    <article>
      <h1>Controls</h1>
      <div id="ESP01">My dynamic content...</div>

      <div id="ESPothers">Gathering data from others...</div>
      
    </article>

    <footer>Copyright © xambis </footer>
  </div> 
  
  </body> 
  <script>
  
  
  window.onload = function (){
    
      microAjax("/admin/filldynamicdata", setValues);
      loadOthers();
  }

  
  </script>

)=====";
#endif



String getFunctionalContent(){
  String reply = "";
  reply += "<p>";
  reply += "<table cellspacing=\"0\" cellpadding=\"5\">";
  reply += "ESP - " + XDDP_friendly_name;
  reply += "<tr><td>Light In</td><td>Light Out</td></tr>";
  reply += "<tr>";
  reply += "<td><label class=\"switch\">";
  reply += (String)"<input type=\"checkbox\" id=\"lightIn\" onclick=\"lightOnClick(this)\"" + (digitalRead(LightInPin) ? "" : " checked") + ">";
  reply += "<span class=\"slider round\"></span>";
  reply += "</label></td>";
  reply +=  "<td><label class=\"switch\">";
  reply +=  (String)"<input type=\"checkbox\" id=\"lightOut\" onclick=\"lightOnClick(this)\"" + (digitalRead(LightOutPin) ? "" : " checked") + ">";
  reply +=  "<span class=\"slider round\"></span>";
  reply +=  "</label></td>";
  reply +=  "</tr>";
  reply +=  "</table>";
  reply +=  "</p>";

  return reply;
} 

void filldynamicdata()
{   
    String values = "";
    values += "ESP01|" + getFunctionalContent() + "|div\n";
    values += "navMenu|" + getXDDPnav() + "|div\n";
    values += "ESPothers|" + getXDDPdivs() + "|div\n";
    server.send ( 200, "text/plain", values);   
      
}


void processRequest(bool isAPI){        
    String action12 = "";
    String action13 = "";
    String reply = "";
    
    if (server.args() > 0 )  // Are there any POST/GET Fields ? 
    {
       for ( uint8_t i = 0; i < server.args(); i++ ) {  // Iterate through the fields
            if (server.argName(i) == "lightIn") {               
               action12 = server.arg(i);
            }else if(server.argName(i) == "lightOut"){
               action13 = server.arg(i);
            }
        }

        if(action12 == "T"){
          Serial.println("lightIn=T"); reply += "lightIn=T\n";
          digitalWrite(LightInPin, !digitalRead(LightInPin));
        }else if(action12 == "ON"){
          Serial.println("lightIn=ON"); reply += "lightIn=ON\n";
          digitalWrite(LightInPin, LOW);
        }else if(action12 == "OFF"){
          Serial.println("lightIn=OFF"); reply += "lightIn=OFF\n";
          digitalWrite(LightInPin, HIGH);
        }else if(action12 != ""){
          Serial.println("Warning: lightIn=" + action12 + " invalid"); reply += "Bad request - lightIn=" + action12 + " invalid \n";
        }

        if(action13 == "T"){
          Serial.println("lightOut=T"); reply += "lightOut=T\n";
          digitalWrite(LightOutPin, !digitalRead(LightOutPin));
        }else if(action13 == "ON"){
          Serial.println("lightOut=ON"); reply += "lightOut=ON\n";
          digitalWrite(LightOutPin, LOW);
        }else if(action13 == "OFF"){
          Serial.println("lightOut=OFF"); reply += "lightOut=OFF\n";
          digitalWrite(LightOutPin, HIGH);
        }else if(action13 != ""){
          Serial.println("Warning: lightOut=" + action13 + " invalid"); reply += "Bad request - lightOut=" + action13 + " invalid \n";
        }

        
    }

    if(isAPI){
      if(server.args() == 0) server.send ( 200, "text/html", "No input"  ); 
      if(reply == "") server.send ( 200, "text/html", "Bad input"  ); 
      else server.send ( 200, "text/html", reply  ); 
    }else 
      server.send ( 200, "text/html", PAGE_example  ); 

}

//void processAPI(){
//  bool valid = false;
//}




void send_functional_content_html(){
  Serial.println("send_functional_content_html");
  server.send ( 200, "text/html", getFunctionalContent() );
}

// ================================================================
// ===                  BUTTONS & INTERRUPTS                    ===
// ================================================================
const byte buttonIn = 4; 
const byte buttonOut = 5; 
volatile bool buttonStateIn = true;
volatile bool buttonStateOut = true;
unsigned long debounceDelay = 30;    // the debounce time(ms); increase if the output flickers

#include <Ticker.h>

Ticker changeButtonInT;

void changeButtonIn(){
  if(buttonStateIn != digitalRead(buttonIn)){
    buttonStateIn = !buttonStateIn;
    if(buttonStateIn == HIGH) digitalWrite(LightInPin, !digitalRead(LightInPin));
  }
  changeButtonInT.detach();
}
void interuptButtonIn() {
  changeButtonInT.detach();
  changeButtonInT.attach_ms(debounceDelay, changeButtonIn);
}

Ticker changeButtonOutT;

void changeButtonOut(){
  if(buttonStateOut != digitalRead(buttonOut)){
    buttonStateOut = !buttonStateOut;
    if(buttonStateOut == HIGH) digitalWrite(LightOutPin, !digitalRead(LightOutPin));
  }
  changeButtonOutT.detach();
}
void interuptButtonOut() {
  changeButtonOutT.detach();
  changeButtonOutT.attach_ms(debounceDelay, changeButtonOut);
}

