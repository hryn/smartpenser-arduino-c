#include <Arduino.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "MFRC522.h"

const char* ssid     = "ARIE";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "";     // The password of the Wi-Fi network

#define RST_PIN D1 // RST-PIN for RC522 - RFID - SPI - Modul GPIO15 
#define SS_PIN  D2  // SDA-PIN for RC522 - RFID - SPI - Modul GPIO2

// Mifare
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

// Servo
Servo servo;

// HTTP
HTTPClient http;
const char *HTTP_= "http://";
const char *SERVER= "192.168.2.2";
const char *PATH  = "/platform_futrid/api/device/perangkat/";
const char *PATH_USER  = "/platform_futrid/api/device/user/";
String api_key = "a751bfe7cb390308faed47b51c7828f5";

int getRequest(String kartuid, String kode_ruangan){
  String _tempURI = HTTP_; 
  _tempURI += SERVER;
  _tempURI += PATH_USER;
  _tempURI += kartuid;
  _tempURI += "/";
  _tempURI += kode_ruangan;
  http.begin(_tempURI); //kirim data 

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("X-API-KEY", api_key); //
  int httpCode = http.GET();

  if (httpCode > 0)
  { 
      String payload = http.getString();
    // Serial.println(httpCode);
    // Serial.println(payload);
    }else{
      return httpCode;
      Serial.println("Error on HTTP request");
    }

    Serial.println(httpCode);
    http.end(); //Free the resources
    return httpCode;
}

int inisialiasi_perangkat(String _nama_perangkat, String _kode_ruangan, String _ssid, String _status, String _ip_addr, String _mac_addr){
  String _tempURI = HTTP_;
  _tempURI += SERVER;
  _tempURI += PATH;
  http.begin(_tempURI); //kirim datahttpCode

  String postData_ = "nama_perangkat=" + _nama_perangkat + "&key_ruangan=" + _kode_ruangan + "&ssid=" + _ssid + "&status=" + _status + "&ip=" + _ip_addr  + "&mac=" + _mac_addr;
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("x-api-key", api_key);
  int httpCode = http.POST(postData_);
  http.end();

  Serial.println(httpCode);
 
  return httpCode;
}

void setup() {
    Serial.begin(115200);

    // Init Mifare
    SPI.begin();           // Init SPI bus
    mfrc522.PCD_Init();    // Init MFRC522 

    // Init Servo
    servo.attach(D8); //D4
    servo.write(0);
    delay(2000);

    Serial.println('\n');
    
    WiFi.begin(ssid, password);             // Connect to the network
    Serial.print("Connecting to ");
    Serial.print(ssid); Serial.println(" ...");

    int i = 0;
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(1000);
        Serial.print(++i); Serial.print(' ');
    }

    Serial.println('\n');
    Serial.println("Connection established!");  
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
}

// Helper
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? "0" : "");
    Serial.print(buffer[i], HEX);
  }
} 

/*
    integer analogRead(A0) : < 50 is true
*/


void loop() { 
  //Serial.println(analogRead(A0));

  if ((WiFi.status() == WL_CONNECTED)){

    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
        delay(50);
        return;
    }

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
        delay(50);
        return;
    }

    getRequest("556678", "be3a1fad");

    inisialiasi_perangkat("Arie", "wkwk","axasdasd", "asdasdasdqw", "123.12312.3123123", "whjdhasd");

    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println("");

    servo.write(90);
    delay(1000);

    servo.write(0);
    delay(1000);

  }

}