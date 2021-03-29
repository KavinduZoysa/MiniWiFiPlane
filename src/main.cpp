 #include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <base64.h>

const char* ssid = "ESP";
const char* password = "12345678";

const char* userSSID;
char userPassWord[100];

String userssid;
String userPwd;

ESP8266WebServer server(80); 

void handleWiFiInfo() {
    userssid = server.arg(0);
    userSSID = userssid.c_str();
    Serial.print("1 : ");Serial.println(userSSID);

    userPwd = server.arg(1);
    int pwdLen = userPwd.length();
    char s[pwdLen+1];
    userPwd.toCharArray(s,  pwdLen);
    Serial.print("2 : ");Serial.println(userPwd);
    b64_decode(userPassWord, s, pwdLen-1);
    Serial.print("3 : ");Serial.println(userPassWord);

    server.send(200, "text/plain", "hello from esp8266!");
    // stay 2 seconds
    // close AP in ESP
}

void setup(void) {
    Serial.begin(115200);
    Serial.println("");
    WiFi.mode(WIFI_AP); 
    WiFi.softAPConfig(IPAddress(192, 168, 20,1), IPAddress(192, 168, 20,1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid, password); 

    IPAddress myIP = WiFi.softAPIP(); 
    Serial.print("HotSpt IP:");
    Serial.println(myIP);

    server.on("/", handleWiFiInfo); //Which routine to handle at root location

    server.begin(); //Start server
    Serial.println("HTTP server started");
}

//===============================================================
// LOOP
//===============================================================
void loop(void) {
    server.handleClient(); //Handle client requests
}