#include <Arduino.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "MFRC522.h"
#include <ArduinoJson.h>

#define RST_PIN D1
#define SS_PIN D2

// INIT MIFARE RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

// INIT SERVO
Servo servo;

// INIT WEMOS ESP - WIFI
const char* ssid     = "ARIF";
const char* password = "";

// INIT HTTP ACCESS
HTTPClient http;
const char *HTTP_= "http://";
const char *SERVER= "192.168.3.89";
const char *API_GET_DATA  = "/api/users";
const char *API_POST_DATA  = "/api/users/drinks/insert";

// INIT TIMER
int pull_time = 10000; // 250 ML
unsigned long time_now = 0;

int pull = 0;
int user_id = 0;

bool is_servo_trigerred = false;
bool is_mug_available = false;


// METHOD FIND USER
int find_user_id(String rfid){
  String _tempURI = HTTP_; 
	_tempURI += SERVER;
	_tempURI += API_GET_DATA;
	_tempURI += "?rfid=";
	_tempURI += rfid;
	http.begin(_tempURI);

	http.addHeader("Content-Type", "application/x-www-form-urlencoded"); 
	int httpCode = http.GET();
    
    if (httpCode == 200){
			String payload = http.getString();

			StaticJsonBuffer<300> jsonBuffer;
			JsonObject& users = jsonBuffer.parseObject(payload);
			//Serial.println(payload);
			user_id = users["data"]["id"];
			Serial.print("User ID: ");
			Serial.println(user_id);

			pull = 1;

			return true;
      http.end();

	}else{
			Serial.println("User not found!");
			pull = 0;
			return false;
			http.end();
  }
}

// METHOD SEND DATA
int insert_data(int user_id, int val){
	int vol = val * 25 / 1000;
	String v = String(vol);
	String u = String(user_id);
  String postData = "user_id=" + u + "&milis=" + v;
	
	Serial.println(postData);

  String _tempURI = HTTP_;
  _tempURI += SERVER;
  _tempURI += API_POST_DATA;	
  http.begin(_tempURI);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

	
  int httpCode = http.POST(postData);

  return httpCode;

	http.end();

}

bool servo_on(){
	servo.write(90);

	is_servo_trigerred = true;
	return true;
}

bool reset_variable(){
	user_id = 0;
	pull = 0;
	time_now = 0;
	is_servo_trigerred = false;

	return true;
}

bool servo_off(){
	servo.write(0);
	reset_variable();
	delay(5000);
	return true;
}

bool set_mug(){
	 int infra = analogRead(A0);
	 if(infra < 500){
		 is_mug_available = true;
	 }else{
		 //Serial.println("Mug tidak ditemukan");
		 is_mug_available = false;
	 }
	 return true;
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

		set_mug();
			
		String rfid;
		
		if(is_servo_trigerred){
			if((millis() > time_now + pull_time) || !is_mug_available){
				int vol = millis() - time_now;
				insert_data(user_id, vol);
				servo_off(); 
			}
		}
			
			switch(pull) {
				case 0:
					if ( ! mfrc522.PICC_IsNewCardPresent()) {
							delay(50);
							return;
					}
					
					if ( ! mfrc522.PICC_ReadCardSerial()) {
							delay(50);
							return;
					}

					rfid = generate_rfid(mfrc522.uid.uidByte, mfrc522.uid.size);
					find_user_id(rfid);
					break;

				case 1:
					if(millis() > time_now + pull_time){
						time_now = millis();

						if(is_mug_available){
							servo_on();
						}
					}
					break;
			}  
	}

}
