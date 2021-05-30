# MiniWiFiPlane

1. url to send user's SSID and password -
        http://192.168.20.1/?ssid=SLT&pwd=bmVWZXJnMXYjdVA=

2. url to stop  AP in ESP and connect to user's WiFi -

## EEPROM data format

location 1 - The given constant value (`EEPROM_CONST`)

location 2 - Length of SSID(`len_SSID`)

From location 3 to location 3 + len_SSID - Characters of SSID

location 3 + len_SSID + 1 - Length of password(`len_PWD`)

From location 3 + len_SSID + 2 to location 3 + len_SSID + 2 + len_PWD - Characters on Password
