#include <Arduino.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "MFRC522.h"
#include <ArduinoJson.h>

StaticJsonBuffer<200> jsonBuffer;

#define RST_PIN D1
#define SS_PIN D2

// INIT MIFARE RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

// INIT SERVO
Servo servo;

// INIT WEMOS ESP - WIFI
const char* ssid     = "Pixel_1687";
const char* password = "04nov1994";

// INIT HTTP ACCESS
HTTPClient http;
const char *HTTP_= "http://";
const char *SERVER= "192.168.43.117:3000";
const char *API_GET_DATA  = "/users/";
const char *API_POST_DATA  = "/drinks";

// INIT TIMER
int volume = 10000; // 250 ML
unsigned long time_now = 0;


// METHOD FIND USER
int find_user_id(String rfid){
  String _tempURI = HTTP_; 
	_tempURI += SERVER;
	_tempURI += API_GET_DATA;
	_tempURI += rfid;
	http.begin(_tempURI);

	http.addHeader("Content-Type", "application/x-www-form-urlencoded");
	int httpCode = http.GET();
	int user_id;

	if (httpCode == 200){
			String payload = http.getString();
			JsonObject& users = jsonBuffer.parseObject(payload);
			user_id = users['data']['user_id'];

	}else{
		return false;
		Serial.println("Error on HTTP request");
	}

	http.end();

	return user_id;
}

// METHOD SEND DATA
int insert_data(int user_id, int val){
  String _tempURI = HTTP_;
  _tempURI += SERVER;
  _tempURI += API_POST_DATA;
  http.begin(_tempURI);

	int vol = val * 25 / 1000;

  String _postData = "user_id=" + user_id;
	 _postData += "&value=" + vol;

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(_postData);
  http.end();

  Serial.println(httpCode);
 
  return httpCode;
}

bool servo_on(){
	servo.write(90);
	return true;
}

bool servo_off(){
	servo.write(0);
	return true;
}

bool is_mug_available(){
	 int data = analogRead(A0);

	 if(data < 90){
		 return true;
	 }else{
		 return false;
	 }
	 
}

// SETUP
void setup(){
	Serial.begin(115200);

	SPI.begin();
	mfrc522.PCD_Init();

	servo.attach(D4);
	servo.write(0);
	delay(1000);

	WiFi.begin(ssid, password);
	Serial.print("Connecting to ");
	Serial.print(ssid); Serial.println(" ...");

	int i = 0;
	while (WiFi.status() != WL_CONNECTED) {
			delay(1000);
			Serial.print(++i); Serial.print(' ');
	}

	Serial.println('\n');
	Serial.println("Connection established!");  
	Serial.print("IP address:\t");
	Serial.println(WiFi.localIP());
}

String generate_rfid(byte *buffer, byte bufferSize) {
  String CONTENT = "";
  String DATA_ID = "";

  for (byte i = 0; i < bufferSize; i++) {
    CONTENT.concat(String(buffer[i] < 0x10 ? "0" : ""));
    CONTENT.concat(String(buffer[i], HEX));
  }

  return CONTENT;
}


void loop(){
	if ((WiFi.status() == WL_CONNECTED)){
			
			if ( ! mfrc522.PICC_IsNewCardPresent()) {
					delay(50);
					return;
			}
			
			if ( ! mfrc522.PICC_ReadCardSerial()) {
					delay(50);
					return;
			}

			if( ! is_mug_available()){
				return;
			}

			String rfid = generate_rfid(mfrc522.uid.uidByte, mfrc522.uid.size);
			int user_id = find_user_id(rfid);

			if(user_id){
					if(millis() > time_now + volume){

						time_now = millis();
						if(is_mug_available()){
							servo_on();

							if(volume > time_now){
								servo_off();
								insert_data(user_id, volume);
								time_now = 0;

								return;
							}

						}else{
							servo_off();
							insert_data(user_id, time_now);
							time_now = 0;

							return;
						}
						
					}
					
			}
  
	}

}
