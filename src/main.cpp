#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <base64.h>

const char *ssid = "ESP";
const char *password = "12345678";

const char *userSSID;
char userPassWord[100];

IPAddress staticIp(192, 168, 1, 20);

unsigned int localUdpPort = 4210;
IPAddress phoneIp;
uint16_t phonePort;

String userssid;
String userPwd;

ESP8266WebServer server(80);
WiFiUDP Udp;

char incomingPacket[255];
char replyPacket[] = "Hi there! Got the message :-)";

void connectUserWiFi() {
    // WiFi.disconnect();
    WiFi.softAPdisconnect();
    server.close();

    WiFi.mode(WIFI_STA);
    if (WiFi.config(staticIp, IPAddress(192, 168, 1, 1), IPAddress(255, 255, 0, 0))) {
        Serial.println("STA Failed to configure");
    }

    WiFi.begin(userSSID, userPassWord);

    Serial.print("Connecting to ");
    Serial.print(userSSID);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());

    Udp.begin(localUdpPort);
    Serial.print("Started UDP communication ");
}

void getUserWiFiInfo() {
    userssid = server.arg(0);
    userSSID = userssid.c_str();

    userPwd = server.arg(1);
    int pwdLen = userPwd.length();
    char s[pwdLen + 1];
    userPwd.toCharArray(s, pwdLen);
    b64_decode(userPassWord, s, pwdLen - 1);

    server.send(200, "text/plain", "hello from esp8266!");
    Serial.println("Waiting for 3 seconds");
    delay(3000);
    connectUserWiFi();
}

void setup(void) {
    Serial.begin(115200);
    Serial.println("");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(staticIp, IPAddress(192, 168, 1, 1), IPAddress(255, 255, 0, 0));
    WiFi.softAP(ssid, password);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("HotSpt IP:");
    Serial.println(myIP);

    server.on("/", getUserWiFiInfo); 

    server.begin(); // Start server
    Serial.println("HTTP server started");
}

void loop(void) {
    if (WiFi.getMode() == WIFI_AP) {
        server.handleClient();
    }

    int packetSize = Udp.parsePacket();
    if (packetSize) {
        // receive incoming UDP packets
        Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(),
                      Udp.remotePort());
        int len = Udp.read(incomingPacket, 255);
        if (len > 0) {
            incomingPacket[len] = 0;
        }
        Serial.printf("UDP packet contents: %s\n", incomingPacket);

        phoneIp = Udp.remoteIP();
        phonePort = Udp.remotePort();

        // send back a reply, to the IP address and port we got the packet from
        Udp.beginPacket(phoneIp, phonePort);
        Udp.beginPacket(phoneIp, 49456);
        Udp.write(replyPacket);
        Udp.endPacket();
    }
}
