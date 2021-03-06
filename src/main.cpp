#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <base64.h>
#include <stdint.h>
#include <EEPROM.h>

#define LEFT 4
#define RIGHT 5
#define EEPROM_SIZE 100

const char *ssid = "ESP";
const char *password = "12345678";

char userSSID[100]; // TODO : Change this based on the requirement
char userPassWord[100]; // TODO : Change this based on the requirement
uint pwdLen = 0;
uint ssidLen = 0;

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
double voltage = 2.5;
int dB = 34;
unsigned long t = millis();
bool isUrgent = false;
uint8_t intArr[] = {52, 51, 50, 49};

bool connectUserWiFi() {
    WiFi.softAPdisconnect();
    server.close();

    if (!WiFi.mode(WIFI_STA)) {
        Serial.println("WIFI_STA Failed");
    }
    if (WiFi.config(staticIp, IPAddress(192, 168, 1, 1), IPAddress(255, 255, 0, 0))) {
        // This config fails. Should check it
        Serial.println("STA Failed to configure");
    }

    WiFi.begin(userSSID, userPassWord);

    Serial.print("Connecting to ");
    Serial.print(userSSID);
    int i = 100; // Wait for 10 seconds
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
        i--;
        if (i < 0) {
            return false;
        }
    }

    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());

    Udp.begin(localUdpPort);
    Serial.print("Started UDP communication ");
    return true;
}

void saveToROM() {
    int totLen = ssidLen + pwdLen + 3;
    int nextPos = 0;
    if (totLen > EEPROM_SIZE) {
        EEPROM.begin(totLen);
    } else {
        EEPROM.begin(EEPROM_SIZE);
    }

    EEPROM.write(nextPos, totLen);
    EEPROM.commit();
    nextPos = nextPos + 1;
    EEPROM.write(nextPos, ssidLen);
    EEPROM.commit();
    nextPos = nextPos + 1;
    for (size_t i = 0; i < ssidLen; i++) {
        EEPROM.write(nextPos + i, userSSID[i]);
        EEPROM.commit();
    }
    nextPos = nextPos + ssidLen;
    EEPROM.write(nextPos, pwdLen);
    EEPROM.commit();
    nextPos = nextPos + 1;
    for (size_t i = 0; i < pwdLen; i++) {
        EEPROM.write(nextPos + i, userPassWord[i]);
        EEPROM.commit();
    }    
}

void getUserWiFiInfo() {
    userssid = server.arg(0);
    ssidLen = userssid.length();
    userssid.toCharArray(userSSID, ssidLen + 1);

    userPwd = server.arg(1);
    int pwdEncodedLen = userPwd.length();
    char s[pwdEncodedLen + 1];
    userPwd.toCharArray(s, pwdEncodedLen);
    pwdLen = b64_decode(userPassWord, s, pwdEncodedLen);
    
    saveToROM();

    server.send(200, "text/plain", "hello from esp8266!");
    Serial.println("Waiting for 3 seconds");
    delay(3000);
    connectUserWiFi();
}

void readDataFromROM() {
    int totLen = ssidLen + pwdLen + 3;
    if (totLen > EEPROM_SIZE) {
        EEPROM.begin(totLen);
    } else {
        EEPROM.begin(EEPROM_SIZE);
    }

    int nextPos = 1;
    ssidLen = EEPROM.read(nextPos);
    Serial.printf("%d\n", ssidLen);
    nextPos = nextPos + 1;
    for (size_t i = 0; i < ssidLen; i++) {
        userSSID[i] = EEPROM.read(nextPos + i);
        Serial.printf("%c", userSSID[i]);
    }
    nextPos = nextPos + ssidLen;
    pwdLen = EEPROM.read(nextPos);
    Serial.printf("\n%d\n", pwdLen);
    nextPos = nextPos + 1;
    for (size_t i = 0; i < pwdLen; i++) {
        userPassWord[i] = EEPROM.read(nextPos + i);
        Serial.printf("%c", userPassWord[i]);
    }
}

void writeToROMForTesting() {
    Serial.println("New entry!");
    userSSID[0] = 'S';
    userSSID[1] = 'L';
    userSSID[2] = 'T';

    userPassWord[0] = '1';
    userPassWord[1] = '2';
    userPassWord[2] = '3';
    userPassWord[3] = '4';
    userPassWord[4] = '1';
    userPassWord[5] = '2';
    userPassWord[6] = '3';
    userPassWord[7] = '4';
    userPassWord[8] = 'a';

    ssidLen = strlen(userSSID);
    pwdLen = strlen(userPassWord);
    saveToROM();
    readDataFromROM();
}

void startServer() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(staticIp, IPAddress(192, 168, 1, 1), IPAddress(255, 255, 0, 0));
    WiFi.softAP(ssid, password);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("\nHotSpt IP:");
    Serial.println(myIP);

    server.on("/", getUserWiFiInfo);

    server.begin(); // Start server
    Serial.println("HTTP server started");
}

void setup(void) {
    Serial.begin(115200);
    readDataFromROM();
    // If user's wifi connection was successful, it should be indicated via bulb.
    if (!connectUserWiFi()) {
        startServer();
    }
}

void send() {
    // send back a reply, to the IP address and port we got the packet from
    printf("Ip : %s\n", phoneIp.toString().c_str());
    printf("port : %d", phonePort);
    // Udp.beginPacket(phoneIp, phonePort);
    Udp.beginPacket(phoneIp, 49456);
    // int sendingPckt = dB*100 + (voltage * 10) + 1;
    Udp.write(intArr, 4);
    // Udp.write(replyPacket);
    Udp.endPacket();
    Serial.print("Sent!");
}

void recieveAndProcess() {
    int packetSize = Udp.parsePacket();
    if (!packetSize) {
        return;
    }
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
    isUrgent = true;
}

void loop(void) {
    if (WiFi.getMode() == WIFI_AP) {
        server.handleClient();
    }

    recieveAndProcess();
    if (isUrgent && ((millis() - t) > 5000)) {
        t = millis();
        send();
    }
    // for (size_t i = 0; i < 256; i++) {
    //     analogWrite(LEFT, i);
    //     analogWrite(RIGHT, 256 - i);
    //     delay(2500);
    // }
}
