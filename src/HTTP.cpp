#include "HTTP.hpp"

#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
   connected to this access point to see it.
*/
void handleRoot() {
  Serial.println("Serving root");
  server.send(200, "text/html", "<h1>You are connected</h1><form method='post' action='/settime'><input type=text id=timestamp name=timestamp /></form><script>const elm = document.querySelector('#timestamp').value=Math.floor(Date.now() / 1000); document.querySelector('form').submit();</script>");
}

void handleSetTime(){
      if (server.hasArg("plain")== false){ //Check if body received
            server.send(200, "text/plain", "Body not received");
            return;
      }
 
      String message = "Body received:\n";
             message += server.arg("plain");
             message += "\n";
 
      server.send(200, "text/plain", message);
      Serial.println(message);
}

void HTTP_setup(){

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);

  server.on("/settime", handleSetTime);
  server.begin();
  Serial.println("HTTP server started");
}

void HTTP_loop(){
  server.handleClient();
}
