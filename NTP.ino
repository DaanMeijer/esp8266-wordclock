

#include <WiFiUdp.h>

/*-------- NTP code ----------*/


const int   timeZone        = 1;     // Central European Time
bool        autoDST         = true;

IPAddress   timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName   = "nl.pool.ntp.org";
const int   NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte        packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets


#define SYNC_INTERVAL   1200

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

unsigned long   lastdisplayupdate   = 0;

void setupNtp(){
  

    setSyncProvider(getNtpTime);
    setSyncInterval(SYNC_INTERVAL);
}

time_t getNtpTime()
{
    IPAddress ntpServerIP; // NTP server's ip address

    while (Udp.parsePacket() > 0) ; // discard any previously received packets
    Serial.println("Transmit NTP Request");
    // get a random server from the pool
    WiFi.hostByName(ntpServerName, ntpServerIP);
    Serial.print(ntpServerName);
    Serial.print(": ");
    Serial.println(ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
            Serial.println("Receive NTP Response");
            Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            // New time in seconds since Jan 1, 1970
            unsigned long newTime = secsSince1900 - 2208988800UL +
                timeZone * SECS_PER_HOUR;

            // Auto DST
            if (autoDST) {
                if (isDSTSwitchDay(day(newTime), month(newTime), year(newTime))) {
                    if (month(newTime) == 3 && hour(newTime) >= 2) {
                        newTime += SECS_PER_HOUR;
                    } else if (month(newTime) == 10 && hour(newTime) < 2) {
                        newTime += SECS_PER_HOUR;
                    }
                } else if (isDST(day(newTime), month(newTime), year(newTime))) {
                    newTime += SECS_PER_HOUR;
                }
            }

            setSyncInterval(SYNC_INTERVAL);
            return newTime;
        }
    }
    Serial.println("No NTP Response :-(");
    // Retry soon
    setSyncInterval(10);
    return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}

